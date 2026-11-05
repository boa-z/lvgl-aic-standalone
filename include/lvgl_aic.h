/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * lvgl-aic-standalone public API.
 */

#ifndef LVGL_AIC_H
#define LVGL_AIC_H

#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    LVGL_AIC_OK = 0,
    LVGL_AIC_ERR_INVALID_STATE = -1,
    LVGL_AIC_ERR_INVALID_ARG = -2,
    LVGL_AIC_ERR_NO_MEMORY = -3,
    LVGL_AIC_ERR_IO = -4,
    LVGL_AIC_ERR_UNSUPPORTED = -5,
} lvgl_aic_result_t;

typedef struct
{
    const char *display_device;
    const char *touch_device;
    uint32_t ui_stack_size;
    uint8_t ui_priority;
    uint32_t ui_period_ms;
    void (*create_ui)(lv_display_t *display);
    void *user_data;
} lvgl_aic_config_t;

typedef struct
{
    uint32_t flush_count;
    uint32_t fps;
    uint32_t flush_ms_last;
    uint32_t flush_ms_max;
    uint32_t flush_ms_avg;
    uint32_t touch_reads;
    uint32_t touch_events;
    uint32_t touch_errors;
    int16_t touch_x;
    int16_t touch_y;
    uint8_t touch_pressed;
    uint32_t async_dropped;
    uint32_t display_errors;
} lvgl_aic_stats_t;

/* Initializes LVGL and the display/input ports, but does not start the UI loop. */
lvgl_aic_result_t lvgl_aic_init(const lvgl_aic_config_t *config);
/* Starts or resumes the single UI thread. */
lvgl_aic_result_t lvgl_aic_start(void);
/* Stops UI processing without destroying LVGL objects. */
void lvgl_aic_stop(void);
/* Stops the UI thread and releases all component-owned resources. */
void lvgl_aic_deinit(void);

lv_display_t *lvgl_aic_get_display(void);
lv_indev_t *lvgl_aic_get_touch_indev(void);
void lvgl_aic_get_stats(lvgl_aic_stats_t *stats);

/* Runs callback in the LVGL owner thread. */
lvgl_aic_result_t lvgl_aic_async_call(void (*callback)(void *user_data),
                                      void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_AIC_H */
