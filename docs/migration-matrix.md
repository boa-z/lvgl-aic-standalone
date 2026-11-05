# Migration matrix

## From current D213 BSP LVGL port

| Current BSP item | Destination | Decision |
|---|---|---|
| `lvgl/lvgl_port.c` display flush | `src/port/display/lvgl_aic_display_port.c` | rewrite as backend-neutral port |
| D21x `aic_disp_*` calls | `src/backend/d21x/lvgl_aic_d21x_display.c` | keep optional BSP backend |
| current GT911 LVGL read callback | `src/port/input/lvgl_aic_touch_port.c` | rewrite against RT-Thread Touch Device |
| touch coordinate transform | `src/utils/lvgl_aic_touch_state.c` | extracted for host tests |
| `lvgl_demo` page | `examples/d213-smoke/lvgl_aic_smoke.c` | moved out of BSP |
| `lv_conf.h` | `config/lv_conf.h` | component owns LVGL policy |
| SConscript LVGL build | `SConscript` | version check + external source build |
| Kconfig demo/touch options | `Kconfig` | component-owned options |

The BSP should keep only a thin integration wrapper and hardware APIs.

## From `lvgl-aic` (Luban-Lite)

| `lvgl-aic` area | Standalone decision |
|---|---|
| `port/lv_aic_display.c` | concept reused; MPP handle replaced by BSP backend |
| `port/lv_aic_indev.c` | concept reused; OSAL worker/event path replaced by RT-Thread device polling |
| `port/lv_aic.c` | lifecycle/error-unwind concept reused; API simplified |
| `draw/ge2d/` | not migrated in phase 1; future optional draw unit |
| `image/mpp/` | not migrated; MPP is Luban-Lite-specific |
| `compat/lvgl_aic_private.h` | not migrated; no private LVGL API in phase 1 |
| `tests/host/input_contract.c` | concept reused for touch state tests |
| `tests/host/display_flush_contract.c` | concept reused for buffer-index tests |

## Deliberately not migrated

- MPP framebuffer/GE2D/CMA/OSAL.
- FreeType, image decoder, vector, APNG/GIF.
- Luban-Lite OneStep/SConscript/AIC_ROOT dependencies.
- Business pages and board-private globals.

These remain in `lvgl-aic` or become optional future backends.
