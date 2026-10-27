# src/gaps.txt

This file lists the calls of `webgpu.h` and `wgpu.h` that wgpu-native `v29.0.1.1` does not implement.
A release ships it, and the check `[check] refuse` of a WebGPU module reads it. Each call of the list in `src` or `include` of the module is a fault.

## The list

The list holds the functions of the file unimplemented.rs, in the folder src of wgpu-native, at the tag `v29.0.1.1`: 35 names.
Each line holds one name, and nothing else. The order is the order of that file.
Examples are `wgpuInstanceWaitAny`, `wgpuDeviceCreateRenderPipelineAsync`, `wgpuShaderModuleGetCompilationInfo`, and each call `SetLabel`.

Such a call stops the process inside wgpu-native. Thus a case that reaches it ends with no report, and a check of the source finds it first.
A module gives each object its label in the descriptor that makes the object, since each call `SetLabel` is on the list.

## A new release of wgpu-native

When the release of wgpu-native changes, the list is made again from its file unimplemented.rs.
A name that leaves the list becomes a call that a module can use.
