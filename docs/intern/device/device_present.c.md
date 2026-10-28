# src/device_present.c

This file holds the texture of the frame of each round, from `wgpuSurfaceGetCurrentTexture` to its present, and the system `present`.

## The texture of the frame

`device_take_frame` first releases the frame of the round before, if `present` did not take it.
wgpu-native discards a texture that was not presented: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L368-L383.

Then it gets the texture of the frame. wgpu-native stops the process when the surface holds a texture already, or has no configure: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L4161-L4213.
For `SuccessOptimal` and `SuccessSuboptimal`, it makes the view of the texture, with the label `webgpu:device frame` and the usage `RenderAttachment`.
The view follows the rule of the counts in `docs/intern/device_targets.c.md`, *The targets*. `SuccessSuboptimal` also sets `configure`.

For each other status, it releases the texture if there is one. Then `skip_frame` acts as `docs/extern/webgpu_device.h.md`, *The status of a frame*, tells:

| Status | `skip_frame` | Step |
| --- | --- | --- |
| `Timeout`, `Outdated` | Sets `configure`. | `DEVICE_WAITING` |
| `Lost` | Releases the surface, which `keep_device` makes again. | `DEVICE_WAITING` |
| `Occluded` of `wgpu.h` | Nothing. | `DEVICE_WAITING` |
| `Error`, and each other status | Writes its line. | `DEVICE_NO_FRAME` |

## The present

`bridge_present` presents the surface when the state of `webgpu:Frame` is `WEBGPU_FRAME_READY`. Then it gives the frame to `device_release_frame`.
`docs/extern/webgpu_device.h.md`, *The status of a frame*, tells why it reads no status of `wgpuSurfacePresent`: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L4216-L4233.

## Functions

| Function | Result |
| --- | --- |
| `device_take_frame` | Gets the texture of the frame and its view. Gives `DEVICE_READY`, else the step of `skip_frame`. |
| `skip_frame` | Acts on a status that gives no frame. Gives `DEVICE_WAITING`, or `DEVICE_NO_FRAME` for an error. |
| `bridge_present` | Presents the frame of the round, then releases it. Gives 0. |
| `device_release_frame` | Releases the view and the texture of the frame that are not 0. |
