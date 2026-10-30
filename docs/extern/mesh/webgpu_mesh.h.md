# include/mesh/webgpu_mesh.h

`include/mesh/webgpu_mesh.h` is the header of the module `webgpu:mesh@1.0.0`, a renderer of Bridge that draws with WebGPU.
The module draws what its components describe: views and meshes, with textures, draws and batches.
It draws into the frame that the device module writes, for example `webgpu:device`. It does not cover the shaders of an app, or text.

## The module

| Word of the manifest | Value |
| --- | --- |
| The identity | `webgpu:mesh@1.0.0` |
| `abi` | `2` |
| `config` | `webgpu:MeshSettings` |
| `settings` | `load blend clear_depth depth_compare cull clockwise repeat gpu_memory` |
| `pinned` | `false`, because WebGPU encodes and submits passes on any thread, and the module does not touch the surface. |
| `reloadable` | `false`, because its pipelines and samplers live in its own memory, and a reload would lose them. |

The manifest gives the defaults of this table. `readme.md`, *The modules*, tells how a deployment replaces them.

| Key | Default | Reason |
| --- | --- | --- |
| `scratch` | `8192` | The declarations of the fourteen types of `init`: its twelve, and `webgpu:webgpu_frame_state_t` and `webgpu:Frame` of the part `contract`. At a run of a system, the texts of its lines. |
| `log_ring` | `64` | Room for 2 records of 32 bytes: a round writes each line of *Messages* at most once. |
| `spans` | `65536` | A cell for each of the fourteen types in each table, the spans of its five systems, and the lists of its two observers. A list holds 16 bytes for each mesh and each texture that leaves at one barrier. |
| `spawns` | `"none"` | The module makes no entity. |
| `changes` | `65536` | The attaches and the detaches of `webgpu:MeshGpu` and `webgpu:TextureGpu` in one round: *The GPU objects*. |
| `persistent` | `"none"` | The arena holds no data of the module. |
| `hot_reload` | `false` | The module cannot reload. |

The file of the module is `webgpu_mesh`, with the suffix of a library. It runs as `readme.md`, *The modules*, tells.
It needs no SDL3: it reads the device and the frame from `webgpu:Frame`.

## The systems

The module has five systems in the phase `render`. They run one after another, in the order of this table:

| Full name for `[order]` | Term of `BRIDGE_AND` | What it does |
| --- | --- | --- |
| `webgpu:mesh:keep_meshes` | `webgpu:Mesh` | It starts the round from `webgpu:Frame`, and uploads the meshes. |
| `webgpu:mesh:keep_textures` | `webgpu:Texture` | It uploads the textures. |
| `webgpu:mesh:draws` | `webgpu:Draw` | It adds each draw to the round. |
| `webgpu:mesh:batches` | `webgpu:Batch` | It adds each batch to the round. |
| `webgpu:mesh:render` | `webgpu:View` | It adds each view to the round, and encodes the pass of the round. |

Each system reads only the tables that hold its kind of item. In each round, the five systems do these steps:

1. If no `webgpu:Frame` is there, or its state is `WEBGPU_FRAME_NO_DEVICE`, they release each object.
   Each component of *The GPU objects* stays, with no object.
2. If the state is not `WEBGPU_FRAME_READY`, they draw nothing.
3. For a new device, they make its samplers, the buffer of its defaults and a white texture.
   They make each pipeline at the first draw that needs it.
4. They upload each mesh and each texture whose `gen` changed, and keep its GPU objects: *The GPU objects*.
5. With `WEBGPU_LOAD_CLEAR`, `render` clears the frame with `clear` of `webgpu:Scene`, and the depth with `clear_depth`.
6. For each view in `order`, `render` draws the draws, then the batches, that hold a bit of its `mask`.
7. With more than 1 sample, the pass resolves the multisample texture into the frame.

A round with no view still runs `render`, which then does only step 5.
`render` does not present the frame. The device module does that.
A new device, or a new format, samples or depth format of `webgpu:Frame`, makes the module release each object and draw nothing in that round.
From the next round, each item uploads again into its component.
With no `webgpu:Scene`, `render` takes each of its fields as 0.

`blend`, `depth_compare`, `cull`, `clockwise` and `repeat` act from the next device. `load`, `clear_depth` and `gpu_memory` act at the next run of the phase `render`.
Each object that the module makes has a label that starts with `webgpu:mesh`. Thus an error of WebGPU tells that the module made the object.

## The GPU objects

The module keeps the GPU objects of an entity in a component of its own on that entity:

| Component | On the entity of | Holds |
| --- | --- | --- |
| `webgpu:MeshGpu` | `webgpu:Mesh` | The buffer of the vertices, the `gen` that it shows, and the count, `layout` and `primitive` of its vertices. |
| `webgpu:TextureGpu` | `webgpu:Texture` | The texture, its view, its bind group, the `gen` that it shows, and its bytes. |

Each one is a sparse set, with `on_destroy` set to `BRIDGE_ON_DESTROY_KEEP`. Thus an entity that the app destroys stays as a husk, until the module releases its objects.
The observers `release_mesh` and `release_texture` hear `BRIDGE_ON_LEAVE` of `webgpu:Mesh` and of `webgpu:Texture`.
Each one releases the GPU objects of the entity, and detaches the component of the module. Then Bridge destroys a husk.

A detach that finds no room waits for a later round. If the item uploads again before it, the component takes the new objects.

The first upload of an item takes two rounds. In the first round, `keep_meshes` or `keep_textures` attaches the component with no object. From the next round, it uploads into the component.
Thus an attach that Bridge does not apply loses no GPU object. A draw whose mesh or texture waits draws from that next round.
An attach that finds no room in `changes` waits for the next round, so its item draws one round later.

The two components are the module's own, so their C types are in its private header, not in `include/mesh/webgpu_mesh.h`.
The full names of the observers for `[order]` are `webgpu:mesh:release_mesh` and `webgpu:mesh:release_texture`.

## webgpu:MeshSettings

`webgpu:MeshSettings` is the singleton of the settings. `readme.md`, *The modules*, tells how Bridge fills it.

| Field | C type | Offset | Value |
| --- | --- | --- | --- |
| `load` | `webgpu_load_t` | 0 | What the module does with the frame before it draws. The default is `WEBGPU_LOAD_CLEAR`. |
| `blend` | `bool` | 4 | `true`: each draw and each batch blends by its alpha, source alpha over one minus source alpha. The alpha of the frame becomes the alpha of the item. The default is `true`. |
| `clear_depth` | `float` | 8 | The depth that `WEBGPU_LOAD_CLEAR` clears to, from 0 to 1. A value below 0 is taken as 0, and a value above 1 as 1. The views share it. The default is 1.0. |
| `depth_compare` | `webgpu_compare_t` | 12 | The test of bit 0 of `depth`. The default is `WEBGPU_COMPARE_LESS_EQUAL`. |
| `cull` | `webgpu_cull_t` | 16 | The faces that the module does not draw. The default is `WEBGPU_CULL_NONE`. |
| `clockwise` | `bool` | 20 | `true`: a front face turns clockwise. The default is `false`, as in WebGPU. |
| `repeat` | `bool` | 21 | `true`: the texture coordinates repeat. `false`: they clamp to the edge. The default is `true`. |
| `gpu_memory` | `uint64_t` | 24 | The most bytes of the buffers and the textures that the module holds. The default is 9223372036854775807, the largest value of a setting, so only the device bounds them. |

The bytes 5 to 7 and 22 to 23 hold no field.

An alpha of 1 gives the result of an opaque draw. Draw the transparent items last, with a higher `order`, and without bit 1 of `depth`.

Past `gpu_memory`, the module leaves out the new item, and writes a line at the level 4: *Messages*.
The buffer of the defaults and the white texture of each device take 28 bytes of `gpu_memory`.
The buffer of a round counts too: 256 bytes for each view, 92 for each draw and each batch, and the vertices of the batches.
Its size is the smallest power of 2 that holds these bytes. If it cannot grow, that round draws no view.

With no depth texture in `webgpu:Frame`, `depth`, `depth_compare` and `clear_depth` do nothing. An example is `WEBGPU_DEPTH_NONE` of `webgpu:device`.

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

| Field | C type | Offset | Value |
| --- | --- | --- | --- |
| `clear` | `float[4]` | 0 | The color that `WEBGPU_LOAD_CLEAR` clears the frame to: red, green, blue and alpha. |
| `light` | `float[3]` | 16 | The direction from a face toward the light, for the shading `lambert`. |
| `light_color` | `float[3]` | 28 | The color of the light. |
| `ambient` | `float[3]` | 40 | The color that each face gets with no light. |

## webgpu:View

`webgpu:View` is a table, with one entity for each view.

| Field | C type | Offset | Value |
| --- | --- | --- | --- |
| `matrix` | `float[16]` | 0 | The matrix of the view and of the projection, column by column. It maps the world to the clip space of WebGPU, where the depth goes from 0 to 1. |
| `rect` | `uint32_t[4]` | 64 | The rectangle of the view, in pixels from the top left: x, y, width and height. A width of 0 is the whole frame. |
| `mask` | `uint32_t` | 80 | The bits of the view. |
| `order` | `int32_t` | 84 | The views draw from the lowest `order`. |

A view can reach past the right or the bottom edge of the frame. The module draws its part inside the frame, with no change to its projection.
The module leaves out a view with no part inside the frame, and a view wider or taller than `maxTextureDimension2D` of the device.

## webgpu:Mesh

`webgpu:Mesh` is a table, with one entity for each mesh.

| Field | C type | Offset | Value |
| --- | --- | --- | --- |
| `data` | `bridge_text_t` | 0 | The vertices, in the persistent memory of the app. |
| `layout` | `uint32_t` | 8 | The parts of a vertex, as bits. *The vertices* tells them. |
| `primitive` | `uint32_t` | 12 | 0 triangles, 1 lines, 2 points. A line joins each two vertices. A list drops the vertices after its last whole triangle or line. |
| `gen` | `uint32_t` | 16 | A change uploads the data again. A change of `data`, `layout` or `primitive` acts only with a new `gen`. |

## webgpu:Texture

`webgpu:Texture` is a table, with one entity for each texture.

| Field | C type | Offset | Value |
| --- | --- | --- | --- |
| `data` | `bridge_text_t` | 0 | The pixels in RGBA8, row by row from the top. (0, 0) is the top left. |
| `width`, `height` | `uint32_t` | 8, 12 | The size of the texture, in pixels. |
| `gen` | `uint32_t` | 16 | A change uploads the pixels again. A change of `data`, `width`, `height` or `filter` acts only with a new `gen`. |
| `filter` | `uint32_t` | 20 | 0 nearest, 1 linear. |

## webgpu:Draw

`webgpu:Draw` is a table, with one entity for each mesh that draws.

| Field | C type | Offset | Value |
| --- | --- | --- | --- |
| `mesh` | `uint64_t` | 0 | The entity of the mesh. |
| `texture` | `uint64_t` | 8 | The entity of the texture, or 0. |
| `model` | `float[16]` | 16 | The matrix of the model, column by column. |
| `color` | `float[4]` | 80 | The color that multiplies each vertex. |
| `shading` | `uint32_t` | 96 | 0 unlit, 1 lambert, 2 textured. *The shading* tells them. |
| `views` | `uint32_t` | 100 | The bits of the views that draw it. |
| `order` | `int32_t` | 104 | In a view, the draws draw from the lowest `order`. Draws of one `order` draw in the order of their entity ids, from the lowest. |
| `depth` | `uint32_t` | 108 | Bit 0 tests the depth. Bit 1 writes the depth. The other bits do nothing. |
| `bias` | `float` | 112 | A positive `bias` moves each vertex toward the view: its depth after the projection, from 0 to 1, gets smaller by `bias`. It keeps lines in front of faces. The GPU does not draw a part with a depth below 0. |
| `size` | `float` | 116 | The width of each point and of each line of the mesh, in pixels. *The widths* tells it. |

## webgpu:Batch

`webgpu:Batch` is a table, with one entity for each batch of vertices that the app writes again at each round.

| Field | C type | Offset | Value |
| --- | --- | --- | --- |
| `data` | `bridge_text_t` | 0 | The vertices of the round. |
| `layout`, `primitive` | `uint32_t` | 8, 12 | As in `webgpu:Mesh`. |
| `views`, `depth` | `uint32_t` | 16, 20 | As in `webgpu:Draw`. |
| `order` | `int32_t` | 24 | As in `webgpu:Draw`. |
| `size` | `float` | 28 | As in `webgpu:Draw`. |

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
The module lights each vertex, and the GPU interpolates the color between the vertices.

The module gives each color as it is, and takes each texel as it is.
With a frame of an sRGB format, the GPU blends the colors as they are, and then encodes the result to sRGB. Thus a color of 0.5 shows as 188 of 255.

## The widths

A `size` of 1 or less draws each point and each line 1 pixel wide, as WebGPU draws them.
Above 1, the module draws each point as a square, and each line as a band, of `size` pixels. A wide line has no join at a sharp corner.
On the GL backend of wgpu-native, the module needs OpenGL 4.3 or OpenGL ES 3.1, the level of the compatibility of WebGPU. Below them, a buffer of stride 0 reads wrong data.

## The data rules

- The app allocates the data in its own persistent memory, and a field holds its place.
- The module leaves out an item whose data is outside the persistent arena, or whose length is not a multiple of the stride.
- It leaves out a texture whose length is not `width` times `height` times 4.
- It leaves out a draw that names no mesh, and an item whose data has a length of 0.
- It leaves out an item with a `primitive` above 2, a `layout` above 7, a `shading` above 2 or a `filter` above 1.
- It leaves out a texture that is larger than the limits of the device, and data that is larger than a buffer of the device.
- It tries a left-out item again at each round. Thus a fix of its fields acts with no new `gen`.

Each rule that leaves out an item, a view or a round writes the line `left out` of *Messages*.

## The C types

Each type of the module has its C type and its descriptor in `include/mesh/webgpu_mesh.h`, as `docs/extern/contract/webgpu_contract.h.md`, *The C types*, tells.
The header includes only headers of Bridge.

| Type of Bridge | C type | Descriptor | Macro of the text | Size | Alignment |
| --- | --- | --- | --- | --- | --- |
| `webgpu:webgpu_load_t` | `webgpu_load_t` | `webgpu_load_t_desc` | `WEBGPU_LOAD_VALUES` | 4 | 4 |
| `webgpu:webgpu_compare_t` | `webgpu_compare_t` | `webgpu_compare_t_desc` | `WEBGPU_COMPARE_VALUES` | 4 | 4 |
| `webgpu:webgpu_cull_t` | `webgpu_cull_t` | `webgpu_cull_t_desc` | `WEBGPU_CULL_VALUES` | 4 | 4 |
| `webgpu:MeshSettings` | `webgpu_mesh_settings_t` | `webgpu_mesh_settings_t_desc` | `WEBGPU_MESH_SETTINGS_FIELDS` | 32 | 8 |
| `webgpu:Scene` | `webgpu_scene_t` | `webgpu_scene_t_desc` | `WEBGPU_SCENE_FIELDS` | 52 | 4 |
| `webgpu:View` | `webgpu_view_t` | `webgpu_view_t_desc` | `WEBGPU_VIEW_FIELDS` | 88 | 4 |
| `webgpu:Mesh` | `webgpu_mesh_t` | `webgpu_mesh_t_desc` | `WEBGPU_MESH_FIELDS` | 20 | 4 |
| `webgpu:Texture` | `webgpu_texture_t` | `webgpu_texture_t_desc` | `WEBGPU_TEXTURE_FIELDS` | 24 | 4 |
| `webgpu:Draw` | `webgpu_draw_t` | `webgpu_draw_t_desc` | `WEBGPU_DRAW_FIELDS` | 120 | 8 |
| `webgpu:Batch` | `webgpu_batch_t` | `webgpu_batch_t_desc` | `WEBGPU_BATCH_FIELDS` | 32 | 4 |

In the order of the table, the descriptors hold these bytes:

```text
uint32_t { WEBGPU_LOAD_CLEAR = 1, WEBGPU_LOAD_KEEP = 2 }
uint32_t { WEBGPU_COMPARE_NEVER = 1, WEBGPU_COMPARE_LESS = 2, WEBGPU_COMPARE_EQUAL = 3, WEBGPU_COMPARE_LESS_EQUAL = 4, WEBGPU_COMPARE_GREATER = 5, WEBGPU_COMPARE_NOT_EQUAL = 6, WEBGPU_COMPARE_GREATER_EQUAL = 7, WEBGPU_COMPARE_ALWAYS = 8 }
uint32_t { WEBGPU_CULL_NONE = 1, WEBGPU_CULL_FRONT = 2, WEBGPU_CULL_BACK = 3 }
{ webgpu_load_t load; bool blend; float clear_depth; webgpu_compare_t depth_compare; webgpu_cull_t cull; bool clockwise; bool repeat; uint64_t gpu_memory; }
{ float clear[4]; float light[3]; float light_color[3]; float ambient[3]; }
{ float matrix[16]; uint32_t rect[4]; uint32_t mask; int32_t order; }
{ bridge_text_t data; uint32_t layout; uint32_t primitive; uint32_t gen; }
{ bridge_text_t data; uint32_t width; uint32_t height; uint32_t gen; uint32_t filter; }
{ uint64_t mesh; uint64_t texture; float model[16]; float color[4]; uint32_t shading; uint32_t views; int32_t order; uint32_t depth; float bias; float size; }
{ bridge_text_t data; uint32_t layout; uint32_t primitive; uint32_t views; uint32_t depth; int32_t order; float size; }
```

`webgpu:webgpu_frame_state_t` and `webgpu:Frame` are the types of the part `contract`. Its header `include/contract/webgpu_contract.h` gives their C types and descriptors.
`docs/extern/contract/webgpu_contract.h.md` tells their fields and their constants.

## The deployment

- `phases` lists `render` after the phase in which the device module writes the frame, and before the phase in which it presents the frame.
  For `webgpu:device`, these phases are `frame` and `present`.
- The app writes `webgpu:Scene` and the five tables, so it needs `write` with the six names.
- A module of another namespace that changes a setting while the world runs needs `write` with `webgpu:MeshSettings`.
  It also needs `read` with `webgpu:webgpu_load_t`, `webgpu:webgpu_compare_t` and `webgpu:webgpu_cull_t`.
- The module reads `webgpu:Frame` and `webgpu:webgpu_frame_state_t` of its own namespace, so it needs no grant for them.
- The app needs a `persistent` share for the data.
- The world needs `[budget] terms` of 4 or more, for `keep_meshes` and `render`, and `[budget] sparse_sets` of 2 or more.
- The world needs `[budget] types` for the fourteen types of the module, and `[budget] systems` for its five systems and its two observers.
- Bridge stops the module when the list of an observer does not fit in `spans`. Thus a deployment sizes `spans` for the most items that one barrier removes.
- A deployment gives the settings in `[module."webgpu:mesh@1.0.0".config]`.
- The wgpu-native library sits beside the module, and `content.ignore` names it.
- `readme.md`, *wgpu_native*, tells the need of the library on aarch64-windows-gnu.
- The module has no limit and no timeout: `readme.md`, *The modules*.

## Messages

Each of these lines has the level 4, and a round writes it at most once.

- `no room: <bytes> bytes of gpu_memory are taken, and the item needs <bytes> more`: the item does not fit in `gpu_memory`, and the module leaves it out.
- `left out <entity>: <rule>`: a rule leaves out the item or the view of the entity. For a round that the module leaves out, the entity is 0.

The line `left out` names the first item that a rule leaves out in the round. The module looks at the meshes, the textures, the draws, the batches, the views, and then the round.
`<rule>` is one of these texts:

| Rule | What it leaves out |
| --- | --- |
| `its data lies outside the persistent arena` | A mesh, a texture or a batch. |
| `its data has a length of 0` | A mesh, a texture or a batch. |
| `its length is not a multiple of its stride` | A mesh or a batch. |
| `its length is not width times height times 4` | A texture. |
| `its primitive is above 2` | A mesh or a batch. |
| `its layout is above 7` | A mesh or a batch. |
| `its shading is above 2` | A draw. |
| `its filter is above 1` | A texture. |
| `it names no mesh` | A draw whose `mesh` is not an entity with `webgpu:Mesh`. |
| `its data is larger than a buffer of the device` | A mesh. |
| `it is wider or taller than maxTextureDimension2D` | A texture or a view. |
| `it has no part inside the frame` | A view. |
| `the round is larger than a buffer of the device` | The round, which then draws no view. |

Each error of a call of the module reaches the error callback of the device, with the label of its object.
If the commands of a round hold an error, wgpu-native v29.0.1.1 stops the process in `wgpuQueueSubmit`: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L3090.
Before it stops, it writes `Error in wgpuQueueSubmit:` and the text of the error to the standard error.
The device module writes the line of an error at its next frame, so that line does not come.

If the memory of a round cannot be allocated, the system that needs it gives 1 with no line. That round draws nothing, and the next round tries again.
