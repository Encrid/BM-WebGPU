# src/mesh_pass.c

This file encodes the render pass of a round, and draws each item of each view.

## The pass

`mesh_encode` does these steps:

1. It makes a command encoder on the device of `webgpu:Frame`.
2. `begin_pass` begins the render pass on the frame.
3. `draw_view` draws each view of the round, in the sorted order.
4. It ends the pass, and makes the command buffer.
5. It submits the command buffer to `queue` of `webgpu:Frame`.
6. It releases the pass, the command buffer and the encoder.

`begin_pass` gives the pass these attachments:

| Attachment | Value |
| --- | --- |
| The color | `target`, which resolves into `frame` with more than 1 sample. `WEBGPU_LOAD_CLEAR` clears it to `clear` of the scene. Each other `load` loads it. |
| The depth | `depth`, when it is not 0. `WEBGPU_LOAD_CLEAR` clears it to `clear_depth`. Each other `load` loads it. |
| The stencil | Only for a depth format with a stencil. `WEBGPU_LOAD_CLEAR` clears it to 0. Each other `load` loads it. |

Each attachment stores what the pass drew, so a later render module with `WEBGPU_LOAD_KEEP` loads it.
A round with no view only clears, or draws nothing.
`begin_pass` clamps `clear_depth` as `docs/extern/webgpu_mesh.h.md`, *webgpu:MeshSettings*, tells. wgpu-core refuses a pass with a clear depth outside 0 to 1.
wgpu-core needs a load and a store of the stencil for a format with a stencil, and reads none for another format.
The source is https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-core/src/command/render.rs, lines 1696 to 1716.

## A view

`draw_view` sets the viewport and the scissor of the view, and binds the group 0 at the block of the view.
The dynamic offset of the group is the index of the block times `BLOCK`.
Then `draw_view` gives each item that shares a bit with `mask` of the view to `draw_item`.

## A draw

`draw_item` binds the pipeline of the item, its group 1 and its vertex buffers, then it draws:

| Shape | Vertex buffers | The draw |
| --- | --- | --- |
| Triangles, lines and points | The vertices at 0, the records of the round at 1, and the defaults at 2. | `count` vertices, one instance, and the record of the item as `firstInstance`. |
| Wide points | The vertices at 0, the record of the item at 1, and the defaults at 2. | 6 vertices, and `count` instances. |
| Wide lines | The vertices at 0, the record of the item at 1, the vertices from the second vertex at 2, and the defaults at 3. | 6 vertices, and `count` divided by 2 instances. |

The defaults are bound only when the `layout` of the item lacks a part. Each binding takes the rest of its buffer, from its offset.
`docs/intern/mesh_pipeline.c.md`, *The vertex buffers*, tells what each pipeline reads from them.
A direct draw takes a `firstInstance` above 0 with no feature of the device: lines 2634 to 2666 of the same file of wgpu-core.

`bind_buffer` keeps the buffer and the offset at each index, and makes no call that binds them again. `draw_item` does the same for the pipeline and the group 1.
Thus a run of thin draws binds the records of the round once, and each draw then binds only its own vertices.

## Functions

| Function | Result |
| --- | --- |
| `mesh_encode` | Encodes the pass of the round, submits it, and releases its objects. |
| `begin_pass` | Begins the render pass with the attachments of the frame and the settings. |
| `draw_view` | Sets the viewport, the scissor and the block of a view, and draws its items. |
| `draw_item` | Binds the pipeline, the group 1 and the vertex buffers of an item, and draws it. |
| `bind_buffer` | Binds a vertex buffer at an index, unless that buffer and offset are bound there already. |
