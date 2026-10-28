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
4. It adds the system `render` in the phase `render`.
5. It registers the observers `release_mesh` and `release_texture`. Each one hears `BRIDGE_ON_LEAVE`.

Each type states `BRIDGE_STRIP`, so an unload of the module destroys no entity of the app.
Each enum states `BRIDGE_TABLE`. Each component states the storage of its section in `docs/extern/webgpu_mesh.h.md`.
`webgpu:MeshGpu` and `webgpu:TextureGpu` state `BRIDGE_ON_DESTROY_KEEP`, and each other type states `BRIDGE_ON_DESTROY_REMOVE`.

The module reads `webgpu:Frame`, so it declares it with the C type and the descriptor of `webgpu_device.h`.
Bridge refuses a second declarer with another descriptor, kind, storage or `on_destroy`. The header of `webgpu:device` gives them all.

| Statement | Terms | Changes |
| --- | --- | --- |
| `render` | The ten components of the slots 3 and 5 to 13, each as `BRIDGE_OPTIONAL`. It writes the two GPU components, and reads the others. | `webgpu:MeshGpu` and `webgpu:TextureGpu` |
| `release_mesh` | `webgpu:Mesh` as `BRIDGE_AND` that it reads, and `webgpu:MeshGpu` as `BRIDGE_OPTIONAL` that it writes. | `webgpu:MeshGpu` |
| `release_texture` | `webgpu:Texture` as `BRIDGE_AND` that it reads, and `webgpu:TextureGpu` as `BRIDGE_OPTIONAL` that it writes. | `webgpu:TextureGpu` |

A query with only optional terms matches each table, so `render` sees each row of each component.
An observer hears an entity by its term of `BRIDGE_AND` alone. Its optional term lets it write the GPU component in place.

## The stop

`bridge_shutdown` calls `mesh_release_all` of `src/mesh_round.c`, then `mesh_free_round` of `src/mesh_memory.c`. It gives 0.
An unload and `bridge_destroy` remove the GPU components after `shutdown`, also from each husk. Thus `shutdown` must release their objects.

`bridge_reload` gives 1. Bridge needs the entry, but never calls it, because the manifest states `reloadable=false`.

## Functions

| Function | Result |
| --- | --- |
| `bridge_init` | Writes zeros over the state, declares the 14 types, and registers the system and the two observers. Gives 0. |
| `declare_settings` | Declares the three enums of the settings, `webgpu:MeshSettings`, the enum of the frame, `webgpu:Frame` and `webgpu:Scene`. |
| `declare_tables` | Declares the five tables that the app writes, and the two GPU components. |
| `bridge_shutdown` | Releases each GPU object and frees the arrays of the round. Gives 0. |
| `bridge_reload` | Gives 1. |
