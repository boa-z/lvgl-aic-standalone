/*
 * Copyright (c) 2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * LVGL 9.6 configuration for lvgl-aic-standalone.
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <rtconfig.h>

#define LV_COLOR_FORMAT_DEFAULT     LV_COLOR_FORMAT_XRGB8888
#define LV_DPI_DEF                  130
#define LV_DEF_REFR_PERIOD          16

#define LV_USE_OS                   LV_OS_RTTHREAD
#define LV_USE_STDLIB_MALLOC        LV_STDLIB_RTTHREAD
#define LV_USE_STDLIB_STRING        LV_STDLIB_RTTHREAD
#define LV_USE_STDLIB_SPRINTF       LV_STDLIB_RTTHREAD

#define LV_USE_LOG                  0
#define LV_DISABLE_ASSERT_HANDLER_INCLUDE_WARNING 1
#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1
#define LV_USE_ASSERT_OBJ           1
#define LV_USE_ASSERT_STYLE         0

#define LV_USE_PERF_MONITOR         0
#define LV_USE_MEM_MONITOR          0
#define LV_USE_SYSMON               0

#define LV_USE_FS_STDIO             0
#define LV_USE_FS_POSIX             0
#define LV_USE_FS_WIN32             0
#define LV_USE_FS_FATFS             0
#define LV_USE_FS_LITTLEFS          0

#define LV_USE_DRAW_SW              1
#define LV_DRAW_SW_DRAW_UNIT_CNT    1
#define LV_USE_DRAW_SW_ASM          LV_DRAW_SW_ASM_NONE
#define LV_DRAW_SW_SUPPORT_ARGB8888 1
#define LV_DRAW_SW_SUPPORT_XRGB8888 1
#define LV_DRAW_SW_SUPPORT_RGB565   1

#define LV_DRAW_BUF_STRIDE_ALIGN    64
#define LV_DRAW_BUF_ALIGN           64

#define LV_USE_FLOAT                0
#define LV_USE_MATRIX               0

#define LV_FONT_MONTSERRAT_14       1
#define LV_FONT_MONTSERRAT_20       1
#define LV_FONT_MONTSERRAT_28       1
#define LV_FONT_DEFAULT             &lv_font_montserrat_14

#define LV_USE_THEME_DEFAULT        1
#define LV_USE_LABEL                1
#define LV_USE_BUTTON               1
#define LV_USE_BAR                  1
#define LV_USE_SLIDER               1
#define LV_USE_FLEX                 1
#define LV_USE_GRID                 1

#endif /* LV_CONF_H */
