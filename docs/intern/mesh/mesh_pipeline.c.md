# src/mesh_pipeline.c

This file holds the WGSL shaders of the module, its layouts and its pipelines.
It also holds the quads of the wide points and of the wide lines.

## The shaders

The WGSL text is a constant text of the file, with four entry points:

| Entry point | Gives |
| --- | --- |
| `vs_thin` | A vertex of triangles, of thin lines or of thin points. |
| `vs_point` | A corner of the square of a wide point. |
| `vs_line` | A corner of the band of a segment of a wide line. |
| `fs_main` | The color of a fragment. |

Each vertex entry point places a vertex with `place`:

```pseudo
FUNCTION place(r : Record, b : Block, p : Part) RETURNS Out
   o.position <- multiply(b.matrix, multiply(r.model, point(p.position)))
   o.position.z <- o.position.z - r.bias * o.position.w
   o.color <- r.color * p.color
   IF r.shading = 1 THEN
      o.color <- lambert(o.color, multiply(r.model, direction(p.normal)), b)
   ENDIF
   o.uv <- p.uv
   o.shading <- r.shading
   RETURN o
ENDFUNCTION
```

`point` gives the position with a `w` of 1. `direction` gives the normal with a `w` of 0, so the normal turns with `model` and does not move.
The second line makes the depth after the division by `w` smaller by `bias`.
`lambert` applies `docs/extern/webgpu_mesh.h.md`, *The shading*, to the red, the green and the blue.

```pseudo
FUNCTION fs_main(i : Out) RETURNS Color
   t <- textureSample(texture, sampler, i.uv)
   IF i.shading = 2 THEN
      RETURN i.color * t
   ENDIF
   RETURN i.color
ENDFUNCTION
```

`fs_main` samples before it tests `shading`, because WGSL refuses `textureSample` in control flow that is not uniform.

## The quads

A wide point is a square of `size` pixels, and each segment of a wide line is a band of `size` pixels.
Each one is one instance of two triangles. The vertex index, from 0 to 5, gives its corner.
A corner moves its end by half of `size` in pixels. In clip space, that is `size` divided by the width or the height of the view, times `w`.

`vs_line` places the two ends of its segment, then trims an end that lies before the near plane:

```pseudo
PROCEDURE trim(BYREF a : Out, BYVAL b : Out)
   IF a.position.z < 0.0 AND b.position.z >= 0.0 THEN
      a.position <- mix(a.position, b.position, a.position.z / (a.position.z - b.position.z))
   ENDIF
ENDPROCEDURE
```

`vs_line` trims each end in turn. Then the direction of the segment on the screen has a meaning.
The band moves each end at a right angle to that direction. A segment of length 0 takes the direction of the x axis.
When the two ends lie before the near plane, the GPU clips the whole band.

The color, the texture coordinate and the normal of a corner are those of its end.
The three.js class `LineMaterial` trims a segment in the same way: https://github.com/mrdoob/three.js/blob/a2cc62d47a69f7c9a90ee0b08d9b0e3cddcdcd9c/examples/jsm/lines/LineMaterial.js, lines 122 to 155.

## The layouts

`mesh_make_layouts` makes the shader module, the two bind group layouts and the pipeline layout of each pipeline:

| Group | Binding | Holds | Stage |
| --- | --- | --- | --- |
| 0 | 0 | The block of a view: 128 bytes of a uniform buffer, with a dynamic offset. | Vertex |
| 1 | 0 | The texture of a draw. | Fragment |
| 1 | 1 | The sampler of its `filter`. | Fragment |

## The vertex buffers

`set_buffers` gives the vertex buffers of a pipeline, by its shape:

| Index | Thin shapes | Wide points | Wide lines |
| --- | --- | --- | --- |
| 0 | The vertices, by vertex, with the stride of `layout`. | The vertices, by instance. | The first vertex of each segment, by instance, with twice the stride. |
| 1 | The records, by instance, with a stride of 92. | The record, with a stride of 0. | The record, with a stride of 0. |
| 2 | The defaults, with a stride of 0. | The defaults, with a stride of 0. | The second vertex of each segment, by instance, with twice the stride. |
| 3 | No buffer. | No buffer. | The defaults, with a stride of 0. |

`set_parts` gives the parts of one vertex. Each part of `layout` is at its offset in the vertex: `docs/extern/webgpu_mesh.h.md`, *The vertices*.
Each part that `layout` lacks reads the buffer of the defaults. It holds a normal of 0 at the offset 0, a texture coordinate of 0 at 12, and white at 20.
The locations 0 to 3 hold the position and the three parts. The locations 4 to 10 hold the record, and 11 to 14 the second vertex of a segment.

A stride of 0 gives one record to each instance of a wide item, and one value of the defaults to each vertex.
wgpu-hal reads a stride of 0 on Metal with a constant step function: https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/metal/device.rs, lines 1380 to 1394.
The floor of GL in `docs/extern/webgpu_mesh.h.md`, *The widths*, comes from this stride. Below that floor, GL has no binding of a vertex buffer with its own stride.
There wgpu-hal 29.0.1 gives each stride to `glVertexAttribPointer`, which reads a stride of 0 as packed data.
The source is https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/gles/queue.rs, lines 1372 to 1391, and `adapter.rs` of the same folder, lines 609 to 612.

A wide line reads 15 attributes from 4 buffers, the most of the shapes. Thus each pipeline fits the default limits: 16 attributes and 8 vertex buffers.
When `layout` lacks no part, the pipeline has no buffer of the defaults. Thus no buffer of a pipeline has no attribute.
wgpu-native panics on a hole in the list of buffers. wgpu-core 29.0.1 drops a buffer with no attribute, and the backend then numbers the later buffers wrongly.

## The pipelines

`mesh_get_pipeline` gives the pipeline of a `layout`, a shape and the bits 0 and 1 of `depth`. It makes the pipeline at its first use:

```pseudo
FUNCTION mesh_get_pipeline(layout : U32, shape : U32, depth : U32) RETURNS U32
   i <- layout + 8 * (shape + 5 * depth)
   IF state.pipelines[i] = 0 THEN
      state.pipelines[i] <- make_pipeline(layout, shape, depth)
   ENDIF
   RETURN state.pipelines[i]
ENDFUNCTION
```

Step 3 of `docs/extern/webgpu_mesh.h.md`, *The systems*, makes no pipeline in advance. wgpu-native has no call that makes a pipeline in the background.
Thus the round of the first draw of a pipeline makes it, and a pipeline that no draw needs is never made.
`make_pipeline` gives each pipeline these states:

| State | Value |
| --- | --- |
| The vertex | The entry point of the shape, and the buffers of `set_buffers`. |
| The primitive | A list of triangles, of lines or of points. A wide shape draws a list of triangles. |
| The faces | `cull` and `clockwise` of the settings of the device. A wide shape culls no face, since a point or a line has no face. |
| The depth | None when `depth_format` is 0. Else `depth_compare` when bit 0 of `depth` is set, else always, and a write when bit 1 is set. |
| The blend | With `blend`: the source alpha and one minus the source alpha for the color, and the factors one and zero for the alpha. |
| The samples | `samples` of the frame. |
| The color target | `format` of the frame. |

The settings of the device are the copy that `docs/intern/mesh_device.c.md`, *The objects of a device*, tells.
The pipelines of a `depth_format` of 0 have no depth state. Thus a frame with no depth texture acts as `docs/extern/webgpu_mesh.h.md`, *webgpu:MeshSettings*, tells.
The descriptor of each object gives a label that starts with `webgpu:mesh:`, because wgpu-native has no call `SetLabel`.

## Functions

| Function | Result |
| --- | --- |
| `mesh_make_layouts` | Makes the shader module, the two bind group layouts and the pipeline layout. |
| `mesh_get_pipeline` | Gives the pipeline of a layout, a shape and two bits of `depth`, made at its first use. |
| `make_pipeline` | Makes one pipeline with the states of the frame and of the settings of the device. |
| `set_buffers` | Writes the vertex buffers of a shape and a layout into the description of a pipeline. |
| `set_parts` | Writes the attributes of one vertex: its parts from the vertices, and the parts that it lacks from the defaults. |
