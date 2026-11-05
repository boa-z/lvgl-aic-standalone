/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Internal display backend contract.
 */

#ifndef LVGL_AIC_DISPLAY_BACKEND_H
#define LVGL_AIC_DISPLAY_BACKEND_H

#include <stddef.h>
#include <stdint.h>

#include "lvgl_aic.h"

typedef struct
{
    int32_t width;
    int32_t height;
    int32_t bpp;
    int32_t stride;
    int32_t buffer_count;
    int32_t current_index;
    void *buffer[2];
} lvgl_aic_display_info_t;

typedef struct
{
    lvgl_aic_result_t (*init)(lvgl_aic_display_info_t *info);
    void (*deinit)(void);
    /* Optional: copy a packed LVGL frame into a backend buffer. */
    int (*copy)(const void *src, int32_t src_stride, void *dst,
                int32_t dst_stride, int32_t width, int32_t height,
                int32_t bpp);
    /* Optional: present a buffer (PAN/VSync/RECT_UPDATE). */
    int (*present)(int32_t index);
    /* Optional: clean/invalidate cache before DMA reads. */
    void (*cache_clean)(const void *addr, size_t size);
} lvgl_aic_display_backend_t;

const lvgl_aic_display_backend_t *lvgl_aic_backend_d21x(void);
const lvgl_aic_display_backend_t *lvgl_aic_backend_rtthread(void);

#endif /* LVGL_AIC_DISPLAY_BACKEND_H */
