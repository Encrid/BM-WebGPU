# src/mesh_round.c

This file holds the system `render`. It gives the order of the steps of a round, and the release of all the GPU objects.

## A round

`bridge_render` follows the steps of `docs/extern/webgpu_mesh.h.md`, *The system render*:

```pseudo
FUNCTION bridge_render(f : Frame) RETURNS I32
   IF f.state = WEBGPU_FRAME_NO_DEVICE THEN
      CALL mesh_release_all()
      RETURN 0
   ENDIF
   IF f.state <> WEBGPU_FRAME_READY THEN
      RETURN 0
   ENDIF
   IF state.frame.device <> 0 AND NOT is_same_device(state.frame, f) THEN
      CALL mesh_release_all()
      RETURN 0
   ENDIF
   IF state.left THEN
      meshes <- mesh_sweep(MESH_GPU, FALSE)
      textures <- mesh_sweep(TEXTURE_GPU, FALSE)
      state.left <- meshes OR textures
   ENDIF
   IF state.frame.device = 0 THEN
      CALL mesh_make_device(f)
   ENDIF
   IF NOT read_round(f) THEN
      RETURN 1
   ENDIF
   CALL mesh_encode(f)
   RETURN 0
ENDFUNCTION
```

The pseudo code takes a frame that is not there as a frame of the state `WEBGPU_FRAME_NO_DEVICE`.
`is_same_device` compares the copy that the state keeps with the four fields of `webgpu:Frame` that `docs/extern/webgpu_mesh.h.md`, *The system render*, names.
The detaches of a release apply at the next barrier, so no upload of the next round meets an object of the old device.
`state.left` tells that a detach found no room: `docs/intern/mesh_release.c.md`, *No room for a detach*.

## The read

`read_round` does these steps:

1. `mesh_reserve_round` grows the arrays of the round: `docs/intern/mesh_memory.c.md`, *The arrays of a round*.
2. It clears `said` of the state, and keeps where the frame and the scene of the round are.
3. With `mesh_walk`, it gives each mesh to `mesh_keep_mesh`, then each texture to `mesh_keep_texture`.
4. With `mesh_walk`, it gives each view, each draw and each batch to its visit in `src/mesh_draw.c`.
5. It sorts the views and the items with `compare_keys`.
6. `mesh_put_round` puts the blocks, the records and the vertices of the batches into the round buffer.

Step 3 comes before step 4, so a draw finds the objects of each upload of its round.
For a round with no `webgpu:Scene`, step 2 keeps where a constant scene of zeros is.
If the arrays of the round cannot grow, `read_round` gives false, and `render` acts as `docs/extern/webgpu_mesh.h.md`, *Messages*, tells.

## The order

`compare_keys` orders two keys by `rank`, then by `order`, then by `id`.
A view has the rank 0. A draw has the rank 0, and a batch has the rank 1, so in each view the draws come before the batches.
Two items of one `order` keep the order of their entity ids, the same on each run.
`compare_keys` gives -1, 0 or 1, and does not subtract, so a large `order` cannot overflow.

## The release of all

`mesh_release_all` sweeps each `webgpu:MeshGpu` and each `webgpu:TextureGpu` with `all`, and then releases the objects of the device.
It sets `state.left` when a detach of the sweep found no room.
`render` calls it at a lost device and at a new device, and `bridge_shutdown` calls it at the stop.

## Functions

| Function | Result |
| --- | --- |
| `bridge_render` | Runs the steps of a round. Gives 0, or 1 when the arrays of the round cannot grow. |
| `read_round` | Uploads what changed, and fills the sorted views and items of the round. Gives false when its arrays cannot grow. |
| `compare_keys` | Orders two sort keys by rank, then order, then entity. |
| `mesh_release_all` | Releases the objects of each GPU component and of the device. |
