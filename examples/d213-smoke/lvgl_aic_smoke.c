/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * D213 smoke demo for lvgl-aic-standalone.  All platform access goes through
 * the component public API; the demo does not touch BSP internals.
 */

#include "lvgl_aic.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <stdlib.h>

static lv_obj_t *g_click_label;
static lv_obj_t *g_drag_label;
static lv_obj_t *g_status_label;
static lv_obj_t *g_slider;
static lv_obj_t *g_bench_obj;
static lv_timer_t *g_bench_timer;
static rt_int32_t g_bench_dir = 1;
static rt_uint32_t g_click_count;

static void demo_click_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
        return;

    g_click_count++;
    if (g_click_label != RT_NULL)
        lv_label_set_text_fmt(g_click_label, "CLICK %u",
                              (unsigned)g_click_count);
}

static void demo_slider_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED)
        return;
    if (g_drag_label != RT_NULL && g_slider != RT_NULL)
        lv_label_set_text_fmt(g_drag_label, "DRAG %d%%",
                              (int)lv_slider_get_value(g_slider));
}

static void demo_status_timer(lv_timer_t *timer)
{
    lvgl_aic_stats_t stats;

    (void)timer;
    lvgl_aic_get_stats(&stats);

    if (g_status_label != RT_NULL)
    {
        lv_label_set_text_fmt(g_status_label,
                              "touch %4d,%4d %-8s  flush %u  fps %u",
                              (int)stats.touch_x, (int)stats.touch_y,
                              stats.touch_pressed ? "pressed" : "released",
                              (unsigned)stats.flush_count,
                              (unsigned)stats.fps);
    }
}

static void demo_bench_timer(lv_timer_t *timer)
{
    int32_t x;

    (void)timer;
    if (g_bench_obj == RT_NULL)
        return;

    x = lv_obj_get_x(g_bench_obj) + g_bench_dir * 10;
    if (x > 860)
    {
        x = 860;
        g_bench_dir = -1;
    }
    else if (x < 120)
    {
        x = 120;
        g_bench_dir = 1;
    }
    lv_obj_set_x(g_bench_obj, x);
}

static void demo_bench_start(void *user_data)
{
    (void)user_data;

    if (g_bench_obj == RT_NULL)
    {
        lv_obj_t *scr = lv_display_get_screen_active(lvgl_aic_get_display());

        g_bench_obj = lv_obj_create(scr);
        lv_obj_set_size(g_bench_obj, 48, 48);
        lv_obj_set_pos(g_bench_obj, 120, 150);
        lv_obj_set_style_radius(g_bench_obj, 24, 0);
        lv_obj_set_style_bg_color(g_bench_obj, lv_color_hex(0xffb020), 0);
        lv_obj_set_style_border_width(g_bench_obj, 0, 0);
    }

    if (g_bench_timer == RT_NULL)
        g_bench_timer = lv_timer_create(demo_bench_timer, 16, RT_NULL);
}

static void demo_bench_stop(void *user_data)
{
    (void)user_data;

    if (g_bench_timer != RT_NULL)
    {
        lv_timer_delete(g_bench_timer);
        g_bench_timer = RT_NULL;
    }
}

static lv_obj_t *demo_hint(lv_obj_t *parent, const char *text,
                           lv_align_t align, int32_t x, int32_t y,
                           uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_align(label, align, x, y);
    return label;
}

static void demo_create_ui(lv_display_t *display)
{
    lv_obj_t *scr = lv_display_get_screen_active(display);
    lv_obj_t *title;
    lv_obj_t *subtitle;
    lv_obj_t *button;
    lv_obj_t *cross;

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(scr, 4, 0);
    lv_obj_set_style_border_color(scr, lv_color_hex(0x40a0ff), 0);
    lv_obj_set_scrollable(scr, false);

    title = lv_label_create(scr);
    lv_label_set_text(title, "D213  LVGL 9.6.0");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 24);

    subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "lvgl-aic-standalone / DE / GT911");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xa9d0ff), 0);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);

    demo_hint(scr, "TL", LV_ALIGN_TOP_LEFT, 20, 76, 0x40c060);
    demo_hint(scr, "TR", LV_ALIGN_TOP_RIGHT, -20, 76, 0xe0a030);
    demo_hint(scr, "BL", LV_ALIGN_BOTTOM_LEFT, 20, -56, 0xe06090);
    demo_hint(scr, "BR", LV_ALIGN_BOTTOM_RIGHT, -20, -56, 0x40a0ff);

    cross = lv_obj_create(scr);
    lv_obj_remove_style_all(cross);
    lv_obj_set_size(cross, 80, 2);
    lv_obj_set_style_bg_color(cross, lv_color_hex(0x6080a0), 0);
    lv_obj_set_style_bg_opa(cross, LV_OPA_COVER, 0);
    lv_obj_center(cross);

    cross = lv_obj_create(scr);
    lv_obj_remove_style_all(cross);
    lv_obj_set_size(cross, 2, 80);
    lv_obj_set_style_bg_color(cross, lv_color_hex(0x6080a0), 0);
    lv_obj_set_style_bg_opa(cross, LV_OPA_COVER, 0);
    lv_obj_center(cross);

    button = lv_button_create(scr);
    lv_obj_set_size(button, 220, 72);
    lv_obj_align(button, LV_ALIGN_CENTER, 0, -80);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x1f6feb), 0);
    lv_obj_add_event_cb(button, demo_click_cb, LV_EVENT_CLICKED, RT_NULL);

    g_click_label = lv_label_create(button);
    lv_label_set_text(g_click_label, "CLICK 0");
    lv_obj_set_style_text_color(g_click_label, lv_color_white(), 0);
    lv_obj_center(g_click_label);

    g_slider = lv_slider_create(scr);
    lv_obj_set_width(g_slider, 760);
    lv_slider_set_range(g_slider, 0, 100);
    lv_slider_set_value(g_slider, 50, LV_ANIM_OFF);
    lv_obj_align(g_slider, LV_ALIGN_BOTTOM_MID, 0, -90);
    lv_obj_add_event_cb(g_slider, demo_slider_cb, LV_EVENT_VALUE_CHANGED,
                        RT_NULL);

    g_drag_label = lv_label_create(scr);
    lv_label_set_text(g_drag_label, "DRAG 50%");
    lv_obj_set_style_text_color(g_drag_label, lv_color_white(), 0);
    lv_obj_align(g_drag_label, LV_ALIGN_BOTTOM_MID, 0, -50);

    g_status_label = lv_label_create(scr);
    lv_label_set_text(g_status_label, "touch   ---,  ---  released");
    lv_obj_set_style_text_color(g_status_label, lv_color_hex(0xa9d0ff), 0);
    lv_obj_align(g_status_label, LV_ALIGN_BOTTOM_MID, 0, -12);

    lv_timer_create(demo_status_timer, 200, RT_NULL);
}

static int demo_ensure_started(void)
{
    lvgl_aic_config_t config;
    lvgl_aic_result_t result;

    if (lvgl_aic_get_display() == RT_NULL)
    {
        rt_memset(&config, 0, sizeof(config));
        config.create_ui = demo_create_ui;
        result = lvgl_aic_init(&config);
        if (result != LVGL_AIC_OK)
            return (int)result;
    }

    return (int)lvgl_aic_start();
}

static void cmd_lvgl_demo(int argc, char **argv)
{
    lvgl_aic_stats_t stats;

    if (argc > 1 && !rt_strcmp(argv[1], "stop"))
    {
        lvgl_aic_stop();
        rt_kprintf("lvgl: stopped\n");
        return;
    }

    if (argc > 1 && !rt_strcmp(argv[1], "deinit"))
    {
        lvgl_aic_deinit();
        rt_kprintf("lvgl: deinitialized\n");
        return;
    }

    if (argc > 1 && !rt_strcmp(argv[1], "status"))
    {
        lvgl_aic_get_stats(&stats);
        rt_kprintf("lvgl: flush=%u fps=%u flush_ms=%u/%u/%u "
                   "touch=%u/%u err=%u pos=%d,%d %s\n",
                   (unsigned)stats.flush_count, (unsigned)stats.fps,
                   (unsigned)stats.flush_ms_last,
                   (unsigned)stats.flush_ms_avg,
                   (unsigned)stats.flush_ms_max,
                   (unsigned)stats.touch_reads, (unsigned)stats.touch_events,
                   (unsigned)stats.touch_errors,
                   (int)stats.touch_x, (int)stats.touch_y,
                   stats.touch_pressed ? "pressed" : "released");
        return;
    }

    if (argc > 1 && !rt_strcmp(argv[1], "bench"))
    {
        lvgl_aic_stats_t before;
        lvgl_aic_stats_t after;
        rt_uint32_t seconds = argc > 2 ?
            (rt_uint32_t)strtoul(argv[2], RT_NULL, 0) : 5;
        rt_tick_t start;
        rt_tick_t elapsed;

        if (seconds == 0 || seconds > 60)
            seconds = 5;
        if (demo_ensure_started() != LVGL_AIC_OK)
        {
            rt_kprintf("lvgl: start failed\n");
            return;
        }

        lvgl_aic_get_stats(&before);
        (void)lvgl_aic_async_call(demo_bench_start, RT_NULL);
        rt_thread_mdelay(50);
        start = rt_tick_get();
        rt_thread_mdelay(seconds * RT_TICK_PER_SECOND);
        elapsed = rt_tick_get() - start;
        (void)lvgl_aic_async_call(demo_bench_stop, RT_NULL);
        rt_thread_mdelay(50);
        lvgl_aic_get_stats(&after);

        rt_kprintf("lvgl bench: %u s, flushes=%u, fps~%u, "
                   "flush_ms last/avg/max=%u/%u/%u\n",
                   (unsigned)seconds,
                   (unsigned)(after.flush_count - before.flush_count),
                   elapsed ? (unsigned)((after.flush_count -
                       before.flush_count) * RT_TICK_PER_SECOND / elapsed) : 0,
                   (unsigned)after.flush_ms_last,
                   (unsigned)after.flush_ms_avg,
                   (unsigned)after.flush_ms_max);
        return;
    }

    if (demo_ensure_started() == LVGL_AIC_OK)
        rt_kprintf("lvgl: started\n");
    else
        rt_kprintf("lvgl: start failed\n");
}
MSH_CMD_EXPORT_ALIAS(cmd_lvgl_demo, lvgl_demo,
                     start/stop/status/deinit the LVGL 9.6 smoke demo);

static void cmd_lvgl_touch(int argc, char **argv)
{
    lvgl_aic_stats_t stats;
    rt_uint32_t seconds = 10;
    rt_tick_t stop;
    int32_t min_x = 32767, min_y = 32767;
    int32_t max_x = -32768, max_y = -32768;
    rt_uint32_t samples = 0;

    if (lvgl_aic_get_touch_indev() == RT_NULL)
    {
        rt_kprintf("lvgl_touch: run lvgl_demo first\n");
        return;
    }

    if (argc > 1)
        seconds = (rt_uint32_t)strtoul(argv[1], RT_NULL, 0);
    if (seconds == 0 || seconds > 60)
        seconds = 10;

    rt_kprintf("lvgl_touch: touch the panel for %u s "
               "(TL/TR/BL/BR/centre)\n", seconds);
    stop = rt_tick_get() + seconds * RT_TICK_PER_SECOND;

    while (rt_tick_get() < stop)
    {
        lvgl_aic_get_stats(&stats);
        if (stats.touch_pressed)
        {
            if (stats.touch_x < min_x) min_x = stats.touch_x;
            if (stats.touch_x > max_x) max_x = stats.touch_x;
            if (stats.touch_y < min_y) min_y = stats.touch_y;
            if (stats.touch_y > max_y) max_y = stats.touch_y;
            samples++;
        }
        rt_thread_mdelay(20);
    }

    lvgl_aic_get_stats(&stats);
    rt_kprintf("lvgl_touch: events=%u samples=%u",
               (unsigned)stats.touch_events, (unsigned)samples);
    if (samples)
        rt_kprintf(" extent x=%d..%d y=%d..%d",
                   (int)min_x, (int)max_x, (int)min_y, (int)max_y);
    rt_kprintf(" last x=%d y=%d\n", (int)stats.touch_x, (int)stats.touch_y);
}
MSH_CMD_EXPORT_ALIAS(cmd_lvgl_touch, lvgl_touch,
                     verify LVGL touch coordinates);
