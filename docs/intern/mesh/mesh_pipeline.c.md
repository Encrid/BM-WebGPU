# src/mesh/mesh_pipeline.c

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

The struct `Block` of the WGSL text has the layout of `mesh_block_t`: `docs/intern/mesh/mesh_inside.h.md`, *Types*.
WGSL aligns a `vec3f` to 16 bytes, but `webgpu:Scene` packs each `float[3]`.
Thus `Block` reads the light, its color and the ambient color as nine scalars, and `size` at the offset 120.

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
`lambert` applies `docs/extern/mesh/webgpu_mesh.h.md`, *The shading*, to the red, the green and the blue.

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

Step 3 of `docs/extern/mesh/webgpu_mesh.h.md`, *The systems*, makes no pipeline in advance. wgpu-native has no call that makes a pipeline in the background.
Thus the round of the first draw of a pipeline makes it, and a pipeline that no draw needs is never made.
`make_pipeline` gives each pipeline these states:

| State | Value |
| --- | --- |
| The vertex | The entry point of the shape, and the buffers of `mesh_set_buffers`: `docs/intern/mesh/mesh_vertex.c.md`, *The vertex buffers*. |
| The primitive | A list of triangles, of lines or of points. A wide shape draws a list of triangles. |
| The faces | `cull` and `clockwise` of the settings of the device. A wide shape culls no face, since a point or a line has no face. |
| The depth | None when `depth_format` is 0. Else `depth_compare` when bit 0 of `depth` is set, else always, and a write when bit 1 is set. |
| The blend | With `blend`: the source alpha and one minus the source alpha for the color, and the factors one and zero for the alpha. |
| The samples | `samples` of the frame. |
| The color target | `format` of the frame. |

`get_primitive` gives the primitive and the faces. `get_depth_state` gives the depth.
The settings of the device are the copy that `docs/intern/mesh/mesh_device.c.md`, *The objects of a device*, tells.
The pipelines of a `depth_format` of 0 have no depth state. Thus a frame with no depth texture acts as `docs/extern/mesh/webgpu_mesh.h.md`, *webgpu:MeshSettings*, tells.

## The labels

wgpu-native has no call `SetLabel`, so the descriptor of each object gives its label.
Each label is `webgpu:mesh:` and the kind of the object: `buffer`, `texture`, `view`, `group`, `sampler`, `shader`, `block`, `image`, `layout`, `pipeline`, `encoder`, `commands` or `pass`.
`block` and `image` are the layouts of the groups 0 and 1, and `layout` is the pipeline layout.

## Functions

| Function | Result |
| --- | --- |
| `mesh_make_layouts` | Makes the shader module, the two bind group layouts and the pipeline layout. |
| `mesh_get_pipeline` | Gives the pipeline of a layout, a shape and two bits of `depth`, made at its first use. |
| `make_pipeline` | Makes one pipeline with the states of the frame and of the settings of the device. |
| `get_primitive` | Gives the primitive state of a shape: its topology, the front face, and the faces that it culls. |
| `get_depth_state` | Gives the depth state: its compare from bit 0 of `depth`, and its write from bit 1. |
