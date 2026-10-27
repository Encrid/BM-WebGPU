# include/webgpu_mesh.h

`include/webgpu_mesh.h` is the header of the module `webgpu:mesh@1.0.0`, a renderer of Bridge that draws with WebGPU.
The module draws what its components describe: views and meshes, with textures, draws and batches.
It draws into the frame of `webgpu:device`. It does not cover the shaders of an app, or text.

## The module

| Word of the manifest | Value |
| --- | --- |
| The identity | `webgpu:mesh@1.0.0` |
| `abi` | `1` |
| `config` | `webgpu:MeshSettings` |
| `settings` | `load blend clear_depth depth_compare cull clockwise repeat gpu_memory` |
| `pinned` | `false`, because WebGPU binds no call to a thread. |
| `reloadable` | `false` |

The manifest also gives a default for each of these keys of the module. A deployment that states one of them replaces its default.

| Key | Default | Reason |
| --- | --- | --- |
| `scratch` | `8192` | The declarations of the ten types of `init`. |
| `log_ring` | `2048` | Room for 64 records of 32 bytes: the lines of one round. |
| `spans` | `4096` | A cell for each of the ten types in each table, and the spans of `render`, which matches each table. |
| `spawns` | `"none"` | The module makes no entity. |
| `changes` | `"none"` | The module attaches, detaches and destroys nothing. |
| `persistent` | `"none"` | The module keeps no data in the arena. |
| `hot_reload` | `false` | The module cannot reload. |

The header `bridge.h` of Bridge tells the room that `spans` needs in a world with many tables.

The file of the module is `webgpu_mesh`, with the suffix of a library. It runs on the backend of BB-Native, and loads the wgpu-native library of BL-WGPU-Native, which sits beside it.
It needs no SDL3: it reads the device and the frame from `webgpu:Frame`.

## The system render

`render` runs in the phase `render`, and does these steps:

1. If no `webgpu:Frame` is there, or its state is `WEBGPU_FRAME_NO_DEVICE`, it releases each object that it holds.
2. If the state is not `WEBGPU_FRAME_READY`, it stops.
3. For a device that it holds no objects of, it makes its pipelines and its samplers.
4. It uploads each mesh and each texture whose `gen` changed.
5. With `WEBGPU_LOAD_CLEAR`, it clears the frame with `clear` of `webgpu:Scene`, and the depth with `clear_depth`.
6. For each view in `order`, it draws the draws, then the batches, that hold a bit of its `mask`.
7. With more than 1 sample, its pass resolves the multisample texture into the frame.

`render` does not present the frame. The module `webgpu:device` does that.
Each object that the module makes has a label that starts with `webgpu:mesh`. Thus an error of WebGPU tells that the module made the object.
The full name of the system for `[order]` is `webgpu:mesh:render`.

## webgpu:MeshSettings

`webgpu:MeshSettings` is the singleton of the settings. Bridge writes it from the deployment, or from the defaults of the manifest.

| Field | C type | Value |
| --- | --- | --- |
| `load` | `webgpu_load_t` | What the module does with the frame before it draws. The default is `WEBGPU_LOAD_CLEAR`. |
| `blend` | `bool` | `true`: each draw and each batch blends by its alpha, source alpha over one minus source alpha. The alpha of the frame becomes the alpha of the item. The default is `true`. |
| `clear_depth` | `float` | The depth that `WEBGPU_LOAD_CLEAR` clears to. The views share it. The default is 1.0. |
| `depth_compare` | `webgpu_compare_t` | The test of bit 0 of `depth`. The default is `WEBGPU_COMPARE_LESS_EQUAL`. |
| `cull` | `webgpu_cull_t` | The faces that the module does not draw. The default is `WEBGPU_CULL_NONE`. |
| `clockwise` | `bool` | `true`: a front face turns clockwise. The default is `false`, as in WebGPU. |
| `repeat` | `bool` | `true`: the texture coordinates repeat. `false`: they clamp to the edge. The default is `true`. |
| `gpu_memory` | `uint64_t` | The most bytes of the buffers and the textures that the module holds. The default is 18446744073709551615, so only the device bounds them. |

An alpha of 1 gives the result of an opaque draw. Draw the transparent items last, with a higher `order`, and without bit 1 of `depth`.
Past `gpu_memory`, the module leaves out the new item, and writes a line at the level 4: *Messages*.

Each enum of the settings is of `uint32_t`. A deployment states a constant by its name or by its value.

### webgpu:webgpu_load_t

| Constant | Value | What the module does before it draws |
| --- | --- | --- |
| `WEBGPU_LOAD_CLEAR` | 1 | It clears the frame and the depth. |
| `WEBGPU_LOAD_KEEP` | 2 | It draws over what the frame holds. |

In a deployment with more render modules, the first one in the order of the phases clears, and each other one keeps.

### webgpu:webgpu_compare_t

The values are those of `webgpu.h`.

| Constant | Value |
| --- | --- |
| `WEBGPU_COMPARE_NEVER` | 1 |
| `WEBGPU_COMPARE_LESS` | 2 |
| `WEBGPU_COMPARE_EQUAL` | 3 |
| `WEBGPU_COMPARE_LESS_EQUAL` | 4 |
| `WEBGPU_COMPARE_GREATER` | 5 |
| `WEBGPU_COMPARE_NOT_EQUAL` | 6 |
| `WEBGPU_COMPARE_GREATER_EQUAL` | 7 |
| `WEBGPU_COMPARE_ALWAYS` | 8 |

### webgpu:webgpu_cull_t

The values are those of `webgpu.h`: `WEBGPU_CULL_NONE` 1, `WEBGPU_CULL_FRONT` 2 and `WEBGPU_CULL_BACK` 3.

## webgpu:Scene

`webgpu:Scene` is a singleton that the app writes at each round.

| Field | C type | Value |
| --- | --- | --- |
| `clear` | `float[4]` | The color that `WEBGPU_LOAD_CLEAR` clears the frame to: red, green, blue and alpha. |
| `light` | `float[3]` | The direction from a face toward the light, for the shading `lambert`. |
| `light_color` | `float[3]` | The color of the light. |
| `ambient` | `float[3]` | The color that each face gets with no light. |

## webgpu:View

`webgpu:View` is a table, with one entity for each view.

| Field | C type | Value |
| --- | --- | --- |
| `matrix` | `float[16]` | The matrix of the view and of the projection, column by column. It maps the world to the clip space of WebGPU, where the depth goes from 0 to 1. |
| `rect` | `uint32_t[4]` | The rectangle of the view, in pixels from the top left: x, y, width and height. A width of 0 is the whole frame. |
| `mask` | `uint32_t` | The bits of the view. |
| `order` | `int32_t` | The views draw from the lowest `order`. |

A view can reach past the right or the bottom edge of the frame. The module draws its part inside the frame, with no change to its projection.

## webgpu:Mesh

`webgpu:Mesh` is a table, with one entity for each mesh.

| Field | C type | Value |
| --- | --- | --- |
| `data` | `bridge_text_t` | The vertices, in the persistent memory of the app. |
| `layout` | `uint32_t` | The parts of a vertex, as bits. *The vertices* tells them. |
| `primitive` | `uint32_t` | 0 triangles, 1 lines, 2 points. |
| `gen` | `uint32_t` | A change uploads the data again. |

## webgpu:Texture

`webgpu:Texture` is a table, with one entity for each texture.

| Field | C type | Value |
| --- | --- | --- |
| `data` | `bridge_text_t` | The pixels in RGBA8, row by row from the top. (0, 0) is the top left. |
| `width`, `height` | `uint32_t` | The size of the texture, in pixels. |
| `gen` | `uint32_t` | A change uploads the pixels again. |
| `filter` | `uint32_t` | 0 nearest, 1 linear. |

## webgpu:Draw

`webgpu:Draw` is a table, with one entity for each mesh that draws.

| Field | C type | Value |
| --- | --- | --- |
| `mesh` | `uint64_t` | The entity of the mesh. |
| `texture` | `uint64_t` | The entity of the texture, or 0. |
| `model` | `float[16]` | The matrix of the model, column by column. |
| `color` | `float[4]` | The color that multiplies each vertex. |
| `shading` | `uint32_t` | 0 unlit, 1 lambert, 2 textured. *The shading* tells them. |
| `views` | `uint32_t` | The bits of the views that draw it. |
| `order` | `int32_t` | In a view, the draws draw from the lowest `order`. Draws of one `order` draw in the order of their entities. |
| `depth` | `uint32_t` | Bit 0 tests the depth. Bit 1 writes the depth. |
| `bias` | `float` | A positive `bias` moves each vertex toward the view: its depth after the projection, from 0 to 1, gets smaller by `bias`. It keeps lines in front of faces. |
| `size` | `float` | The width of each point and of each line of the mesh, in pixels. *The widths* tells it. |

## webgpu:Batch

`webgpu:Batch` is a table, with one entity for each batch of vertices that the app writes again at each round.

| Field | C type | Value |
| --- | --- | --- |
| `data` | `bridge_text_t` | The vertices of the round. |
| `layout`, `primitive` | `uint32_t` | As in `webgpu:Mesh`. |
| `views`, `depth` | `uint32_t` | As in `webgpu:Draw`. |
| `order` | `int32_t` | As in `webgpu:Draw`. |
| `size` | `float` | As in `webgpu:Draw`. |

A batch draws unlit, with no texture and no model. Its color is the color of its vertices, or white.

## The vertices

A vertex holds a position of 3 `float`, then the parts that `layout` names, in this order:

| Bit | Part | Bytes |
| --- | --- | --- |
| 0 | A normal of 3 `float`. | 12 |
| 1 | A texture coordinate of 2 `float`. | 8 |
| 2 | A color in RGBA8. | 4 |

The stride is 12 bytes, plus the bytes of each part.

## The shading

- `unlit`: the color is `color` times the color of the vertex.
- `lambert`: the color of `unlit`, times `ambient` plus `light_color` times the larger of 0 and the dot product of the normal and `light`. The normal turns with `model`. The alpha stays.
- `textured`: the color of `unlit`, times the texel of `texture`.

A vertex with no color is white, and a vertex with no normal or no texture coordinate gets 0 for it.
A draw gets a white texture in each round in which its `texture` is 0, or names a texture that is left out or gone.

## The widths

A `size` of 1 or less draws each point and each line 1 pixel wide, as WebGPU draws them.
Above 1, the module draws each point as a square, and each line as a band, of `size` pixels. A wide line has no join at a sharp corner.

## The data rules

- The app allocates the data in its own persistent memory, and a field holds its place.
- The module leaves out an item whose data is outside the memory of the world, or whose length is not a multiple of the stride.
- It leaves out a texture whose length is not `width` times `height` times 4.
- It leaves out a draw that names no mesh, and an item whose data has a length of 0.
- It leaves out an item with a `primitive` above 2, a `layout` above 7, a `shading` above 2 or a `filter` above 1.
- It leaves out a texture that is larger than the limits of the device, and data that is larger than a buffer of the device.

## The C types

`include/webgpu_mesh.h` gives the C type of each type of the module, and its descriptor for Bridge.

| Type of Bridge | C type | Descriptor |
| --- | --- | --- |
| `webgpu:webgpu_load_t` | `webgpu_load_t` | `webgpu_load_t_desc` |
| `webgpu:webgpu_compare_t` | `webgpu_compare_t` | `webgpu_compare_t_desc` |
| `webgpu:webgpu_cull_t` | `webgpu_cull_t` | `webgpu_cull_t_desc` |
| `webgpu:MeshSettings` | `webgpu_mesh_settings_t` | `webgpu_mesh_settings_t_desc` |
| `webgpu:Scene` | `webgpu_scene_t` | `webgpu_scene_t_desc` |
| `webgpu:View` | `webgpu_view_t` | `webgpu_view_t_desc` |
| `webgpu:Mesh` | `webgpu_mesh_t` | `webgpu_mesh_t_desc` |
| `webgpu:Texture` | `webgpu_texture_t` | `webgpu_texture_t_desc` |
| `webgpu:Draw` | `webgpu_draw_t` | `webgpu_draw_t_desc` |
| `webgpu:Batch` | `webgpu_batch_t` | `webgpu_batch_t_desc` |

## The deployment

- `phases` lists `render` after the phase `frame` of `webgpu:device`, and before `present`.
- The app writes `webgpu:Scene` and the five tables, so it needs `write` with the six names.
- The module reads `webgpu:Frame`, so it needs `read` with `webgpu:Frame`.
- The app needs a `persistent` share for the data.
- A deployment gives the settings in `[module."webgpu:mesh@1.0.0".config]`.
- The backend of BB-Native has no capability of a limit, so `work_limit`, `memory_limit` and the three timeouts of the module are `"none"`.

The readme of Bridge-Host tells each key of a deployment.

## Messages

The module writes this line at the level 4, and leaves out the item:

- `no room: <bytes> bytes of gpu_memory are taken, and the item needs <bytes> more`: the item does not fit in `gpu_memory`.

The errors of WebGPU are lines of the module `webgpu:device`, with the label of the object that the error names.
If the memory of the caches or of a round cannot be allocated, `render` gives 1 with no line, and tries again.
