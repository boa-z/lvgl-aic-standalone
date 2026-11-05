# Architecture

## Ownership

| Area | Owner | Notes |
|---|---|---|
| CPU/SoC startup, clocks, interrupts | RT-Thread BSP | no LVGL dependency |
| Framebuffer, DE, VSync, GE, GT911 register driver | RT-Thread BSP | public hardware APIs only |
| LVGL core | official LVGL v9.6.0 | unmodified external source |
| LVGL runtime, display/input port, UI thread | lvgl-aic-standalone | this repository |
| D21x display/GE backend adapter | lvgl-aic-standalone | calls BSP public APIs |
| UI pages, business logic | application/demo | not in BSP or component core |

## Runtime model

- One UI owner thread.
- LVGL uses official `LV_OS_RTTHREAD`.
- `lv_timer_handler()` runs only in the UI thread.
- Application updates use `lvgl_aic_async_call()`; the runtime does not expose
  a second LVGL entry point.
- `lvgl_aic_init()` creates LVGL, display, touch and the parked UI thread.
- `lvgl_aic_start()`/`lvgl_aic_stop()` are idempotent.
- `lvgl_aic_deinit()` waits for the UI thread to leave LVGL, then tears down
  input, display, message queue and LVGL.

## Display port

The generic port owns the LVGL display object and a cache-aligned full-screen
draw buffer.  A backend owns framebuffer memory and presentation:

- `init()` returns width, height, bpp, stride and one/two buffers;
- `copy()` optionally uses GE for the frame copy;
- `present()` performs PAN / VSync / RECT_UPDATE;
- `cache_clean()` is called before any DMA read of the draw buffer.

Two backends are provided:

1. generic RT-Thread Graphic Device, single framebuffer;
2. D21x public `drv_disp` backend, double buffer + PAN/VSync + optional GE.

The port never assumes a virtual address is DMA-safe.  The D21x backend only
uses framebuffers explicitly returned by the BSP and keeps GE disabled by
default.

## Input port

- Uses the standard RT-Thread Touch Device interface.
- Reads DOWN/MOVE/UP events; never accesses GT911 registers.
- Tracks one active `track_id`; secondary fingers are ignored for the LVGL
  single-pointer contract.
- Coordinate scaling, swap and flips are pure logic in `src/utils/`.
- Invalid read counts release a potentially stuck pointer.
- Polling is driven by the LVGL UI loop.  IRQ mode can be added without changing
  the public API.

## GE Draw Unit

The GE Draw Unit is intentionally not part of the first standalone baseline.
The current optional GE use is a full-frame Normal-mode bitblt copy.  A future
draw unit must keep separate states for:

- unsupported task before submission;
- failed before DMA submission;
- completed DMA;
- uncertain DMA completion.

Software fallback is only allowed before submission or after confirmed
completion.  Buffers must remain owned until DMA completion is certain.
