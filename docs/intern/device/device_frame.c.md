# src/device_frame.c

This file holds the system `frame`. At each run, it takes one step of the make, or it gets the frame of the round.
Then it writes `webgpu:Frame`.

## A run

`bridge_frame` follows `docs/extern/webgpu_device.h.md`, *The frame*:

```pseudo
FUNCTION bridge_frame(api : bridge_api_t) RETURNS I32
   d <- device_get_state()
   w <- bridge_native_get_single(api, WINDOW)
   d.api <- api
   step <- DEVICE_GONE
   IF NOT device_drain_room(d) AND has_window(w) THEN
      step <- device_make_next(d, bridge_native_get_single(api, SETTINGS), w)
   ENDIF
   IF step = DEVICE_READY THEN
      step <- keep_surface(d, w)
   ENDIF
   IF step = DEVICE_READY THEN
      step <- device_take_frame(d)
   ENDIF
   IF step = DEVICE_GONE THEN
      CALL device_drain_room(d)
      CALL device_release(d)
   ENDIF
   CALL write_frame(bridge_native_get_single(api, FRAME), d, step)
   RETURN RESULTS[step]
ENDFUNCTION
```

`device_drain_room` is step 1. It gives `true` when the device was lost: `docs/intern/device_errors.c.md`.
`has_window` makes the test of step 2.
`device_make_next` takes the steps of the make, and gives `DEVICE_READY` after them: `docs/intern/device_make.c.md`.

After the make, its check of the device is step 4, and it gives `DEVICE_GONE` for a lost device.
The second drain writes the line of a loss that the check found. Thus the line and the state of the loss come in one run.

## The surface

```pseudo
FUNCTION keep_surface(d : device_t, w : webgpu_window_t) RETURNS U32
   IF w.width = 0 OR w.height = 0 OR w.width > d.gpu.max_side OR w.height > d.gpu.max_side THEN
      RETURN DEVICE_WAITING
   ENDIF
   IF d.gpu.configure OR w.width <> d.gpu.width OR w.height <> d.gpu.height THEN
      RETURN configure(d, w)
   ENDIF
   RETURN DEVICE_READY
ENDFUNCTION
```

The first test keeps the rule of the size of `docs/extern/webgpu_device.h.md`, *The frame*.
That rule comes first, because a configure that fails stops the process: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L4036-L4077.
`configure` keeps the size and clears `configure`. It calls `wgpuSurfaceConfigure` with the device, the modes, the size and the usage `RenderAttachment`.
Then it gives the step of `device_make_targets`.

## webgpu:Frame

`write_frame` writes the state and the fields of the step, from `gpu`: `docs/intern/device_inside.h.md`, *Types*.
`target` is the view of `color` with more than 1 sample, else the view of `frame`.

## Functions

| Function | Result |
| --- | --- |
| `bridge_frame` | Takes a step of the make or the frame of the round, then writes `webgpu:Frame`. Gives 0, or 1 when a step fails or the surface gives an error. |
| `has_window` | Gives true when the app gives a window. |
| `keep_surface` | Configures the surface when its size changed, or when a status asked for it. Gives `DEVICE_WAITING` for a size that the device cannot give. |
| `configure` | Configures the surface for the size of the window, then makes the targets again. |
| `write_frame` | Writes `webgpu:Frame` for a step. |
