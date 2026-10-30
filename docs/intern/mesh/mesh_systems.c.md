# src/mesh/mesh_systems.c

This file gives Bridge the statements of the systems and of the observers of the module.

## The statements

`mesh_add_systems` registers each system of this table in its order, then the two observers. Each system has the phase `render`.

| Statement | Terms | Changes | Runs after |
| --- | --- | --- | --- |
| `keep_meshes` | `webgpu:Mesh` as `BRIDGE_AND`, `webgpu:MeshGpu` that it writes, `webgpu:Frame` and `webgpu:MeshSettings` | `webgpu:MeshGpu` | None |
| `keep_textures` | `webgpu:Texture` as `BRIDGE_AND`, `webgpu:TextureGpu` that it writes, and `webgpu:MeshSettings` | `webgpu:TextureGpu` | `keep_meshes` |
| `draws` | `webgpu:Draw` as `BRIDGE_AND`, `webgpu:MeshGpu` and `webgpu:TextureGpu` | None | `keep_textures` |
| `batches` | `webgpu:Batch` as `BRIDGE_AND` | None | `draws` |
| `render` | `webgpu:View` as `BRIDGE_AND`, `webgpu:Frame`, `webgpu:Scene` and `webgpu:MeshSettings` | None | `batches` |
| `release_mesh` | `webgpu:Mesh` as `BRIDGE_AND`, and `webgpu:MeshGpu` that it writes | `webgpu:MeshGpu` | None |
| `release_texture` | `webgpu:Texture` as `BRIDGE_AND`, and `webgpu:TextureGpu` that it writes | `webgpu:TextureGpu` | None |

Each other term is `BRIDGE_OPTIONAL`, and each term reads its component, unless the table tells that it writes it.
A singleton is a term of `BRIDGE_OPTIONAL`: `bridge_abi.h.md` of Bridge, in its section bridge_term_t.
`draws` reads the GPU components of other entities, by their ids. Its two optional terms tell the scheduler of Bridge that it reads them.
An observer hears an entity by its term of `BRIDGE_AND` alone. Its optional term lets it write the GPU component in place.

## The order

`before` of each system names the system before it in the table, so the five systems make one chain.
Thus each system runs after the system that writes what it reads: `bridge_abi.h.md` of Bridge, in its section Conflicts.
The state of the module carries the data of a round from one system to the next: `docs/intern/mesh/mesh_store.c.md`, *The state*.

`mesh_add_systems` registers each system with `BRIDGE_NATIVE_ADD_SYSTEM` of BB-Native. Its `before` is the slot of the system before it, and its `before_count` is 1.
`keep_meshes` has no system before it, so it gives `nullptr` and 0.
The macro takes the name of a system as a string literal. Thus each system has its own call, with the terms and the changes of its row.
The two observers do not conflict, so `BRIDGE_NATIVE_OBSERVER` registers each one with `nullptr` and 0 as its `before`.

## Functions

| Function | Result |
| --- | --- |
| `mesh_add_systems` | Registers the five systems in their order, and the two observers. |
