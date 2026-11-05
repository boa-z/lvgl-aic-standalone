/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "lvgl_aic_display_state.h"

int32_t lvgl_aic_next_buffer(int32_t current, int32_t count)
{
    if (count <= 0)
        return -1;
    if (count == 1)
        return 0;
    if (current < 0 || current >= count)
        current = 0;
    return (current + 1) % count;
}
