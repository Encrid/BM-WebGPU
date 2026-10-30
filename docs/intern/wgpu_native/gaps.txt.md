# src/wgpu_native/gaps.txt

This file lists the calls of `webgpu.h` and `wgpu.h` that wgpu-native `v29.0.1.1` does not implement.
The zip of the part ships the file as `gaps.txt`.
`[check] refuse` of `.toolchain/project.toml` names the file, and a WebGPU module of a different project names the copy in the zip.
Then each call of the list in `src` or `include` is a fault.

## The list

The list holds the functions of the file unimplemented.rs, in the folder src of wgpu-native, at the tag `v29.0.1.1`: 35 names.
Then it holds the 5 functions of the file lib.rs of that folder whose body is `unimplemented!`: 40 names in all.
Each line holds one name, and nothing else. The order is the order of the two files.
Examples are `wgpuInstanceWaitAny`, `wgpuDeviceCreateRenderPipelineAsync`, `wgpuShaderModuleGetCompilationInfo`, each call `SetLabel`, and `wgpuBufferReadMappedRange` of lib.rs.

Such a call stops the process inside wgpu-native. Thus a case that reaches it ends with no report, and a check of the source finds it first.
A module gives each object its label in the descriptor that makes the object, since each call `SetLabel` is on the list.

## A new release of wgpu-native

When the release of wgpu-native changes, a person makes the list again from its two files.
A case compares the list with the two files at the tag of the release.
A name that leaves the list becomes a call that a module can use.
