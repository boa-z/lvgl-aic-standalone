# NOTICE

This repository does not relicense LVGL or ArtInChip vendor code.

## LVGL

- Source: https://github.com/lvgl/lvgl
- Version: v9.6.0
- Commit: 80ca777e37a2b176770726a02e07a6fb79ef0b39
- License: MIT

LVGL is an external dependency and is not stored in this repository.

## ArtInChip reference material

The initial standalone port was informed by:

- RT-Thread D213 BSP: https://github.com/boa-z/rt-thread
  branch `bsp/artinchip-d21x`, `bsp/artinchip/d213-demo128-nand/lvgl/`
- Luban-Lite LVGL adaptation: https://github.com/boa-z/lvgl-aic
  branch `main`
- ArtInChip Luban-Lite / D21x public display, GE and touch interfaces

New standalone runtime/port code is Apache-2.0.  Any source copied from
ArtInChip or LVGL in the future must retain its original copyright, SPDX
identifier and author notices, and must not be relicensed implicitly.

No BSP binary, bootloader, proprietary firmware or vendor asset is included
in this repository.
