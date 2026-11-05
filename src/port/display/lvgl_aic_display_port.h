/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LVGL_AIC_DISPLAY_PORT_H
#define LVGL_AIC_DISPLAY_PORT_H

#include "lvgl_aic.h"
#include "lvgl_aic_display_backend.h"

#ifdef __cplusplus
extern "C" {
#endif

lvgl_aic_result_t lvgl_aic_display_port_init(
    lv_display_t **display, const lvgl_aic_display_backend_t *backend);
void lvgl_aic_display_port_deinit(lv_display_t *display);
void lvgl_aic_display_stats(uint32_t *flush_count, uint32_t *fps,
                            uint32_t *errors, uint32_t *flush_ms_last,
                            uint32_t *flush_ms_max, uint32_t *flush_ms_avg);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_AIC_DISPLAY_PORT_H */
