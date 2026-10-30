# src/device_instance.c

This file fills the extras of the instance from the settings: `docs/extern/webgpu_device.h.md`, *The instance*.

## The extras

`device_set_instance` writes each field of `WGPUInstanceExtras` from its setting. Then it chains the extras to the descriptor of the instance.
It does not write the display, because `set_wayland_source` writes it: `docs/intern/device_source.c.md`.

| Field of `WGPUInstanceExtras` | Value |
| --- | --- |
| `backends` | `instance_backends`, which has the bits of `wgpu.h`. |
| `flags` | `instance_flags`, which has the bits of `wgpu.h`. |
| `dx12ShaderCompiler` | `d3d12_compiler`, less 1. |
| `gles3MinorVersion` | `gles_version`, less 1. |
| `glFenceBehaviour` | `gl_fence`, less 1. |
| `dxcPath` | `dxc_path`, with the length that `get_path_length` gives. |
| `dx12PresentationSystem` | `d3d12_swapchain`, which has the values of `wgpu.h`. |
| `budgetForDeviceCreation` | The address of `creation_threshold`, or null for 0. |
| `budgetForDeviceLoss` | The address of `loss_threshold`, or null for 0. |

The two addresses and the text of `dxcPath` point into the settings. wgpu-native copies their values in `wgpuCreateInstance`.
Thus they must be valid only during that call.
https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L371-L423 shows the copy of each field.

## The path of DXC

`get_path_length` gives the bytes of `dxc_path` before its first zero. A system can write 1024 bytes with no zero, and then it gives 1024.
Thus the text of `dxcPath` is never null, and its length is never `WGPU_STRLEN`.

With a null text and that length, wgpu-native asks for a DXC that its release does not hold, and stops the process.
https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L376-L385 and https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/dx12/shader_compilation.rs#L70-L93 show it.
An empty `dxc_path` names no library. Thus with `WEBGPU_D3D12_COMPILER_DXC`, D3D12 does not start.

## Functions

| Function | Result |
| --- | --- |
| `device_set_instance` | Fills the extras of the instance from the settings, and chains them to the descriptor of the instance. |
| `get_threshold` | Gives the address of a percent, or null for 0. |
| `get_path_length` | Gives the length of a path: the bytes before its first zero, or the size of its room. |
