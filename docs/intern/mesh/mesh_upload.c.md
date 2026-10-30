# src/mesh/mesh_upload.c

This file makes the GPU objects of a mesh or of a texture again when its `gen` changes.
It also counts the bytes of `gpu_memory`.

## Keep by gen

`keep_meshes` gives each row of `webgpu:Mesh` to `mesh_keep_mesh`, and `keep_textures` gives each row of `webgpu:Texture` to `mesh_keep_texture`.
Each one does these steps:

1. It finds the GPU component of the entity with `mesh_find`.
2. With no component, it attaches a component of zeros, if `changes` has room. Then it stops.
3. If the component holds an object of the same `gen`, it stops.
4. `mesh_release_gpu` releases the old objects of the component, with no detach.
5. It writes the new value in place: the objects of the upload, or zeros when a rule refuses the item.

Steps 2 and 5 give the two rounds of a first upload: `docs/extern/mesh/webgpu_mesh.h.md`, *The GPU objects*.
Bridge drops an attach to a dead id, and a barrier that runs out of `tables` or `columns` applies no change of the entity.
The attach holds zeros, so such a loss loses no object. Thus each GPU object of an entity is in a component from the call that makes it.
A write in place takes no room in `changes`, and the draws of the same round find the new objects.
Step 3 needs an object, so a component of zeros tries its item again at each round.

## The upload of a mesh

`upload_mesh` refuses each mesh that a rule of `docs/extern/mesh/webgpu_mesh.h.md`, *The data rules*, leaves out. It tests the rules in this order:

1. `layout` is at most 7, so `STRIDES` gives the stride of its vertices.
2. `primitive` is at most 2.
3. `mesh_get_data` accepts the data, with the stride as the unit.
4. The length of the data is at most `maxBufferSize` of the limits of the device.

`upload_mesh` gives the first refusal to `mesh_say_left_out`, with the id of the mesh and the rule.
Then `upload_mesh` takes the length of the data from `gpu_memory`, and `mesh_make_buffer` makes the buffer.
The value also keeps `count`, `layout` and `primitive` from the upload. Thus each draw matches its buffer, also when the app changes these fields and keeps `gen`.

## The upload of a texture

`upload_texture` refuses each texture that a rule of the same section leaves out. It tests the rules in this order:

1. `filter` is at most 1.
2. `mesh_get_data` accepts the data, with the unit 1.
3. The length of the data is `width` times `height` times 4. This product has 64 bits, so it cannot wrap.
4. `width` and `height` are at most `maxTextureDimension2D` of the limits of the device.

`upload_texture` gives the first refusal to `mesh_say_left_out`, as `upload_mesh` does.
Then `upload_texture` takes these bytes from `gpu_memory`, and `mesh_make_texture` makes the objects.
The value keeps the `bytes` of the texture, so a release gives back what the upload took.

## gpu_memory

`mesh_take_memory` adds bytes to `held` of the state, if `held` then stays at most `gpu_memory`. Else it gives false.
The test does not add the two counts, so a large `gpu_memory` cannot wrap.

A refusal gives `held` and the bytes of the item to `mesh_say_no_room`: `docs/intern/mesh/mesh_lines.c.md`, *The lines*.

`held` counts each buffer and each texture that the module holds: the buffer of each mesh, the texture of each texture, and the round buffer.
It also counts the white texture and the buffer of the defaults. They are no items, so `mesh_make_device` adds their 28 bytes with no test.
Each release gives its bytes back.

## Functions

| Function | Result |
| --- | --- |
| `mesh_keep_mesh` | Attaches the `webgpu:MeshGpu` of a new mesh, or uploads a mesh whose `gen` changed into its component. |
| `mesh_keep_texture` | Attaches the `webgpu:TextureGpu` of a new texture, or uploads a texture whose `gen` changed into its component. |
| `upload_mesh` | Gives the value of a new buffer, or zeros with the `gen` when a rule refuses the mesh. |
| `upload_texture` | Gives the value of a new texture, or zeros with the `gen` when a rule refuses the texture. |
| `mesh_take_memory` | Takes bytes of `gpu_memory`. Gives false when they do not fit, and gives the line to `mesh_say_no_room`. |
