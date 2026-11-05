# lvgl-aic-standalone

Native RT-Thread adaptation for LVGL 9.6.x, separate from the Luban-Lite
`lvgl-aic` component.  It owns LVGL runtime, display/input ports, optional
ArtInChip GE frame copy and a small D213 smoke demo.  It does not own the BSP
hardware drivers.

## Fixed dependencies

- LVGL: official v9.6.0
  - tag: `v9.6.0`
  - commit: `80ca777e37a2b176770726a02e07a6fb79ef0b39`
  - source is external and must not be modified
- RT-Thread: BSP/app supplied
- ArtInChip BSP: public `drv_disp.h`, `drv_ge.h`, `board/cache.h` only for the
  optional D21x backend

## Repository layout

```text
include/                  public lvgl_aic.h
src/runtime/              LVGL lifecycle, UI thread, async calls
src/port/display/         generic LVGL display port + backend contract
src/port/input/           RT-Thread Touch Device pointer port
src/backend/d21x/         D21x double-buffer/PAN/VSync/GE frame copy backend
src/backend/rtthread/     generic single-framebuffer RT-Thread backend
src/utils/                host-testable touch/display state logic
examples/d213-smoke/      MSH smoke demo
tests/host/               pure logic host tests
docs/                     architecture, migration matrix, validation
```

## Integration

The application provides both external source trees:

```sh
export LVGL_DIR=/path/to/lvgl-9.6.0
export LVGL_AIC_DIR=/path/to/lvgl-aic-standalone
export LVGL_AIC_BSP_DIR=/path/to/bsp/artinchip/d213-demo128-nand
```

BSP/app Kconfig options:

```text
LVGL_AIC_USE_D21X_BACKEND=y
LVGL_AIC_USE_GE_COPY=n
LVGL_AIC_USE_TOUCH=y
LVGL_AIC_TOUCH_DEVICE="gt911"
LVGL_AIC_TOUCH_SWAP_XY=n
LVGL_AIC_TOUCH_X_FLIP=n
LVGL_AIC_TOUCH_Y_FLIP=n
LVGL_AIC_DEMO_SMOKE=y
LVGL_AIC_UI_STACK_SIZE=32768
LVGL_AIC_UI_PRIORITY=15
LVGL_AIC_UI_PERIOD_MS=10
```

The component SConscript builds unmodified LVGL under
`build/lvgl_aic/lvgl_ext`.

## Public API

```c
#include "lvgl_aic.h"

static void create_ui(lv_display_t *display)
{
    /* Application-owned pages. */
}

lvgl_aic_config_t cfg = {0};
cfg.create_ui = create_ui;

lvgl_aic_init(&cfg);
lvgl_aic_start();
/* ... */
lvgl_aic_stop();
lvgl_aic_deinit();
```

`lvgl_aic_async_call()` schedules a callback in the single LVGL owner thread.
The runtime does not expose BSP globals or device internals to the UI.

## Demo

If `LVGL_AIC_DEMO_SMOKE=y`:

```text
msh > lvgl_demo
msh > lvgl_demo status
msh > lvgl_touch 10
msh > lvgl_demo bench 10
msh > lvgl_demo stop
msh > lvgl_demo deinit
```

The page verifies display, click, drag and touch direction.  It uses only the
public component API.  `status` reports last/average/maximum flush time.
`bench` animates a small object and reports the presented-frame rate; a static
page can legitimately show ~5 fps because only the 200 ms status timer
invalidates it.

## Tests

```sh
cd tests/host
make test
```

## Status

- BUILD: PASS (D213 BSP + component, macOS GCC 16.1.0)
- HOST: PASS (`tests/host`)
- PACKAGE: PASS (`rtthread.bin` 589544, ITB 591872, IMG 1441280)
- HW: PENDING for this standalone image (the earlier in-BSP port was HW
  validated; it is not transferred to the standalone component)

See `docs/validation.md` for the exact matrix.  Do not treat build-only
results as hardware validation.

## License

New code in this repository is Apache-2.0.  LVGL remains MIT and is consumed
as an external dependency.  See `NOTICE.md`.
