# src/mesh_inside.h

This private header is shared by the sources of the module.
It includes `include/webgpu_mesh.h`, the header `webgpu_device.h` of the module `webgpu:device`, `webgpu.h` of wgpu-native and the headers of BB-Native.

## The slots

Bridge numbers the types of a module from 0, in the order of their declarations.
Each slot is a `constexpr`, and `bridge_init` of `src/webgpu_mesh.c` declares the types in this order:

| Slot | Name | Type |
| --- | --- | --- |
| 0 | `LOAD` | `webgpu:webgpu_load_t` |
| 1 | `COMPARE` | `webgpu:webgpu_compare_t` |
| 2 | `CULL` | `webgpu:webgpu_cull_t` |
| 3 | `SETTINGS` | `webgpu:MeshSettings` |
| 4 | `FRAME_STATE` | `webgpu:webgpu_frame_state_t` |
| 5 | `FRAME` | `webgpu:Frame` |
| 6 | `SCENE` | `webgpu:Scene` |
| 7 | `VIEW` | `webgpu:View` |
| 8 | `MESH` | `webgpu:Mesh` |
| 9 | `TEXTURE` | `webgpu:Texture` |
| 10 | `DRAW` | `webgpu:Draw` |
| 11 | `BATCH` | `webgpu:Batch` |
| 12 | `MESH_GPU` | `webgpu:MeshGpu` |
| 13 | `TEXTURE_GPU` | `webgpu:TextureGpu` |

An enum comes before the struct whose field names it.
The system `render` has the slot 0. The observers `release_mesh` and `release_texture` have the slots 0 and 1.

## Types

| Type | Holds |
| --- | --- |
| `webgpu_mesh_gpu_t` | The C type of `webgpu:MeshGpu`: `buffer`, then the `gen`, the `count` of vertices, the `layout` and the `primitive` of its upload. 24 bytes. |
| `webgpu_texture_gpu_t` | The C type of `webgpu:TextureGpu`: `texture`, its `view`, its bind `group`, then the `gen` and the `bytes` of its upload. 32 bytes. |
| `mesh_state_t` | The state of the module: the objects of a device, the pipelines, the round buffer, the counts of the two budgets, and the round. |
| `mesh_round_t` | The memory of a round, its views, its items, its blocks and its records, with their counts. |
| `mesh_key_t` | The sort key of a view or of an item: `rank`, `order` and `id`. |
| `mesh_view_t` | A view of the round: its key, `mask`, its viewport, its scissor, and the index of its block. |
| `mesh_item_t` | A draw or a batch of the round: its key, `views`, its shape, `layout`, `depth`, its vertices, its record and its bind group. |
| `mesh_record_t` | The data of one item for the shaders: `model`, `color`, `bias`, `size` and `shading`. 92 bytes. |
| `mesh_block_t` | The data of one view for the shaders: `matrix`, the light of `webgpu:Scene`, and the size of the view in pixels. 128 bytes. |
| `mesh_pass_t` | The pass of a round, with what it binds now: each vertex buffer and its offset, the pipeline, and the bind group 1. |
| `mesh_shape_t` | An enum of `uint32_t`: `SHAPE_TRIANGLES`, `SHAPE_LINES`, `SHAPE_POINTS`, `SHAPE_WIDE_LINES` and `SHAPE_WIDE_POINTS`. |

The first field of a GPU component holds its main object, `buffer` or `texture`. A 0 there means that the component holds no object.
The descriptor of each GPU component comes from the same text as its C type, with the macros of `bridge_schema.h` of Bridge.
The two GPU components have no gap, so they need no field `pad`.

## Constants

| Constant | Value |
| --- | --- |
| `STRIDES` | The bytes of one vertex of each `layout`, from 0 to 7: 12, 24, 20, 32, 16, 28, 24 and 36. |
| `CHANGES` | 65536, the default of `changes` in the manifest: `docs/intern/mesh_store.c.md`, *The room of changes*. |
| `CHANGE` | 24, the bytes of `changes` that one attach, detach or destroy takes before its value. |
| `BLOCK` | 256, the step from one block to the next in the round buffer. |
| `PIPELINES` | 160: 8 values of `layout`, by 5 shapes, by 4 values of the bits 0 and 1 of `depth`. |

## Functions

| Function | Source |
| --- | --- |
| `mesh_release_all` | `src/mesh_round.c` |
| `mesh_get_state` | `src/mesh_store.c` |
| `mesh_walk` | `src/mesh_store.c` |
| `mesh_find` | `src/mesh_store.c` |
| `mesh_get_data` | `src/mesh_store.c` |
| `mesh_take_change` | `src/mesh_store.c` |
| `mesh_keep_mesh` | `src/mesh_upload.c` |
| `mesh_keep_texture` | `src/mesh_upload.c` |
| `mesh_take_memory` | `src/mesh_upload.c` |
| `mesh_sweep` | `src/mesh_release.c` |
| `mesh_release_gpu` | `src/mesh_release.c` |
| `mesh_add_view` | `src/mesh_draw.c` |
| `mesh_add_draw` | `src/mesh_draw.c` |
| `mesh_add_batch` | `src/mesh_draw.c` |
| `mesh_encode` | `src/mesh_pass.c` |
| `mesh_make_layouts` | `src/mesh_pipeline.c` |
| `mesh_get_pipeline` | `src/mesh_pipeline.c` |
| `mesh_make_device` | `src/mesh_device.c` |
| `mesh_make_buffer` | `src/mesh_device.c` |
| `mesh_make_texture` | `src/mesh_device.c` |
| `mesh_release_device` | `src/mesh_device.c` |
| `mesh_reserve_round` | `src/mesh_memory.c` |
| `mesh_put_round` | `src/mesh_memory.c` |
| `mesh_release_round` | `src/mesh_memory.c` |
| `mesh_free_round` | `src/mesh_memory.c` |
