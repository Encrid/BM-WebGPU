# src/device_make.c

This file takes the steps of the make: `docs/extern/webgpu_device.h.md`, *The make*.
It also makes the instance and the surface, and releases each object.

## The steps

wgpu-native does not implement `wgpuInstanceWaitAny`, so no step waits for an answer: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/unimplemented.rs#L226-L234.
Each request has the callback mode `WGPUCallbackMode_AllowProcessEvents`, so its callback comes on the thread of `frame`.
wgpu-native calls it inside the request, so a request sets `waiting` before its call: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L2849-L2932.

```pseudo
FUNCTION device_make_next(d : device_t, s : webgpu_device_settings_t, w : webgpu_window_t) RETURNS U32
   step <- DEVICE_MAKING
   IF d.gpu.instance <> 0 THEN
      CALL wgpuInstanceProcessEvents(d.gpu.instance)
   ENDIF
   IF NOT d.gpu.waiting AND NOT d.gpu.failed THEN
      CASE OF d.gpu.next
         NEXT_ADAPTER : step <- device_ask_adapter(d, s, w)
         NEXT_DEVICE : step <- device_ask_device(d, s)
         NEXT_KEEP : step <- keep_device(d, w)
      ENDCASE
   ENDIF
   IF d.gpu.failed THEN
      CALL device_release(d)
      step <- DEVICE_FAILED
   ENDIF
   RETURN step
ENDFUNCTION
```

| `next` | Function | Steps of the make |
| --- | --- | --- |
| `NEXT_ADAPTER` | `device_ask_adapter` | Step 1: `docs/intern/device_ask.c.md`. |
| `NEXT_DEVICE` | `device_ask_device` | Step 2. |
| `NEXT_KEEP` | `keep_device` | Steps 3 and 4, then each run after the make. |

A step or a callback of the make that fails writes its line, and sets `failed`. Then `device_make_next` releases each object, and the next run takes step 1 again.

## After the device

`keep_device` gives `DEVICE_MAKING` while it takes step 3 or step 4, and `DEVICE_READY` after them:

```pseudo
FUNCTION keep_device(d : device_t, w : webgpu_window_t) RETURNS U32
   IF d.gpu.queue = 0 THEN
      d.gpu.queue <- wgpuDeviceGetQueue(d.gpu.device)
      CALL wgpuDeviceGetLimits(d.gpu.device, limits)
      d.gpu.max_side <- limits.maxTextureDimension2D
      RETURN DEVICE_MAKING
   ENDIF
   IF NOT d.gpu.checked THEN
      d.gpu.width <- 1
      d.gpu.height <- 1
      RETURN device_make_targets(d)
   ENDIF
   IF d.gpu.surface = 0 AND NOT device_make_surface(d, w) THEN
      RETURN DEVICE_NO_FRAME
   ENDIF
   IF device_is_lost(d) THEN
      RETURN DEVICE_GONE
   ENDIF
   RETURN DEVICE_READY
ENDFUNCTION
```

Step 4 makes the targets at the size 1 by 1, in an error scope: `docs/intern/device_targets.c.md`, *The check of the samples*.
After a lost surface, `keep_device` makes the surface again. If the window has no handle then, the system gives 1, and the device stays.
Then `keep_device` checks the device, before each configure and each texture of the frame: `docs/intern/device_present.c.md`, *The check of the device*.

## The surface

`device_make_surface` makes the instance if there is none. Then it makes the surface of the window, with the label `webgpu:device surface`, and sets `configure`.
`set_source` fills the chained struct of the kind of the window. The build of each system compiles only its kinds:

| System | Kind | Chained struct | Handles that the check reads |
| --- | --- | --- | --- |
| Windows | `WEBGPU_SURFACE_WINDOWS` | `WGPUSurfaceSourceWindowsHWND` | `hwnd` |
| macOS | `WEBGPU_SURFACE_METAL` | `WGPUSurfaceSourceMetalLayer` | `layer` |
| Linux | `WEBGPU_SURFACE_X11` | `WGPUSurfaceSourceXlibWindow` | `x11_display`, `x11_window` |
| Linux | `WEBGPU_SURFACE_WAYLAND` | `WGPUSurfaceSourceWaylandSurface` | `wl_display`, `wl_surface` |

wgpu-native stops the process for a kind that its build does not know, and it uses a handle with no check: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L1863-L1942.
So the check comes before each call. A failed check writes the first reason of `no surface` with `device_say_no_surface`: `docs/extern/webgpu_device.h.md`, *Messages*.
Then `device_make_surface` gives false, and its caller acts on it.
On Wayland, the instance gets the display of the window in `WGPUInstanceExtras`. The GLES backend needs it to present: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/ffi/wgpu.h#L1102-L1112.

## The release

`device_release` releases each object that is not 0, then sets `gpu` to 0.
The order is the frame, the targets, the surface, the queue, the device, the adapter and the instance.
wgpu-native stops the process at a release of 0, for example in `wgpuDeviceRelease`: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L2778-L2781.
The release of the last reference of the device waits until the GPU is idle. It calls no callback of a lost device: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L184-L198.

## Functions

| Function | Result |
| --- | --- |
| `device_make_next` | Takes the next step of the make. Gives a `device_step_t`. |
| `device_make_surface` | Makes the instance if there is none, then the surface. Gives false, after its line, for a window with no handle of this system. |
| `set_source` | Fills the chained struct of the surface. Gives false for another kind, or for a handle at 0. |
| `keep_device` | Takes steps 3 and 4 of the make, makes a lost surface again, and checks the device. Gives `DEVICE_GONE` for a lost device, and `DEVICE_READY` after the make. |
| `device_release` | Releases each object, and sets `gpu` to 0. |
