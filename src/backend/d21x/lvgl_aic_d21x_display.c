/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * ArtInChip D21x display backend.  Uses the BSP public display/GE interfaces,
 * not LVGL or Luban-Lite internals.
 */

#include "lvgl_aic_display_backend.h"

#include <rtthread.h>
#include <string.h>

#include "drv_disp.h"
#include "board/cache.h"
#ifdef LVGL_AIC_USE_GE_COPY
#include "drv_ge.h"
#endif

typedef struct
{
    struct aic_disp_info disp;
} d21x_display_t;

static d21x_display_t g_d21x_display;

static void d21x_copy_cpu(const void *src, int32_t src_stride, void *dst,
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

static lvgl_aic_result_t d21x_init(lvgl_aic_display_info_t *out)
{
    d21x_display_t *ctx = &g_d21x_display;

    if (out == NULL)
        return LVGL_AIC_ERR_INVALID_ARG;
    if (aic_disp_get_info(&ctx->disp) != RT_EOK)
        return LVGL_AIC_ERR_IO;

    out->width = (int32_t)ctx->disp.width;
    out->height = (int32_t)ctx->disp.height;
    out->bpp = (int32_t)ctx->disp.bpp;
    out->stride = (int32_t)ctx->disp.stride;
    out->buffer_count = (int32_t)ctx->disp.fb_count;
    if (out->buffer_count > 2)
        out->buffer_count = 2;
    {
        int active = 0;

        aic_disp_get_stats(RT_NULL, RT_NULL, &active);
        out->current_index = active;
    }
    if (out->current_index < 0 || out->current_index >= out->buffer_count)
        out->current_index = 0;
    out->buffer[0] = ctx->disp.fb[0];
    out->buffer[1] = out->buffer_count > 1 ? ctx->disp.fb[1] : NULL;
    return LVGL_AIC_OK;
}

static void d21x_deinit(void)
{
    memset(&g_d21x_display, 0, sizeof(g_d21x_display));
}

static int d21x_copy(const void *src, int32_t src_stride, void *dst,
                     int32_t dst_stride, int32_t width, int32_t height,
                     int32_t bpp)
{
#ifdef LVGL_AIC_USE_GE_COPY
    if (bpp == 32)
    {
        struct ge_bitblt blt;
        memset(&blt, 0, sizeof(blt));
        blt.src_buf.buf_type = MPP_PHY_ADDR;
        blt.src_buf.phy_addr[0] = (uintptr_t)src;
        blt.src_buf.stride[0] = (uint32_t)src_stride;
        blt.src_buf.size.width = (uint32_t)width;
        blt.src_buf.size.height = (uint32_t)height;
        blt.src_buf.format = MPP_FMT_XRGB_8888;
        blt.dst_buf = blt.src_buf;
        blt.dst_buf.phy_addr[0] = (uintptr_t)dst;
        blt.dst_buf.stride[0] = (uint32_t)dst_stride;
        if (aic_ge_bitblt(&blt) == RT_EOK)
            return 0;
    }
#else
    (void)bpp;
#endif
    d21x_copy_cpu(src, src_stride, dst, dst_stride, width, height, bpp);
    return 0;
}

static int d21x_present(int32_t index)
{
    return aic_disp_pan((int)index, 1) == RT_EOK ? 0 : -1;
}

static void d21x_cache_clean(const void *addr, size_t size)
{
    if (addr != NULL && size > 0)
        aic_dcache_clean_range((uintptr_t)addr, size);
}

static const lvgl_aic_display_backend_t g_d21x_backend =
{
    .init = d21x_init,
    .deinit = d21x_deinit,
    .copy = d21x_copy,
    .present = d21x_present,
    .cache_clean = d21x_cache_clean,
};

const lvgl_aic_display_backend_t *lvgl_aic_backend_d21x(void)
{
    return &g_d21x_backend;
}
