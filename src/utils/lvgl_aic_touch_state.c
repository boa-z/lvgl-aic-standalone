/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Pure touch state machine and coordinate mapping.  Kept independent of LVGL
 * and RT-Thread so it can be covered by host tests.
 */

#include "lvgl_aic_touch_state.h"

static int32_t scale_coord(int32_t value, int32_t source_range,
                           int32_t target_range)
{
    if (source_range <= 1 || target_range <= 1)
        return value;
    return ((int64_t)value * (target_range - 1)) / (source_range - 1);
}

static int32_t clamp_coord(int32_t value, int32_t max)
{
    if (value < 0)
        return 0;
    if (value >= max)
        return max > 0 ? max - 1 : 0;
    return value;
}

void lvgl_aic_touch_state_init(lvgl_aic_touch_state_t *state,
                               int32_t range_x, int32_t range_y,
                               int32_t width, int32_t height,
                               int32_t swap_xy, int32_t x_flip, int32_t y_flip)
{
    if (state == 0)
        return;

    state->range_x = range_x;
    state->range_y = range_y;
    state->width = width;
    state->height = height;
    state->swap_xy = swap_xy;
    state->x_flip = x_flip;
    state->y_flip = y_flip;
    state->active_id = -1;
    state->pressed = 0;
    state->x = 0;
    state->y = 0;
}

void lvgl_aic_touch_state_transform(const lvgl_aic_touch_state_t *state,
                                    int32_t raw_x, int32_t raw_y,
                                    int32_t *x, int32_t *y)
{
    int32_t source_x;
    int32_t source_y;
    int32_t tx;
    int32_t ty;

    if (state == 0)
        return;

    source_x = state->range_x > 1 ? state->range_x : state->width;
    source_y = state->range_y > 1 ? state->range_y : state->height;
    tx = raw_x;
    ty = raw_y;

    if (state->swap_xy)
    {
        int32_t tmp = tx;
        int32_t tmp_range = source_x;

        tx = ty;
        ty = tmp;
        source_x = source_y;
        source_y = tmp_range;
    }

    tx = scale_coord(tx, source_x, state->width);
    ty = scale_coord(ty, source_y, state->height);

    if (state->x_flip)
        tx = state->width - 1 - tx;
    if (state->y_flip)
        ty = state->height - 1 - ty;

    tx = clamp_coord(tx, state->width);
    ty = clamp_coord(ty, state->height);

    if (x != 0)
        *x = tx;
    if (y != 0)
        *y = ty;
}

int lvgl_aic_touch_state_update(lvgl_aic_touch_state_t *state,
                                int32_t track_id, int32_t event,
                                int32_t raw_x, int32_t raw_y)
{
    int32_t x;
    int32_t y;

    if (state == 0)
        return 0;
    if (event == LVGL_AIC_TOUCH_NONE)
        return 0;

    if (event == LVGL_AIC_TOUCH_DOWN || event == LVGL_AIC_TOUCH_MOVE)
    {
        if (state->active_id < 0)
            state->active_id = track_id;
        if (track_id != state->active_id)
            return 0;

        lvgl_aic_touch_state_transform(state, raw_x, raw_y, &x, &y);
        state->x = x;
        state->y = y;
        state->pressed = 1;
        return 1;
    }

    if (event == LVGL_AIC_TOUCH_UP)
    {
        if (state->active_id >= 0 && track_id != state->active_id)
            return 0;

        lvgl_aic_touch_state_transform(state, raw_x, raw_y, &x, &y);
        state->x = x;
        state->y = y;
        state->pressed = 0;
        state->active_id = -1;
        return 1;
    }

    return 0;
}
