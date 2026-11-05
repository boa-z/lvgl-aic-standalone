/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * LVGL 9.6 runtime: one UI owner thread, RT-Thread OS integration, lifecycle
 * and safe asynchronous UI calls.
 */

#include "lvgl_aic.h"
#include "lvgl_aic_display_port.h"
#ifdef LVGL_AIC_USE_TOUCH
#include "lvgl_aic_touch_port.h"
#endif

#include <rtthread.h>
#include <string.h>

#ifndef LVGL_AIC_UI_STACK_SIZE
#define LVGL_AIC_UI_STACK_SIZE 32768U
#endif
#ifndef LVGL_AIC_UI_PRIORITY
#define LVGL_AIC_UI_PRIORITY 15U
#endif
#ifndef LVGL_AIC_UI_PERIOD_MS
#define LVGL_AIC_UI_PERIOD_MS 10U
#endif
#ifndef LVGL_AIC_DISPLAY_DEVICE
#define LVGL_AIC_DISPLAY_DEVICE "lcd"
#endif
#ifndef LVGL_AIC_TOUCH_DEVICE
#define LVGL_AIC_TOUCH_DEVICE "gt911"
#endif

#ifdef LVGL_AIC_TOUCH_SWAP_XY
#define LVGL_AIC_TOUCH_SWAP_VALUE 1
#else
#define LVGL_AIC_TOUCH_SWAP_VALUE 0
#endif
#ifdef LVGL_AIC_TOUCH_X_FLIP
#define LVGL_AIC_TOUCH_X_FLIP_VALUE 1
#else
#define LVGL_AIC_TOUCH_X_FLIP_VALUE 0
#endif
#ifdef LVGL_AIC_TOUCH_Y_FLIP
#define LVGL_AIC_TOUCH_Y_FLIP_VALUE 1
#else
#define LVGL_AIC_TOUCH_Y_FLIP_VALUE 0
#endif

#define LVGL_AIC_UI_MQ_DEPTH 16

typedef struct
{
    void (*callback)(void *user_data);
    void *user_data;
} lvgl_aic_msg_t;

typedef struct
{
    lvgl_aic_config_t config;
    lv_display_t *display;
    lv_indev_t *touch_indev;
    rt_thread_t thread;
    rt_mq_t mq;
    rt_sem_t exit_sem;
    volatile int running;
    volatile int exit_requested;
    volatile uint32_t async_dropped;
    int initialized;
} lvgl_aic_runtime_t;

static lvgl_aic_runtime_t g_lvgl_aic;

static uint32_t lvgl_aic_tick_ms(void)
{
    return (uint32_t)rt_tick_get_millisecond();
}

static void lvgl_aic_process_messages(void)
{
    lvgl_aic_msg_t msg;

    while (g_lvgl_aic.mq != RT_NULL &&
           rt_mq_recv(g_lvgl_aic.mq, &msg, sizeof(msg), 0) == RT_EOK)
    {
        if (msg.callback != RT_NULL)
            msg.callback(msg.user_data);
    }
}

static void lvgl_aic_ui_thread(void *parameter)
{
    (void)parameter;

    while (!g_lvgl_aic.exit_requested)
    {
        if (g_lvgl_aic.running)
        {
            lvgl_aic_process_messages();
            lv_timer_handler();
        }
        rt_thread_mdelay(g_lvgl_aic.config.ui_period_ms);
    }

    if (g_lvgl_aic.exit_sem != RT_NULL)
        rt_sem_release(g_lvgl_aic.exit_sem);
}

static const lvgl_aic_display_backend_t *lvgl_aic_select_backend(void)
{
#ifdef LVGL_AIC_USE_D21X_BACKEND
    return lvgl_aic_backend_d21x();
#else
    return lvgl_aic_backend_rtthread();
#endif
}

static void lvgl_aic_fill_defaults(lvgl_aic_config_t *config,
                                   const lvgl_aic_config_t *user)
{
    if (user != NULL)
        *config = *user;

    if (config->display_device == NULL)
        config->display_device = LVGL_AIC_DISPLAY_DEVICE;
    if (config->touch_device == NULL)
        config->touch_device = LVGL_AIC_TOUCH_DEVICE;
    if (config->ui_stack_size == 0)
        config->ui_stack_size = LVGL_AIC_UI_STACK_SIZE;
    if (config->ui_priority == 0)
        config->ui_priority = LVGL_AIC_UI_PRIORITY;
    if (config->ui_period_ms == 0)
        config->ui_period_ms = LVGL_AIC_UI_PERIOD_MS;
}

lvgl_aic_result_t lvgl_aic_init(const lvgl_aic_config_t *config)
{
    lvgl_aic_runtime_t *rt = &g_lvgl_aic;
    const lvgl_aic_display_backend_t *backend;
    lvgl_aic_result_t result;

    if (rt->initialized)
        return LVGL_AIC_ERR_INVALID_STATE;

    memset(rt, 0, sizeof(*rt));
    lvgl_aic_fill_defaults(&rt->config, config);

    lv_init();
    lv_tick_set_cb(lvgl_aic_tick_ms);

    backend = lvgl_aic_select_backend();
    result = lvgl_aic_display_port_init(&rt->display, backend);
    if (result != LVGL_AIC_OK)
        goto fail_lvgl;

#ifdef LVGL_AIC_USE_TOUCH
    {
        lvgl_aic_touch_config_t touch_cfg;

        memset(&touch_cfg, 0, sizeof(touch_cfg));
        touch_cfg.device_name = rt->config.touch_device;
        touch_cfg.swap_xy = LVGL_AIC_TOUCH_SWAP_VALUE;
        touch_cfg.x_flip = LVGL_AIC_TOUCH_X_FLIP_VALUE;
        touch_cfg.y_flip = LVGL_AIC_TOUCH_Y_FLIP_VALUE;
        touch_cfg.poll_ms = rt->config.ui_period_ms;

        result = lvgl_aic_touch_port_init(rt->display, &touch_cfg,
                                          &rt->touch_indev);
        if (result != LVGL_AIC_OK)
            goto fail_display;
    }
#endif

    rt->mq = rt_mq_create("lvaic", sizeof(lvgl_aic_msg_t),
                          LVGL_AIC_UI_MQ_DEPTH, RT_IPC_FLAG_FIFO);
    if (rt->mq == RT_NULL)
    {
        result = LVGL_AIC_ERR_NO_MEMORY;
        goto fail_touch;
    }

    rt->exit_sem = rt_sem_create("lvaicx", 0, RT_IPC_FLAG_FIFO);
    if (rt->exit_sem == RT_NULL)
    {
        result = LVGL_AIC_ERR_NO_MEMORY;
        goto fail_mq;
    }

    rt->thread = rt_thread_create("lvgl_aic", lvgl_aic_ui_thread, RT_NULL,
                                  rt->config.ui_stack_size,
                                  rt->config.ui_priority,
                                  rt->config.ui_period_ms);
    if (rt->thread == RT_NULL)
    {
        result = LVGL_AIC_ERR_NO_MEMORY;
        goto fail_exit_sem;
    }
    rt_thread_startup(rt->thread);

    if (rt->config.create_ui != NULL)
        rt->config.create_ui(rt->display);

    rt->initialized = 1;
    return LVGL_AIC_OK;

fail_exit_sem:
    rt_sem_delete(rt->exit_sem);
    rt->exit_sem = RT_NULL;
fail_mq:
    rt_mq_delete(rt->mq);
    rt->mq = RT_NULL;
fail_touch:
#ifdef LVGL_AIC_USE_TOUCH
    lvgl_aic_touch_port_deinit(rt->touch_indev);
    rt->touch_indev = NULL;
#endif
fail_display:
    lvgl_aic_display_port_deinit(rt->display);
    rt->display = NULL;
fail_lvgl:
    lv_deinit();
    memset(rt, 0, sizeof(*rt));
    return result;
}

lvgl_aic_result_t lvgl_aic_start(void)
{
    if (!g_lvgl_aic.initialized)
        return LVGL_AIC_ERR_INVALID_STATE;
    g_lvgl_aic.running = 1;
    return LVGL_AIC_OK;
}

void lvgl_aic_stop(void)
{
    g_lvgl_aic.running = 0;
}

void lvgl_aic_deinit(void)
{
    lvgl_aic_runtime_t *rt = &g_lvgl_aic;

    if (!rt->initialized)
        return;

    rt->running = 0;
    rt->exit_requested = 1;

    if (rt->thread != RT_NULL && rt->exit_sem != RT_NULL)
        (void)rt_sem_take(rt->exit_sem, rt_tick_from_millisecond(2000));

#ifdef LVGL_AIC_USE_TOUCH
    lvgl_aic_touch_port_deinit(rt->touch_indev);
    rt->touch_indev = NULL;
#endif
    lvgl_aic_display_port_deinit(rt->display);
    rt->display = NULL;

    if (rt->exit_sem != RT_NULL)
    {
        rt_sem_delete(rt->exit_sem);
        rt->exit_sem = RT_NULL;
    }
    if (rt->mq != RT_NULL)
    {
        rt_mq_delete(rt->mq);
        rt->mq = RT_NULL;
    }

    lv_deinit();
    memset(rt, 0, sizeof(*rt));
}

lv_display_t *lvgl_aic_get_display(void)
{
    return g_lvgl_aic.display;
}

lv_indev_t *lvgl_aic_get_touch_indev(void)
{
    return g_lvgl_aic.touch_indev;
}

lvgl_aic_result_t lvgl_aic_async_call(void (*callback)(void *user_data),
                                      void *user_data)
{
    lvgl_aic_msg_t msg;

    if (!g_lvgl_aic.initialized || callback == NULL)
        return LVGL_AIC_ERR_INVALID_STATE;
    if (g_lvgl_aic.mq == RT_NULL)
        return LVGL_AIC_ERR_INVALID_STATE;

    msg.callback = callback;
    msg.user_data = user_data;
    if (rt_mq_send(g_lvgl_aic.mq, &msg, sizeof(msg)) != RT_EOK)
    {
        g_lvgl_aic.async_dropped++;
        return LVGL_AIC_ERR_NO_MEMORY;
    }
    return LVGL_AIC_OK;
}

void lvgl_aic_get_stats(lvgl_aic_stats_t *stats)
{
    if (stats == NULL)
        return;

    memset(stats, 0, sizeof(*stats));
    lvgl_aic_display_stats(&stats->flush_count, &stats->fps,
                           &stats->display_errors, &stats->flush_ms_last,
                           &stats->flush_ms_max, &stats->flush_ms_avg);
    stats->async_dropped = g_lvgl_aic.async_dropped;
#ifdef LVGL_AIC_USE_TOUCH
    lvgl_aic_touch_stats(&stats->touch_reads, &stats->touch_events,
                         &stats->touch_errors, &stats->touch_x,
                         &stats->touch_y, &stats->touch_pressed);
#endif
}
