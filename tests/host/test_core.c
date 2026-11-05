/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Host tests for the pure logic used by lvgl-aic-standalone.
 */

#include <stdio.h>
#include <stdlib.h>

#include "lvgl_aic_touch_state.h"
#include "lvgl_aic_display_state.h"

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        return 1; \
    } \
} while (0)

static int test_identity_transform(void)
{
    lvgl_aic_touch_state_t state;
    int32_t x, y;

    lvgl_aic_touch_state_init(&state, 1024, 600, 1024, 600, 0, 0, 0);
    lvgl_aic_touch_state_transform(&state, 100, 200, &x, &y);
    CHECK(x == 100 && y == 200);
    lvgl_aic_touch_state_transform(&state, 1023, 599, &x, &y);
    CHECK(x == 1023 && y == 599);
    return 0;
}

static int test_swap_and_flip(void)
{
    lvgl_aic_touch_state_t state;
    int32_t x, y;

    lvgl_aic_touch_state_init(&state, 1024, 600, 1024, 600, 1, 1, 1);
    lvgl_aic_touch_state_transform(&state, 0, 0, &x, &y);
    CHECK(x == 1023 && y == 599);
    lvgl_aic_touch_state_transform(&state, 1023, 599, &x, &y);
    CHECK(x == 0 && y == 0);
    return 0;
}

static int test_scaling(void)
{
    lvgl_aic_touch_state_t state;
    int32_t x, y;

    lvgl_aic_touch_state_init(&state, 2048, 1200, 1024, 600, 0, 0, 0);
    lvgl_aic_touch_state_transform(&state, 0, 0, &x, &y);
    CHECK(x == 0 && y == 0);
    lvgl_aic_touch_state_transform(&state, 2047, 1199, &x, &y);
    CHECK(x == 1023 && y == 599);
    lvgl_aic_touch_state_transform(&state, 1024, 600, &x, &y);
    CHECK(x == 511 && y == 299);
    return 0;
}

static int test_single_touch_state_machine(void)
{
    lvgl_aic_touch_state_t state;

    lvgl_aic_touch_state_init(&state, 1024, 600, 1024, 600, 0, 0, 0);
    CHECK(lvgl_aic_touch_state_update(&state, 0, LVGL_AIC_TOUCH_DOWN,
                                      10, 20) == 1);
    CHECK(state.pressed && state.x == 10 && state.y == 20);
    CHECK(lvgl_aic_touch_state_update(&state, 0, LVGL_AIC_TOUCH_MOVE,
                                      30, 40) == 1);
    CHECK(state.pressed && state.x == 30 && state.y == 40);
    CHECK(lvgl_aic_touch_state_update(&state, 0, LVGL_AIC_TOUCH_UP,
                                      31, 41) == 1);
    CHECK(!state.pressed && state.x == 31 && state.y == 41);
    return 0;
}

static int test_secondary_finger_is_ignored(void)
{
    lvgl_aic_touch_state_t state;

    lvgl_aic_touch_state_init(&state, 1024, 600, 1024, 600, 0, 0, 0);
    CHECK(lvgl_aic_touch_state_update(&state, 1, LVGL_AIC_TOUCH_DOWN,
                                      100, 100) == 1);
    CHECK(lvgl_aic_touch_state_update(&state, 2, LVGL_AIC_TOUCH_DOWN,
                                      200, 200) == 0);
    CHECK(state.x == 100 && state.y == 100);
    CHECK(lvgl_aic_touch_state_update(&state, 2, LVGL_AIC_TOUCH_UP,
                                      200, 200) == 0);
    CHECK(state.pressed);
    CHECK(lvgl_aic_touch_state_update(&state, 1, LVGL_AIC_TOUCH_UP,
                                      100, 100) == 1);
    CHECK(!state.pressed);
    return 0;
}

static int test_display_buffer_index(void)
{
    CHECK(lvgl_aic_next_buffer(0, 0) == -1);
    CHECK(lvgl_aic_next_buffer(0, 1) == 0);
    CHECK(lvgl_aic_next_buffer(0, 2) == 1);
    CHECK(lvgl_aic_next_buffer(1, 2) == 0);
    CHECK(lvgl_aic_next_buffer(9, 2) == 1);
    return 0;
}

int main(void)
{
    int failures = 0;

    failures += test_identity_transform();
    failures += test_swap_and_flip();
    failures += test_scaling();
    failures += test_single_touch_state_machine();
    failures += test_secondary_finger_is_ignored();
    failures += test_display_buffer_index();

    if (failures == 0)
        printf("lvgl-aic-standalone host tests: PASS\n");
    return failures ? 1 : 0;
}
