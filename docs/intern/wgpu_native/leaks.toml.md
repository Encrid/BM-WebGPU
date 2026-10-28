# src/leaks.toml

This file names the objects of WebGPU that a module takes, for the count of `test --leaks` of each WebGPU module.
A release ships the file as `leaks.toml`, and each WebGPU module names it in `[leaks] with`.

## The pairs

An object that a call of `webgpu.h` gives starts with one reference of the module, and its `Release` call gives it back.

| Takes | Gives back |
| --- | --- |
| `wgpuCreateInstance` | `wgpuInstanceRelease` |
| `wgpuInstanceCreateSurface` | `wgpuSurfaceRelease` |
| `wgpuDeviceGetQueue` | `wgpuQueueRelease` |
| `wgpuDeviceCreateBuffer` | `wgpuBufferRelease` |
| `wgpuTextureCreateView` | `wgpuTextureViewRelease` |
| `wgpuDeviceCreateSampler` | `wgpuSamplerRelease` |
| `wgpuDeviceCreateShaderModule` | `wgpuShaderModuleRelease` |
| `wgpuDeviceCreateBindGroupLayout` | `wgpuBindGroupLayoutRelease` |
| `wgpuDeviceCreatePipelineLayout` | `wgpuPipelineLayoutRelease` |
| `wgpuDeviceCreateRenderPipeline` | `wgpuRenderPipelineRelease` |
| `wgpuDeviceCreateBindGroup` | `wgpuBindGroupRelease` |
| `wgpuDeviceCreateCommandEncoder` | `wgpuCommandEncoderRelease` |
| `wgpuCommandEncoderBeginRenderPass` | `wgpuRenderPassEncoderRelease` |
| `wgpuCommandEncoderFinish` | `wgpuCommandBufferRelease` |

## The kinds

| Kind | Takes | Gives back |
| --- | --- | --- |
| `webgpu_texture` | `wgpuDeviceCreateTexture` at `result`, and `wgpuSurfaceGetCurrentTexture` at `*2.texture` | `wgpuTextureRelease` at `1` |
| `webgpu_capabilities` | `wgpuSurfaceGetCapabilities` at `*3` | `wgpuSurfaceCapabilitiesFreeMembers` at `1` |
| `webgpu_features` | `wgpuAdapterGetFeatures` at `*2` | `wgpuSupportedFeaturesFreeMembers` at `1` |

A texture is a kind, because one call gives back both a texture that a module makes and the texture of a frame.
The texture of a frame comes in the field `texture` of `WGPUSurfaceTexture`. The place `*<n>.<field>` of the leak count of the template reads it.
The capabilities and the features hold arrays that wgpu-native allocates. A `FreeMembers` call gives them back, and gets its struct by value.

## The adapter and the device

The adapter and the device come through a callback of the module: argument 2 of `WGPURequestAdapterCallback` and of `WGPURequestDeviceCallback`.
This file cannot name a function of a module. Thus each module names its two callbacks in its own `leaks.toml`, each as a take at `2`.
Its kinds are `webgpu_adapter`, which `wgpuAdapterRelease` gives back, and `webgpu_device`, which `wgpuDeviceRelease` gives back.
The device of `WGPUDeviceLostCallback` belongs to wgpu-native, so no module counts it.

## Limits

- A module calls no `AddRef` function, because the count does not see a second reference.
- The two places `<n>` of a callback and `*<n>.<field>` come with the template in phase 4. Until then, the count does not see the adapter, the device and the texture of a frame.
