# src/mesh/mesh_vertex.c

This file gives the vertex state of each pipeline: its entry point and its vertex buffers.

## The vertex buffers

`mesh_set_buffers` gives the vertex buffers of a pipeline, by its shape:

| Index | Thin shapes | Wide points | Wide lines |
| --- | --- | --- | --- |
| 0 | The vertices, by vertex, with the stride of `layout`. | The vertices, by instance. | The first vertex of each segment, by instance, with twice the stride. |
| 1 | The records, by instance, with a stride of 92. | The record, with a stride of 0. | The record, with a stride of 0. |
| 2 | The defaults, with a stride of 0. | The defaults, with a stride of 0. | The second vertex of each segment, by instance, with twice the stride. |
| 3 | No buffer. | No buffer. | The defaults, with a stride of 0. |

`set_parts` gives the parts of one vertex. Each part of `layout` is at its offset in the vertex: `docs/extern/mesh/webgpu_mesh.h.md`, *The vertices*.
Each part that `layout` lacks reads the buffer of the defaults. It holds a normal of 0 at the offset 0, a texture coordinate of 0 at 12, and white at 20.
The locations 0 to 3 hold the position and the three parts. The locations 4 to 10 hold the record, and 11 to 14 the second vertex of a segment.

A stride of 0 gives one record to each instance of a wide item, and one value of the defaults to each vertex.
wgpu-hal reads a stride of 0 on Metal with a constant step function: https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/metal/device.rs, lines 1380 to 1394.
The floor of GL in `docs/extern/mesh/webgpu_mesh.h.md`, *The widths*, comes from this stride. Below that floor, GL has no binding of a vertex buffer with its own stride.
There wgpu-hal 29.0.1 gives each stride to `glVertexAttribPointer`, which reads a stride of 0 as packed data.
The source is https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/gles/queue.rs, lines 1372 to 1391, and `adapter.rs` of the same folder, lines 609 to 612.

A wide line reads 15 attributes from 4 buffers, the most of the shapes. Thus each pipeline fits the default limits: 16 attributes and 8 vertex buffers.
When `layout` lacks no part, the pipeline has no buffer of the defaults. Thus no buffer of a pipeline has no attribute.
wgpu-native panics on a hole in the list of buffers. wgpu-core 29.0.1 drops a buffer with no attribute, and the backend then numbers the later buffers wrongly.

## The vertex state

`mesh_set_buffers` writes the buffers and their attributes into the arrays of the caller. Then it gives the vertex state of the pipeline.
The state names the shader module, the buffers and the entry point of the shape.
The thin shapes take `vs_thin`, the wide lines `vs_line`, and the wide points `vs_point`: `docs/intern/mesh/mesh_pipeline.c.md`, *The shaders*.

## Functions

| Function | Result |
| --- | --- |
| `mesh_set_buffers` | Writes the vertex buffers of a shape and a layout into the description of a pipeline, and gives the vertex state. |
| `set_parts` | Writes the attributes of one vertex: its parts from the vertices, and the parts that it lacks from the defaults. |
