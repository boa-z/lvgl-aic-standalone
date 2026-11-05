# Validation

Status definitions:

- BUILD: compile/link passes
- HOST: host-side logic tests pass
- PACKAGE: firmware image generation passes
- HW: real board test passes
- PENDING: not yet completed or blocked by hardware

## Current matrix

| Item | Status | Evidence |
|---|---|---|
| Touch coordinate transform | HOST | `tests/host` |
| Single active touch state machine | HOST | `tests/host` |
| Display buffer rotation | HOST | `tests/host` |
| LVGL 9.6 build | BUILD | D213 BSP + component, GCC 16.1.0 |
| BSP without LVGL build | BUILD | default `.config` build |
| D21x backend build | BUILD | `LVGL_AIC_USE_D21X_BACKEND=y` |
| Generic RT graphic backend build | BUILD | `LVGL_AIC_USE_D21X_BACKEND=n` |
| GE frame copy build | BUILD | `LVGL_AIC_USE_GE_COPY=y` |
| Missing LVGL source | GUARDED | SConscript aborts with message |
| Wrong LVGL version | GUARDED | 9.5 tree rejected |
| D21x software display path | PENDING | standalone image not flashed |
| GT911 LVGL input | PENDING | standalone image not flashed |
| GE frame copy | PENDING | build only, not HW tested |
| GE Draw Unit | PENDING | not implemented in phase 1 |

The earlier D213 in-BSP LVGL port was HW validated, but that result does not
transfer automatically to the standalone component.  Flash the new image and
run `lvgl_demo`, `lvgl_demo status`, `lvgl_touch 10` before promoting HW.

## Host tests

```sh
cd tests/host
make test
```

## Integrated commands

```text
lvgl_demo
lvgl_demo status
lvgl_touch 10
lvgl_demo bench 10
lvgl_demo stop
lvgl_demo deinit
```

Record:

- flush count, FPS and flush last/avg/max time;
- touch extent for TL/TR/BL/BR/centre;
- presented FPS under `lvgl_demo bench` (static pages can show ~5 fps
  because only the 200 ms status timer invalidates them);
- display underflow/VSync counters;
- heap and stack watermarks;
- start/stop/deinit/restart repetitions.

Do not mark HW until the image has been flashed and the results recorded.
