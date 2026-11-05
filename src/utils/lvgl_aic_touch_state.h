/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LVGL_AIC_TOUCH_STATE_H
#define LVGL_AIC_TOUCH_STATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    LVGL_AIC_TOUCH_NONE = 0,
    LVGL_AIC_TOUCH_DOWN,
    LVGL_AIC_TOUCH_MOVE,
    LVGL_AIC_TOUCH_UP,
};

typedef struct
{
    int32_t range_x;
    int32_t range_y;
    int32_t width;
    int32_t height;
    int32_t swap_xy;
    int32_t x_flip;
    int32_t y_flip;
    int32_t active_id;
    int32_t pressed;
    int32_t x;
    int32_t y;
} lvgl_aic_touch_state_t;

void lvgl_aic_touch_state_init(lvgl_aic_touch_state_t *state,
                               int32_t range_x, int32_t range_y,
                               int32_t width, int32_t height,
                               int32_t swap_xy, int32_t x_flip, int32_t y_flip);
void lvgl_aic_touch_state_transform(const lvgl_aic_touch_state_t *state,
                                    int32_t raw_x, int32_t raw_y,
                                    int32_t *x, int32_t *y);
int lvgl_aic_touch_state_update(lvgl_aic_touch_state_t *state,
                                int32_t track_id, int32_t event,
                                int32_t raw_x, int32_t raw_y);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_AIC_TOUCH_STATE_H */
