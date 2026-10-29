# src/mesh_pass.c

This file encodes the render pass of a round, and draws each view.

## The pass

`mesh_encode` does these steps:

1. It makes a command encoder on the device of `webgpu:Frame`.
2. `begin_pass` begins the render pass on the frame.
3. `draw_view` draws each view of the round, in the sorted order.
4. It ends the pass, and makes the command buffer.
5. It submits the command buffer to `queue` of `webgpu:Frame`.
6. It releases the pass, the command buffer and the encoder.

`mesh_encode` keeps the pass in a `mesh_pass_t`, with the frame and two offsets in the round buffer.
The records start after the blocks of the views, and the batches after the records: `docs/intern/mesh_memory.c.md`, *The round buffer*.

`begin_pass` gives the pass these attachments. `get_color_attachment` gives the color, and `get_depth_attachment` gives the depth and the stencil:

| Attachment | Value |
| --- | --- |
| The color | `target`, which resolves into `frame` with more than 1 sample. `WEBGPU_LOAD_CLEAR` clears it to `clear` of the scene. Each other `load` loads it. |
| The depth | `depth`, when it is not 0. `WEBGPU_LOAD_CLEAR` clears it to `clear_depth`. Each other `load` loads it. |
| The stencil | Only for a depth format with a stencil. `WEBGPU_LOAD_CLEAR` clears it to 0. Each other `load` loads it. |

Each attachment stores what the pass drew, so a later render module with `WEBGPU_LOAD_KEEP` loads it.
A round with no view only clears, or draws nothing.
`begin_pass` clamps `clear_depth` as `docs/extern/webgpu_mesh.h.md`, *webgpu:MeshSettings*, tells. wgpu-core refuses a pass with a clear depth outside 0 to 1.
wgpu-core needs a load and a store of the stencil for a format with a stencil, and reads none for another format.
The source is https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-core/src/command/render.rs, lines 1696 to 1716.

## The errors of the commands

`mesh_encode` catches no error of its commands with a scope of `webgpu.h`.
wgpu-core finds most errors of a pass only when the encoder finishes: https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-core/src/command/render.rs#L1840.
Then the command buffer would have to stay out of the queue.

wgpu-native v29.0.1.1 drops a command buffer only after `wgpuQueueSubmit` takes it: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L111-L118 and https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L3084.
On D3D12 and on WARP, a command buffer that stays holds the back buffer. Then the next configure of the surface fails, and stops the process: https://github.com/gfx-rs/wgpu-native/issues/638.
Thus `mesh_encode` always submits, and an error in the commands stops the process: `docs/extern/webgpu_mesh.h.md`, *Messages*.

## A view

`draw_view` sets the viewport and the scissor of the view, and binds the group 0 at the block of the view.
The viewport is the whole `rect` of the view, also where it passes the frame on the right or at the bottom.
The scissor is the part of `rect` inside the frame. Its sums have 64 bits, so they cannot wrap.

wgpu-core 29.0.1 accepts a viewport that passes the frame, up to that size, but it needs the scissor inside the frame.
Thus the projection of a view stays the same, and the pass draws only its part inside the frame.
The source is https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-core/src/command/render.rs, lines 2545 to 2571 and 2597 to 2600.

The dynamic offset of the group is the index of the block times `BLOCK`.
Then `draw_view` gives each item that shares a bit with `mask` of the view to `mesh_draw_item`: `docs/intern/mesh_bind.c.md`, *A draw*.

## Functions

| Function | Result |
| --- | --- |
| `mesh_encode` | Encodes the pass of the round, submits it, and releases its objects. |
| `begin_pass` | Begins the render pass with the attachments of the frame and the settings. |
| `get_color_attachment` | Gives the color attachment: the target, its resolve, and the load and the clear of the round. |
| `get_depth_attachment` | Gives the attachment of the depth texture: its load, its clamped clear depth, and the stencil of its format. |
| `draw_view` | Sets the viewport, the scissor and the block of a view, and draws its items. |
