# src/mesh_release.c

This file releases the GPU objects that `webgpu:MeshGpu` and `webgpu:TextureGpu` hold.
It holds the observers `release_mesh` and `release_texture`, and the sweep of the two sets.

## A release

`mesh_release_gpu` gets a slot, the value of a component, and the id of its entity. It does these steps:

1. If the main object is not 0, it releases each object of the value.
2. It gives the bytes of the objects back to `held`.
3. It writes zeros over the whole value, in place.
4. If the id is not 0, it detaches the component, when `changes` has room.

It gives false only when a detach found no room.
A caller that keeps the component gives the id 0. These callers are the keep of an upload, a sweep with `all`, and the release of the white texture.
A buffer goes with `wgpuBufferRelease`. A texture goes with `wgpuBindGroupRelease`, `wgpuTextureViewRelease` and `wgpuTextureRelease`, in this order.
Each object holds one reference of the module, and its call `Release` gives it back once: the pairs of `leaks.toml` of BL-WGPU-Native.

## The observers

`release_mesh` and `release_texture` give the slot of their component to `release_list`.
`release_list` reads the list of the observer that runs by `system` of the header: `bridge_abi.h.md` of Bridge, in its section bridge_header_t.
For each entity of the list, `release_list` finds the component with `mesh_find`, and gives it with the id to `mesh_release_gpu`.
`release_list` sets the flag `left` of its set when a detach found no room. An entity with no component needs nothing.

Each entity of the list has lost its `webgpu:Mesh` or its `webgpu:Texture`. Thus no upload writes into its component before the detach applies.
A husk goes at the barrier that applies the detach of its last kept component.

## The sweep

`mesh_sweep` reads the index of one set, cell by cell, and builds the id of each entity from the entity index.
With `all`, it first gives each component of the set to `mesh_release_gpu` with the id 0. Thus each component stays, with no object.
Then it detaches each component of zeros whose entity no longer carries the mesh or the texture.
It gives true when a detach found no room.

## No room for a detach

This section is how a detach waits, as `docs/extern/webgpu_mesh.h.md`, *The GPU objects*, tells.
A detach that finds no room leaves a component of zeros, and sets the flag `left` of its set.
An entity that still carries its mesh or its texture uploads into that component again, as `docs/intern/mesh_upload.c.md`, *Keep by gen*, tells.
A husk keeps the component until a sweep detaches it. While the flag of a set is set, the system of that set sweeps it without `all` before its uploads.
The sweep is in `docs/intern/mesh_round.c.md`, *The sweep of a set*.

The sweep reads each cell of the index, so its cost grows with `entities`.
A sweep without `all` runs only after a detach found no room. A sweep with `all` runs at a lost device, at a new device and at the stop.

## Each object goes once

| How an object goes | What releases it |
| --- | --- |
| The app destroys the entity. | The component stays on the husk. The observer releases the objects at the barrier of the destroy, and detaches the component. |
| The app detaches `webgpu:Mesh` or `webgpu:Texture`. | The observer releases the objects at that barrier, and detaches the component. |
| The `gen` changes. | The keep of `src/mesh_upload.c` releases the old objects, then writes the new objects in place. |
| The device is lost, or a new device comes. | `keep_meshes` and `keep_textures` sweep their sets with `all`. Each component stays, with no object, unless its entity lost its item. `keep_meshes` also releases the objects of the device. |
| The module stops. | `bridge_shutdown` makes the same release, before Bridge removes the components. |

Each row releases through `mesh_release_gpu`, which writes zeros. Thus when two rows meet one component, the second release finds no object.
For example, a destroy in the round of a lost device gives a sweep and an observer, and only the first one releases.

No GPU object of an entity lives outside its component, as `docs/intern/mesh_upload.c.md`, *Keep by gen*, tells. Thus no such object can go by a way outside this table.
A detach that waits for its barrier never meets an upload in place, since its entity carries no mesh or texture.

## Functions

| Function | Result |
| --- | --- |
| `bridge_release_mesh` | Releases the objects of each mesh that left, and detaches their components. Gives 0. |
| `bridge_release_texture` | Releases the objects of each texture that left, and detaches their components. Gives 0. |
| `release_list` | Releases and detaches the GPU component of each entity of the list of an observer. |
| `mesh_sweep` | Releases the objects of each component of one set with `all`, and detaches each component of zeros whose entity lost its item. Gives true when a detach found no room. |
| `mesh_release_gpu` | Releases the objects of one value, and writes zeros over it. Gives false when its detach found no room. |
