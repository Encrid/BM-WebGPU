# src/mesh_memory.c

This file holds the memory of a round: its arrays in the module, and the round buffer on the device.

## The arrays of a round

`mesh_reserve_round` grows the arrays of one part of the round, so that they hold more entries than the round holds now.
The part `ITEMS` is the items and their records, which `draws` and `batches` grow. The part `VIEWS` is the views and their blocks, which `render` grows.
In the array of the blocks, the blocks are `BLOCK` bytes apart, as in the round buffer. Thus one call writes them all.

Each array stays for the next rounds, so a round that is not larger than the rounds before it allocates nothing.
`mesh_reserve_round` gives false when `realloc` cannot grow an array. `mesh_free_round` frees the arrays at the stop.

## The round buffer

The round buffer holds the data of a round for the shaders, in three parts. It is a uniform buffer and a vertex buffer that a copy can write.

| Part | Holds |
| --- | --- |
| The blocks | The block of each view, one each `BLOCK` bytes. No device asks for a `minUniformBufferOffsetAlignment` above 256. |
| The records | The record of each item, 92 bytes each. |
| The batches | The vertices of each batch, after those of the batch before it. |

`mesh_put_round` gives the bytes of the round to `grow_round` when they pass the size of the buffer.
A round with no view draws nothing, so it writes nothing and does not grow the buffer.
`grow_round` takes the next power of 2 that holds them, so a slow growth makes few new buffers.
`mesh_take_memory` takes the bytes of the growth. Then `grow_round` releases the old buffer and its bind group 0, and makes the new ones.

If `gpu_memory` refuses the new size, `mesh_take_memory` gives its line to `mesh_say_no_room`.
If `maxBufferSize` refuses it, `grow_round` gives the rule of the round to `mesh_say_left_out`, with the id 0.
In both cases, the round keeps the old buffer and sets its count of views to 0.
Thus the round acts as `docs/extern/webgpu_mesh.h.md`, *webgpu:MeshSettings*, tells for a buffer of a round that cannot grow.

Then `mesh_put_round` writes the blocks and the records, with one call each. It writes the vertices of each batch from the arena, with no copy in the module.
`mesh_release_round` releases the round buffer and its bind group 0 with the other objects of the device, and gives the bytes of the buffer back.

## Functions

| Function | Result |
| --- | --- |
| `mesh_reserve_round` | Grows the arrays of one part of the round for more entries. Gives false when the memory cannot grow. |
| `mesh_put_round` | Grows the round buffer when the round needs it, then writes the round into it. |
| `grow_round` | Makes a larger round buffer and its bind group 0. Gives false when `gpu_memory` or the device refuses the size. |
| `mesh_release_round` | Releases the round buffer and its bind group 0, and gives their bytes back. |
| `mesh_free_round` | Frees the arrays of the round. |
