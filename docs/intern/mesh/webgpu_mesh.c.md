# src/mesh/webgpu_mesh.c

The manifest of the module and its entries `init`, `shutdown` and `reload` are in this file.

## The manifest

`readme.md`, *wgpu_native*, tells the kind `wgpu_module` that makes the module. Thus the module names the library with no path of the machine.
Its macro `MANIFEST` holds the words of the manifest, and this file puts the macro into the section of the target: `.toolchain/docs/presets.md`, *native_module*.

## The start

`bridge_init` does these steps:

1. It writes zeros over the state, so a second load of the library starts clean.
2. `declare_settings` declares the types of the slots 0 to 6: `docs/intern/mesh/mesh_inside.h.md`, *The slots*.
3. `declare_tables` declares the types of the slots 7 to 13.
4. `mesh_add_systems` registers the systems and the observers: `docs/intern/mesh/mesh_systems.c.md`.

Each type states `BRIDGE_STRIP`, so an unload of the module destroys no entity of the app.
Each enum states `BRIDGE_TABLE`. Each component states the storage of its section in `docs/extern/mesh/webgpu_mesh.h.md`.
`webgpu:MeshGpu` and `webgpu:TextureGpu` state `BRIDGE_ON_DESTROY_KEEP`, and each other type states `BRIDGE_ON_DESTROY_REMOVE`.

The module reads `webgpu:Frame`, so it declares it and its enum `webgpu:webgpu_frame_state_t` from `webgpu_contract.h`.
Bridge refuses a second declarer with another descriptor, kind, storage or `on_destroy`. `docs/extern/contract/webgpu_contract.h.md`, *The declarations*, gives them all.

## The stop

`bridge_shutdown` keeps its api in the state. When the state holds a device, it gives each of the two sets to `mesh_sweep` of `src/mesh/mesh_release.c`, with `all`.
Then it calls `mesh_release_device` of `src/mesh/mesh_device.c` and `mesh_free_round` of `src/mesh/mesh_memory.c`. It gives 0.
Thus it releases the objects of each GPU component before Bridge removes them: `docs/intern/mesh/mesh_release.c.md`, *Each object goes once*.

With no device, no GPU component holds an object, because the loss of each device released them.
A load that fails runs `shutdown` before its first barrier, for example when Bridge refuses the settings.
Then Bridge has written no kept cell, so a sweep cannot read the index of a set.
The source is `bridge_load.c.md` of Bridge, in its section A load that fails.

`bridge_reload` gives 1, and Bridge never calls it, since the manifest states `reloadable=false`.

## Functions

| Function | Result |
| --- | --- |
| `bridge_init` | Writes zeros over the state, declares the 14 types, and calls `mesh_add_systems`. Gives 0. |
| `declare_settings` | Declares the three enums of the settings, `webgpu:MeshSettings`, the enum of the frame, `webgpu:Frame` and `webgpu:Scene`. |
| `declare_tables` | Declares the five tables that the app writes, and the two GPU components. |
| `bridge_shutdown` | Releases each GPU object and frees the arrays of the round. Gives 0. |
| `bridge_reload` | Gives 1. |
