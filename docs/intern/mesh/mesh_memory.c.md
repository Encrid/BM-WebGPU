# src/mesh_memory.c

This file holds the memory of a round: its arrays in the module, and the round buffer on the device.

## The arrays of a round

`mesh_reserve_round` counts the rows of `webgpu:View`, `webgpu:Draw` and `webgpu:Batch` with `mesh_walk`.
Then it grows one block of the module to hold the views, the items, the blocks and the records of these counts.
The block stays for the next rounds, so a round that is not larger than the rounds before it allocates nothing.
`mesh_reserve_round` gives false when `realloc` cannot grow the block. `mesh_free_round` frees the block at the stop.

## The round buffer

The round buffer holds the data of a round for the shaders, in three parts. It is a uniform buffer and a vertex buffer that a copy can write.

| Part | Holds |
| --- | --- |
| The blocks | The block of each view, one each `BLOCK` bytes. No device asks for a `minUniformBufferOffsetAlignment` above 256. |
| The records | The record of each item, 92 bytes each. |
| The batches | The vertices of each batch, after those of the batch before it. |

`mesh_put_round` gives the bytes of the round to `grow_round` when they pass the size of the buffer.
`grow_round` takes the next power of 2 that holds them, so a slow growth makes few new buffers.
`mesh_take_memory` takes the bytes of the growth. Then `grow_round` releases the old buffer and its bind group 0, and makes the new ones.
If `gpu_memory` or `maxBufferSize` refuses the new size, the round keeps the old buffer and sets its count of views to 0.
Thus the round acts as `docs/extern/webgpu_mesh.h.md`, *webgpu:MeshSettings*, tells for a buffer of a round that cannot grow.

Then `mesh_put_round` writes the blocks and the records, with one call each. It writes the vertices of each batch from the arena, with no copy in the module.
`mesh_release_round` releases the round buffer and its bind group 0 with the other objects of the device, and gives the bytes of the buffer back.

## Functions

| Function | Result |
| --- | --- |
| `mesh_reserve_round` | Counts the rows of the round, and grows its arrays to them. Gives false when the memory cannot grow. |
| `mesh_put_round` | Grows the round buffer when the round needs it, then writes the round into it. |
| `grow_round` | Makes a larger round buffer and its bind group 0. Gives false when `gpu_memory` or the device refuses the size. |
| `mesh_release_round` | Releases the round buffer and its bind group 0, and gives their bytes back. |
| `mesh_free_round` | Frees the arrays of the round. |
