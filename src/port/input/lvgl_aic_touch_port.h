/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LVGL_AIC_TOUCH_PORT_H
#define LVGL_AIC_TOUCH_PORT_H

#include "lvgl_aic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char *device_name;
    int swap_xy;
    int x_flip;
    int y_flip;
    uint32_t poll_ms;
} lvgl_aic_touch_config_t;

lvgl_aic_result_t lvgl_aic_touch_port_init(
    lv_display_t *display, const lvgl_aic_touch_config_t *config,
    lv_indev_t **indev);
void lvgl_aic_touch_port_deinit(lv_indev_t *indev);
void lvgl_aic_touch_stats(uint32_t *reads, uint32_t *events,
                          uint32_t *errors, int16_t *x, int16_t *y,
                          uint8_t *pressed);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_AIC_TOUCH_PORT_H */
