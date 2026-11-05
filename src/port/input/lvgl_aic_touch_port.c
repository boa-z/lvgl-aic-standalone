/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * LVGL pointer input port for RT-Thread touch devices.
 */

#include "lvgl_aic_touch_port.h"
#include "lvgl_aic_touch_state.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>

#define TOUCH_MAX_POINTS 5

typedef struct
{
    rt_device_t device;
    struct rt_touch_info info;
    struct rt_touch_data data[TOUCH_MAX_POINTS];
    lvgl_aic_touch_state_t state;
    lvgl_aic_touch_config_t config;
    rt_uint32_t reads;
    rt_uint32_t events;
    rt_uint32_t errors;
    int opened;
} touch_port_t;

static touch_port_t g_touch_port;

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    touch_port_t *port = &g_touch_port;
    rt_size_t count;
    rt_size_t i;

    (void)indev;

    if (port->device == RT_NULL)
    {
        data->point.x = 0;
        data->point.y = 0;
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    count = rt_device_read(port->device, 0, port->data, TOUCH_MAX_POINTS);
    port->reads++;

    if (count > TOUCH_MAX_POINTS)
    {
        port->errors++;
        port->state.pressed = 0;
        port->state.active_id = -1;
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            int event = LVGL_AIC_TOUCH_NONE;

            switch (port->data[i].event)
            {
            case RT_TOUCH_EVENT_DOWN:
                event = LVGL_AIC_TOUCH_DOWN;
                break;
            case RT_TOUCH_EVENT_MOVE:
                event = LVGL_AIC_TOUCH_MOVE;
                break;
            case RT_TOUCH_EVENT_UP:
                event = LVGL_AIC_TOUCH_UP;
                break;
            default:
                break;
            }

            if (lvgl_aic_touch_state_update(&port->state,
                                            port->data[i].track_id,
                                            event,
                                            port->data[i].x_coordinate,
                                            port->data[i].y_coordinate))
            {
                port->events++;
            }
        }
    }

    data->point.x = port->state.x;
    data->point.y = port->state.y;
    data->state = port->state.pressed ? LV_INDEV_STATE_PRESSED :
                                        LV_INDEV_STATE_RELEASED;
}

lvgl_aic_result_t lvgl_aic_touch_port_init(
    lv_display_t *display, const lvgl_aic_touch_config_t *config,
    lv_indev_t **indev)
{
    touch_port_t *port = &g_touch_port;
    lv_indev_t *dev;
    int32_t range_x;
    int32_t range_y;

    if (display == NULL || config == NULL || config->device_name == NULL ||
        indev == NULL)
        return LVGL_AIC_ERR_INVALID_ARG;
    if (port->device != RT_NULL)
        return LVGL_AIC_ERR_INVALID_STATE;

    memset(port, 0, sizeof(*port));
    port->config = *config;
    port->device = rt_device_find(config->device_name);
    if (port->device == RT_NULL)
        return LVGL_AIC_ERR_IO;
    if (rt_device_open(port->device, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        port->device = RT_NULL;
        return LVGL_AIC_ERR_IO;
    }
    port->opened = 1;

    memset(&port->info, 0, sizeof(port->info));
    if (rt_device_control(port->device, RT_TOUCH_CTRL_GET_INFO,
                          &port->info) != RT_EOK)
    {
        port->info.range_x = lv_display_get_horizontal_resolution(display);
        port->info.range_y = lv_display_get_vertical_resolution(display);
        port->info.point_num = TOUCH_MAX_POINTS;
    }

    range_x = port->info.range_x;
    range_y = port->info.range_y;
    if (range_x <= 1)
        range_x = lv_display_get_horizontal_resolution(display);
    if (range_y <= 1)
        range_y = lv_display_get_vertical_resolution(display);

    lvgl_aic_touch_state_init(&port->state, range_x, range_y,
                              lv_display_get_horizontal_resolution(display),
                              lv_display_get_vertical_resolution(display),
                              config->swap_xy, config->x_flip,
                              config->y_flip);

    dev = lv_indev_create();
    if (dev == NULL)
    {
        rt_device_close(port->device);
        port->device = RT_NULL;
        port->opened = 0;
        return LVGL_AIC_ERR_NO_MEMORY;
    }

    lv_indev_set_type(dev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(dev, touch_read_cb);
    lv_indev_set_display(dev, display);
    *indev = dev;
    return LVGL_AIC_OK;
}

void lvgl_aic_touch_port_deinit(lv_indev_t *indev)
{
    touch_port_t *port = &g_touch_port;

    if (indev != NULL)
    {
        lv_indev_set_read_cb(indev, NULL);
        lv_indev_delete(indev);
    }

    if (port->device != RT_NULL && port->opened)
        rt_device_close(port->device);
    memset(port, 0, sizeof(*port));
}

void lvgl_aic_touch_stats(uint32_t *reads, uint32_t *events,
                          uint32_t *errors, int16_t *x, int16_t *y,
                          uint8_t *pressed)
{
    touch_port_t *port = &g_touch_port;

    if (reads != NULL)
        *reads = port->reads;
    if (events != NULL)
        *events = port->events;
    if (errors != NULL)
        *errors = port->errors;
    if (x != NULL)
        *x = (int16_t)port->state.x;
    if (y != NULL)
        *y = (int16_t)port->state.y;
    if (pressed != NULL)
        *pressed = port->state.pressed ? 1 : 0;
}
