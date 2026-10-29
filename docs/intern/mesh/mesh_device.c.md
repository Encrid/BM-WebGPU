# src/mesh_device.c

This file makes the objects of a device and releases them.
It also makes the buffers and the textures of the uploads.

## The objects of a device

`mesh_make_device` does these steps in the first ready round of a device:

1. It keeps a copy of `webgpu:Frame`: the device, the queue, the format, the samples and the depth format.
2. It keeps a copy of `webgpu:MeshSettings` as the settings of the device.
3. It reads the limits of the device with `wgpuDeviceGetLimits`.
4. `mesh_make_layouts` makes the shader module and the layouts.
5. It makes two samplers, one for each `filter`. Each one repeats or clamps as `repeat` tells.
6. `mesh_make_buffer` makes the buffer of the defaults.
7. `mesh_make_texture` makes the white texture of one pixel, and its bind group.
8. It adds the 24 bytes of the defaults and the 4 bytes of the white texture to `held`.

The pipelines and the samplers read the copy, and each round reads `webgpu:MeshSettings` itself. Thus the settings act when `docs/extern/webgpu_mesh.h.md`, *The systems*, tells.
The value of the white texture keeps its 4 bytes, so its release gives them back.
A call of wgpu-native that makes an object always gives one, also on an error. The error goes to the error callback of the device.

## Buffers and textures

`mesh_make_buffer` makes a buffer of the usage that the caller gives. It writes the bytes of the caller into it through the queue, if the caller gives some.
A mesh and the defaults get a vertex buffer, and the round buffer gets the usage of `docs/intern/mesh_memory.c.md`, *The round buffer*.
`mesh_make_texture` makes a texture of `RGBA8Unorm` and its view, and writes the pixels row by row.
It also makes the bind group 1 of the texture: its view, and the sampler of its `filter`.

`mesh_make_group` makes a bind group of the layout of the group 0 or of the group 1, with the entries of the caller.
`mesh_make_texture` makes the group 1 of a texture with it, and `grow_round` of `src/mesh_memory.c` the group 0 of the round buffer.

## The release

`mesh_release_device` releases each pipeline, the layouts, the shader module, the samplers and the defaults. It gives the 24 bytes of the defaults back.
`mesh_release_gpu` releases the white texture, and `mesh_release_round` releases the round buffer.
Then `mesh_release_device` writes zeros over the copy of the frame, so the next ready round makes the objects again.
It releases only the objects that are not 0. Thus a round with no frame and no device releases nothing.

## Functions

| Function | Result |
| --- | --- |
| `mesh_make_device` | Makes the objects that each pipeline of a device shares. |
| `mesh_make_buffer` | Makes a buffer of a usage, and writes some bytes into it. |
| `mesh_make_group` | Makes a bind group of the layout of the group 0 or 1, with its entries. |
| `mesh_make_texture` | Makes a texture with its pixels, its view and its bind group. |
| `mesh_release_device` | Releases the objects of the device, and forgets the device. |
