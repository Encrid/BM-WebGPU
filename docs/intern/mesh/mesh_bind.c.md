# src/mesh/mesh_bind.c

This file binds and draws each item of a view in the pass of a round.

## A draw

`mesh_draw_item` binds the pipeline of the item, its group 1 and its vertex buffers, then it draws:

| Shape | Vertex buffers | The draw |
| --- | --- | --- |
| Triangles, lines and points | The vertices at 0, the records of the round at 1, and the defaults at 2. | `count` vertices, one instance, and the record of the item as `firstInstance`. |
| Wide points | The vertices at 0, the record of the item at 1, and the defaults at 2. | 6 vertices, and `count` instances. |
| Wide lines | The vertices at 0, the record of the item at 1, the vertices from the second vertex at 2, and the defaults at 3. | 6 vertices, and `count` divided by 2 instances. |

`bind_states` binds the pipeline and the group 1, and `bind_buffers` binds the vertex buffers.
The defaults are bound only when the `layout` of the item lacks a part. Each binding takes the rest of its buffer, from its offset.
`docs/intern/mesh/mesh_vertex.c.md`, *The vertex buffers*, tells what each pipeline reads from them.
A direct draw takes a `firstInstance` above 0 with no feature of the device: https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-core/src/command/render.rs, lines 2634 to 2666.

## What the pass binds

`mesh_pass_t` keeps what the pass binds now. `bind_buffer` keeps the buffer and the offset at each index, and makes no call that binds them again.
`bind_states` does the same for the pipeline and the group 1.
Thus a run of thin draws binds the records of the round once, and each draw then binds only its own vertices.

## Functions

| Function | Result |
| --- | --- |
| `mesh_draw_item` | Binds the pipeline, the group 1 and the vertex buffers of an item, and draws it. |
| `bind_states` | Binds the pipeline and the group 1 of an item, unless they are bound already. |
| `bind_buffers` | Binds the vertex buffers of an item, as its shape needs them. |
| `bind_buffer` | Binds a vertex buffer at an index, unless that buffer and offset are bound there already. |
