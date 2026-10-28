# src/mesh_draw.c

This file turns the rows of `webgpu:View`, `webgpu:Draw` and `webgpu:Batch` into the views and the items of a round.
It fills the block of each view, and the record of each item.

## A view

`mesh_add_view` does these steps for each row of `webgpu:View`:

1. A `rect` with a width of 0 becomes the whole frame.
2. The viewport is the whole `rect`, also where it passes the frame on the right or at the bottom.
3. The scissor is the part of `rect` inside the frame. Its sums have 64 bits.
4. It applies the rules of `docs/extern/webgpu_mesh.h.md`, *webgpu:View*: first to the size of `rect`, then to the scissor.
5. It fills the block: `matrix`, the light of the scene, and the width and the height of `rect`.

`mesh_add_view` gives a view that a rule leaves out to `mesh_say_left_out`, with the id of the view and the rule.

wgpu-core 29.0.1 accepts a viewport that passes the frame, up to that size, but it needs the scissor inside the frame.
Thus the projection of a view stays the same, and the pass draws only its part inside the frame.
The source is https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-core/src/command/render.rs, lines 2545 to 2571 and 2597 to 2600.

## An item

`mesh_add_draw` and `mesh_add_batch` give an item and its record to `add_item`:

| Field | Draw | Batch |
| --- | --- | --- |
| The key | The rank 0, `order` and the entity. | The rank 1, `order` and the entity. |
| `views` and `depth` | Those of the draw. | Those of the batch. |
| The vertices | `buffer` of the `webgpu:MeshGpu` of `mesh`, with its `count`, `layout` and `primitive`. | The place of the batch in the round buffer, with its `layout` and `primitive`. |
| The bind group 1 | The group that `get_group` gives. | The group of the white texture. |
| `model` | `model` | The identity. |
| `color` | `color` | White. |
| `bias` | `bias` | 0. |
| `size` | `size` | `size` |
| `shading` | `shading` | 0, unlit. |

`mesh_add_draw` leaves out a draw whose `shading` is above 2. Then it looks at the `mesh` of the draw:

| The `mesh` of the draw | The draw |
| --- | --- |
| 0, a dead id, or an entity with no `webgpu:Mesh` | Left out, with the rule `it names no mesh`. |
| An entity with `webgpu:Mesh` and no `webgpu:MeshGpu` | None: the draw waits for the next round. |
| An entity whose `webgpu:MeshGpu` holds zeros | Left out with no line of its own: the line of the mesh tells why. |
| An entity whose `webgpu:MeshGpu` holds a buffer | It draws. |

`mesh_add_batch` leaves out each batch that a rule of `docs/extern/webgpu_mesh.h.md`, *The data rules*, refuses: first `layout`, then `primitive`, then the data.
`mesh_add_draw` and `mesh_add_batch` give an item that a rule leaves out to `mesh_say_left_out`, with the id of the item and the rule.
The vertices of a batch come after the vertices of the batch before it, in the order of the walk.

`add_item` gives the item the next record. The sort of the round moves the items and not the records, so each item keeps its record.
The shape of an item is its `primitive`, or a wide shape when `size` is above 1 for lines or points.
`add_item` keeps only the two bits of `depth` that `docs/extern/webgpu_mesh.h.md`, *webgpu:Draw*, names.

## The texture of a draw

`get_group` gives the bind group 1 of a draw:

| The `texture` of the draw | The group |
| --- | --- |
| 0 | The group of the white texture. |
| An entity with no `webgpu:Texture` | The group of the white texture. |
| An entity with `webgpu:Texture` and no `webgpu:TextureGpu` | None: the draw waits for the next round. |
| An entity whose `webgpu:TextureGpu` holds zeros | The group of the white texture. |
| An entity whose `webgpu:TextureGpu` holds a texture | The group of that texture. |

The third row is the wait of a draw for a new texture: `docs/extern/webgpu_mesh.h.md`, *The GPU objects*. Thus such a draw never shows white for one round.

## Functions

| Function | Result |
| --- | --- |
| `mesh_add_view` | Adds a view to the round with its block, or leaves the view out. |
| `mesh_add_draw` | Adds a draw to the round with its record, or leaves the draw out. |
| `mesh_add_batch` | Adds a batch to the round with its record and the place of its vertices, or leaves the batch out. |
| `add_item` | Adds an item and its record to the round, with the shape of its primitive and of its size. |
| `get_group` | Gives the bind group 1 of a draw, or 0 while its texture waits for its upload. |
