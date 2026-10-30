# src/wgpu_native/leaks.toml

This file names the objects of WebGPU that a module takes, for the count of `test --leaks` of each WebGPU module.
The part puts the file into its zip as `leaks.toml`, for `[leaks] with` of a module of another project.
`.toolchain/config/leaks.toml` names the file for the modules of this project.

## The pairs

An object that a call of `webgpu.h` or of `wgpu.h` gives starts with one reference of the module, and its `Release` call gives it back.

| Takes | Gives back |
| --- | --- |
| `wgpuCreateInstance` | `wgpuInstanceRelease` |
| `wgpuInstanceCreateSurface` | `wgpuSurfaceRelease` |
| `wgpuDeviceGetQueue` | `wgpuQueueRelease` |
| `wgpuDeviceCreateBuffer` | `wgpuBufferRelease` |
| `wgpuTextureCreateView` | `wgpuTextureViewRelease` |
| `wgpuDeviceCreateSampler` | `wgpuSamplerRelease` |
| `wgpuDeviceCreatePipelineLayout` | `wgpuPipelineLayoutRelease` |
| `wgpuDeviceCreateRenderPipeline` | `wgpuRenderPipelineRelease` |
| `wgpuDeviceCreateComputePipeline` | `wgpuComputePipelineRelease` |
| `wgpuDeviceCreateBindGroup` | `wgpuBindGroupRelease` |
| `wgpuDeviceCreateCommandEncoder` | `wgpuCommandEncoderRelease` |
| `wgpuCommandEncoderBeginRenderPass` | `wgpuRenderPassEncoderRelease` |
| `wgpuCommandEncoderBeginComputePass` | `wgpuComputePassEncoderRelease` |
| `wgpuCommandEncoderFinish` | `wgpuCommandBufferRelease` |
| `wgpuDeviceCreateQuerySet` | `wgpuQuerySetRelease` |
| `wgpuDeviceCreateRenderBundleEncoder` | `wgpuRenderBundleEncoderRelease` |
| `wgpuRenderBundleEncoderFinish` | `wgpuRenderBundleRelease` |

## The kinds

| Kind | Takes | Gives back |
| --- | --- | --- |
| `webgpu_texture` | `wgpuDeviceCreateTexture` at `result`, and `wgpuSurfaceGetCurrentTexture` at `*2.texture` | `wgpuTextureRelease` at `1` |
| `webgpu_bind_group_layout` | `wgpuDeviceCreateBindGroupLayout`, `wgpuRenderPipelineGetBindGroupLayout` and `wgpuComputePipelineGetBindGroupLayout` at `result` | `wgpuBindGroupLayoutRelease` at `1` |
| `webgpu_shader_module` | `wgpuDeviceCreateShaderModule`, and `wgpuDeviceCreateShaderModuleSpirV` and `wgpuDeviceCreateShaderModuleTrusted` of `wgpu.h`, at `result` | `wgpuShaderModuleRelease` at `1` |
| `webgpu_capabilities` | `wgpuSurfaceGetCapabilities` at `*3` | `wgpuSurfaceCapabilitiesFreeMembers` at `1` |
| `webgpu_features` | `wgpuAdapterGetFeatures` and `wgpuDeviceGetFeatures` at `*2` | `wgpuSupportedFeaturesFreeMembers` at `1` |
| `webgpu_adapter_info` | `wgpuAdapterGetInfo` at `*2` | `wgpuAdapterInfoFreeMembers` at `1` |

More than one call gives a texture, a bind group layout or a shader module, and one call gives it back. Thus each one is a kind.
The texture of a frame comes in the field `texture` of `WGPUSurfaceTexture`. The place `*<n>.<field>` of the leak count of the template reads it.
The capabilities, the features and the information of an adapter hold memory that wgpu-native allocates. A `FreeMembers` call gives it back, and gets its struct by value.

## The adapter and the device

The adapter and the device come through a callback of the module: argument 2 of `WGPURequestAdapterCallback` and of `WGPURequestDeviceCallback`.
This file cannot name a function of a module. Thus each module names its two callbacks in its own configuration, each as a take at `2`.
The part `device` names them in `.toolchain/config/device.toml`.
Its kinds are `webgpu_adapter`, which `wgpuAdapterRelease` gives back, and `webgpu_device`, which `wgpuDeviceRelease` gives back.
The device of `WGPUDeviceLostCallback` belongs to wgpu-native, so no module counts it.

## Limits

- A module calls no `AddRef` function, because the count does not see a second reference.
- The count does not see the adapters that `wgpuInstanceEnumerateAdapters` of `wgpu.h` writes into an array.
