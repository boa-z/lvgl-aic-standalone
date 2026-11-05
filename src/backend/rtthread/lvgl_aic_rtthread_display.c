/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Generic RT-Thread Graphic Device backend.  This path uses one framebuffer
 * and the standard RECT_UPDATE / WAIT_VSYNC controls.
 */

#include "lvgl_aic_display_backend.h"

#include <rtthread.h>
#include <rtdevice.h>

#ifndef LVGL_AIC_DISPLAY_DEVICE
#define LVGL_AIC_DISPLAY_DEVICE "lcd"
#endif

typedef struct
{
    rt_device_t device;
    struct rt_device_graphic_info info;
    int opened;
} rtthread_display_t;

static rtthread_display_t g_rt_display;

static lvgl_aic_result_t rt_display_init(lvgl_aic_display_info_t *out)
{
    rtthread_display_t *ctx = &g_rt_display;

    if (out == NULL)
        return LVGL_AIC_ERR_INVALID_ARG;
    if (ctx->device != RT_NULL)
        return LVGL_AIC_ERR_INVALID_STATE;

    ctx->device = rt_device_find(LVGL_AIC_DISPLAY_DEVICE);
    if (ctx->device == RT_NULL)
        return LVGL_AIC_ERR_IO;
    if (rt_device_open(ctx->device, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        ctx->device = RT_NULL;
        return LVGL_AIC_ERR_IO;
    }
    ctx->opened = 1;

    if (rt_device_control(ctx->device, RTGRAPHIC_CTRL_GET_INFO,
                          &ctx->info) != RT_EOK)
    {
        rt_device_close(ctx->device);
        ctx->device = RT_NULL;
        ctx->opened = 0;
        return LVGL_AIC_ERR_IO;
    }

    out->width = ctx->info.width;
    out->height = ctx->info.height;
    out->bpp = ctx->info.bits_per_pixel;
    out->stride = ctx->info.pitch;
    out->buffer_count = 1;
    out->current_index = 0;
    out->buffer[0] = ctx->info.framebuffer;
    out->buffer[1] = NULL;
    return LVGL_AIC_OK;
}

static void rt_display_deinit(void)
{
    rtthread_display_t *ctx = &g_rt_display;

    if (ctx->device != RT_NULL && ctx->opened)
        rt_device_close(ctx->device);
    ctx->device = RT_NULL;
    ctx->opened = 0;
}

static int rt_display_present(int32_t index)
{
    rtthread_display_t *ctx = &g_rt_display;

    (void)index;
    if (ctx->device == RT_NULL)
        return -1;
    if (rt_device_control(ctx->device, RTGRAPHIC_CTRL_RECT_UPDATE,
                          RT_NULL) != RT_EOK)
        return -1;
    (void)rt_device_control(ctx->device, RTGRAPHIC_CTRL_WAIT_VSYNC, RT_NULL);
    return 0;
}

static const lvgl_aic_display_backend_t g_rt_backend =
{
    .init = rt_display_init,
    .deinit = rt_display_deinit,
    .copy = NULL,
    .present = rt_display_present,
    .cache_clean = NULL,
};

const lvgl_aic_display_backend_t *lvgl_aic_backend_rtthread(void)
{
    return &g_rt_backend;
}
