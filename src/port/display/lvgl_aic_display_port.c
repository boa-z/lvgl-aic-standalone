/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Generic LVGL display port.  The backend owns the framebuffer and PAN/VSync
 * policy; this file only owns the LVGL display object and draw buffer.
 */

#include "lvgl_aic_display_port.h"
#include "lvgl_aic_display_state.h"

#include <rtthread.h>
#include <string.h>

typedef struct
{
    lv_display_t *display;
    const lvgl_aic_display_backend_t *backend;
    lvgl_aic_display_info_t info;
    void *draw_buf;
    uint32_t draw_buf_size;
    int32_t back_index;
    uint32_t flush_count;
    uint32_t fps;
    uint32_t fps_frames;
    uint32_t fps_start;
    uint32_t errors;
} display_port_t;

static display_port_t g_display_port;

static uint32_t display_tick_ms(void)
{
    return (uint32_t)rt_tick_get_millisecond();
}

static void display_fps_update(void)
{
    uint32_t now = display_tick_ms();

    g_display_port.fps_frames++;
    if ((uint32_t)(now - g_display_port.fps_start) >= 1000U)
    {
        g_display_port.fps = g_display_port.fps_frames;
        g_display_port.fps_frames = 0;
        g_display_port.fps_start = now;
    }
}

static void display_copy_cpu(const void *src, int32_t src_stride, void *dst,
                             int32_t dst_stride, int32_t width, int32_t height,
                             int32_t bpp)
{
    int32_t row_bytes = width * bpp / 8;
    int32_t y;

    for (y = 0; y < height; y++)
    {
        memcpy((uint8_t *)dst + y * dst_stride,
               (const uint8_t *)src + y * src_stride, (size_t)row_bytes);
    }
}

static void display_flush(lv_display_t *display, const lv_area_t *area,
                          uint8_t *px_map)
{
    display_port_t *port = &g_display_port;
    int32_t target;

    (void)area;

    if (!lv_display_flush_is_last(display))
    {
        lv_display_flush_ready(display);
        return;
    }

    if (port->backend == NULL || port->info.buffer_count <= 0 ||
        port->info.buffer[0] == NULL)
    {
        port->errors++;
        lv_display_flush_ready(display);
        return;
    }

    target = lvgl_aic_next_buffer(port->back_index, port->info.buffer_count);
    if (target < 0)
    {
        port->errors++;
        lv_display_flush_ready(display);
        return;
    }

    if (port->backend->cache_clean != NULL)
        port->backend->cache_clean(px_map, port->draw_buf_size);

    if (port->backend->copy != NULL)
    {
        if (port->backend->copy(px_map, port->info.stride,
                                port->info.buffer[target],
                                port->info.stride, port->info.width,
                                port->info.height, port->info.bpp) != 0)
        {
            port->errors++;
            lv_display_flush_ready(display);
            return;
        }
    }
    else
    {
        display_copy_cpu(px_map, port->info.stride,
                         port->info.buffer[target], port->info.stride,
                         port->info.width, port->info.height,
                         port->info.bpp);
    }

    if (port->backend->present != NULL)
    {
        if (port->backend->present(target) != 0)
        {
            port->errors++;
            lv_display_flush_ready(display);
            return;
        }
    }

    port->back_index = target;
    port->flush_count++;
    display_fps_update();
    lv_display_flush_ready(display);
}

lvgl_aic_result_t lvgl_aic_display_port_init(
    lv_display_t **display, const lvgl_aic_display_backend_t *backend)
{
    display_port_t *port = &g_display_port;
    lv_color_format_t color_format;
    lv_display_t *disp;
    int32_t bpp;

    if (display == NULL || backend == NULL || backend->init == NULL)
        return LVGL_AIC_ERR_INVALID_ARG;
    if (port->display != NULL)
        return LVGL_AIC_ERR_INVALID_STATE;

    memset(port, 0, sizeof(*port));
    port->backend = backend;

    if (backend->init(&port->info) != LVGL_AIC_OK)
        return LVGL_AIC_ERR_IO;
    if (port->info.width <= 0 || port->info.height <= 0 ||
        port->info.stride <= 0 || port->info.buffer_count <= 0)
    {
        if (backend->deinit != NULL)
            backend->deinit();
        return LVGL_AIC_ERR_IO;
    }

    bpp = port->info.bpp;
    if (bpp == 32)
        color_format = LV_COLOR_FORMAT_XRGB8888;
    else if (bpp == 16)
        color_format = LV_COLOR_FORMAT_RGB565;
    else
    {
        if (backend->deinit != NULL)
            backend->deinit();
        return LVGL_AIC_ERR_UNSUPPORTED;
    }

    port->draw_buf_size = (uint32_t)port->info.stride * port->info.height;
    port->draw_buf = rt_malloc_align(port->draw_buf_size, 64);
    if (port->draw_buf == NULL)
    {
        if (backend->deinit != NULL)
            backend->deinit();
        return LVGL_AIC_ERR_NO_MEMORY;
    }
    memset(port->draw_buf, 0, port->draw_buf_size);

    disp = lv_display_create(port->info.width, port->info.height);
    if (disp == NULL)
    {
        rt_free_align(port->draw_buf);
        port->draw_buf = NULL;
        if (backend->deinit != NULL)
            backend->deinit();
        return LVGL_AIC_ERR_NO_MEMORY;
    }

    lv_display_set_default(disp);
    lv_display_set_color_format(disp, color_format);
    lv_display_set_buffers_with_stride(disp, port->draw_buf, NULL,
                                       port->draw_buf_size,
                                       (uint32_t)port->info.stride,
                                       LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(disp, display_flush);

    port->display = disp;
    port->back_index = port->info.current_index;
    if (port->back_index < 0 || port->back_index >= port->info.buffer_count)
        port->back_index = 0;
    *display = disp;
    return LVGL_AIC_OK;
}

void lvgl_aic_display_port_deinit(lv_display_t *display)
{
    display_port_t *port = &g_display_port;

    if (display != NULL)
        lv_display_delete(display);

    if (port->backend != NULL && port->backend->deinit != NULL)
        port->backend->deinit();

    if (port->draw_buf != NULL)
    {
        rt_free_align(port->draw_buf);
        port->draw_buf = NULL;
    }
    memset(port, 0, sizeof(*port));
}

void lvgl_aic_display_stats(uint32_t *flush_count, uint32_t *fps,
                            uint32_t *errors)
{
    if (flush_count != NULL)
        *flush_count = g_display_port.flush_count;
    if (fps != NULL)
        *fps = g_display_port.fps;
    if (errors != NULL)
        *errors = g_display_port.errors;
}
