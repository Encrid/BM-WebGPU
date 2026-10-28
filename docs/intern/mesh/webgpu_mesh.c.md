# src/webgpu_mesh.c

This file holds the manifest of the module, and its entries `init`, `shutdown` and `reload`.

## The manifest

The kind `native_module` gives the words of the manifest as the macro `MANIFEST`.
The file puts that macro into the section of the target, with the line that the preset of the module tells.

## The start

`bridge_init` does these steps:

1. It writes zeros over the state, so a second load of the library starts clean.
2. `declare_settings` declares the types of the slots 0 to 6: `docs/intern/mesh_inside.h.md`, *The slots*.
3. `declare_tables` declares the types of the slots 7 to 13.
4. `mesh_add_systems` registers the systems and the observers: `docs/intern/mesh_systems.c.md`.

Each type states `BRIDGE_STRIP`, so an unload of the module destroys no entity of the app.
Each enum states `BRIDGE_TABLE`. Each component states the storage of its section in `docs/extern/webgpu_mesh.h.md`.
`webgpu:MeshGpu` and `webgpu:TextureGpu` state `BRIDGE_ON_DESTROY_KEEP`, and each other type states `BRIDGE_ON_DESTROY_REMOVE`.

The module reads `webgpu:Frame`, so it declares it and its enum `webgpu:webgpu_frame_state_t` from `webgpu_contract.h`.
Bridge refuses a second declarer with another descriptor, kind, storage or `on_destroy`. The document of that header of BL-WebGPU-Contract gives them all.

## The stop

`bridge_shutdown` gives each of the two sets to `mesh_sweep` of `src/mesh_release.c`, with `all`.
Then it calls `mesh_release_device` of `src/mesh_device.c` and `mesh_free_round` of `src/mesh_memory.c`. It gives 0.
Thus it releases the objects of each GPU component before Bridge removes them: `docs/intern/mesh_release.c.md`, *Each object goes once*.

`bridge_reload` gives 1. Bridge needs the entry, but never calls it, because the manifest states `reloadable=false`.

## Functions

| Function | Result |
| --- | --- |
| `bridge_init` | Writes zeros over the state, declares the 14 types, and calls `mesh_add_systems`. Gives 0. |
| `declare_settings` | Declares the three enums of the settings, `webgpu:MeshSettings`, the enum of the frame, `webgpu:Frame` and `webgpu:Scene`. |
| `declare_tables` | Declares the five tables that the app writes, and the two GPU components. |
| `bridge_shutdown` | Releases each GPU object and frees the arrays of the round. Gives 0. |
| `bridge_reload` | Gives 1. |
