# src/mesh/mesh_round.c

This file holds the systems `keep_meshes` and `keep_textures`. They start each round, and keep the uploads of the meshes and of the textures.

## The start of a round

`keep_meshes` runs first in the phase `render`: `docs/intern/mesh/mesh_systems.c.md`, *The order*.
It does the steps of `docs/extern/mesh/webgpu_mesh.h.md`, *The systems*, that come before the draws:

```pseudo
FUNCTION bridge_keep_meshes(f : Frame) RETURNS I32
   CALL open_round(f)
   CALL sweep_set(MESH_GPU)
   IF state.release THEN
      CALL mesh_release_device()
   ENDIF
   IF state.ready THEN
      IF state.frame.device = 0 THEN
         CALL mesh_make_device(f)
      ENDIF
      CALL mesh_walk(KEEP_MESHES, MESH, mesh_keep_mesh)
   ENDIF
   RETURN 0
ENDFUNCTION
```

The pseudo code takes a frame that is not there as a frame of the state `WEBGPU_FRAME_NO_DEVICE`.
`keep_textures` runs next. It gives `TEXTURE_GPU` to `sweep_set`, and in a ready round it gives each texture to `mesh_keep_texture`.
Each system first keeps its api in the state, so that the helpers of `src/mesh/mesh_lines.c` write with it.

## The flags of a round

`open_round` sets the flags `release` and `ready` of the state. The systems after it read them:

```pseudo
PROCEDURE open_round(f : Frame)
   lost <- f.state = WEBGPU_FRAME_NO_DEVICE
   moved <- f.state = WEBGPU_FRAME_READY AND state.frame.device <> 0
   moved <- moved AND NOT is_same_device(state.frame, f)
   waits <- state.left[MESH_GPU] OR state.left[TEXTURE_GPU]
   state.release <- moved OR (lost AND (state.frame.device <> 0 OR waits))
   state.ready <- f.state = WEBGPU_FRAME_READY AND NOT moved
   state.said_room <- FALSE
   state.said_rule <- FALSE
ENDPROCEDURE
```

`open_round` also sets the counts of the round to 0: the views, the items and the bytes of the batches.
`is_same_device` compares the copy that the state keeps with the four fields of `webgpu:Frame` that `docs/extern/mesh/webgpu_mesh.h.md`, *The systems*, names.

## The sweep of a set

`sweep_set` sweeps one set, as the flags of the round tell:

```pseudo
PROCEDURE sweep_set(set : U32)
   IF state.release THEN
      state.left[set] <- mesh_sweep(set, TRUE)
   ENDIF
   IF state.ready AND state.left[set] THEN
      state.left[set] <- mesh_sweep(set, FALSE)
   ENDIF
ENDPROCEDURE
```

A round that releases is never ready, so one call makes at most one sweep.
The release writes zeros over each GPU component in place. Thus the next ready round uploads each item into its component, with the objects of the new device.
`left` of the state holds a flag for each set. A flag tells that a detach found no room: `docs/intern/mesh/mesh_release.c.md`, *No room for a detach*.
With no objects of a device and no detach that waits, a round with no frame sweeps nothing.

## Functions

| Function | Result |
| --- | --- |
| `bridge_keep_meshes` | Starts the round, makes or releases the objects of the device, and keeps each mesh. Gives 0. |
| `bridge_keep_textures` | Sweeps the set of the textures, and keeps each texture. Gives 0. |
| `open_round` | Sets the flags and the counts of a round from the frame. |
| `sweep_set` | Sweeps one set at a lost device or a new device, or after a detach that found no room. |
| `is_same_device` | Tells if the frame has the same four fields as the copy that the state keeps. |
