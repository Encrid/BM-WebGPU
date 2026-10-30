# src/device_source.c

This file fills the source of the surface for the kind of the window.
The source is the chained struct of `webgpu.h`, and on Wayland also the display in the extras of the instance.

## The kinds

`device_set_source` gives the window to the function of its kind. The build of each system compiles only its kinds:

| System | Kind | Function | Chained struct | Handles that the check reads |
| --- | --- | --- | --- | --- |
| Windows | `WEBGPU_SURFACE_WINDOWS` | `set_windows_source` | `WGPUSurfaceSourceWindowsHWND` | `hwnd` |
| macOS | `WEBGPU_SURFACE_METAL` | `set_metal_source` | `WGPUSurfaceSourceMetalLayer` | `layer` |
| Linux | `WEBGPU_SURFACE_X11` | `set_x11_source` | `WGPUSurfaceSourceXlibWindow` | `x11_display`, `x11_window` |
| Linux | `WEBGPU_SURFACE_WAYLAND` | `set_wayland_source` | `WGPUSurfaceSourceWaylandSurface` | `wl_display`, `wl_surface` |

Each function fills the chained struct, then gives true when each handle of the check is not 0.
A kind of another system, or a value that names no kind, gives false.
wgpu-native stops the process for a kind that its build does not know, and it uses a handle with no check: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L1863-L1942.
So the check comes before each call that makes a surface.

## Wayland

On Wayland, the instance gets the display of the window in `WGPUInstanceExtras`. The GLES backend needs it to present: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/ffi/wgpu.h#L1102-L1112.
`set_wayland_source` writes the display into the extras. `device_set_instance` writes the other fields of the extras from the settings, and chains them.
For each other kind, the display of the extras stays `WGPUNativeDisplayHandleType_None`.

## Functions

| Function | Result |
| --- | --- |
| `device_set_source` | Fills the source for the kind of the window. Gives false for another kind, or for a handle at 0. |
| `set_windows_source` | Fills the source of a window of Windows. Gives false for an `hwnd` at 0. |
| `set_metal_source` | Fills the source of a `CAMetalLayer`. Gives false for a `layer` at 0. |
| `set_x11_source` | Fills the source of a window of X11. Gives false for an `x11_display` or an `x11_window` at 0. |
| `set_wayland_source` | Fills the source and the display of the instance for a window of Wayland. Gives false for a `wl_display` or a `wl_surface` at 0. |
