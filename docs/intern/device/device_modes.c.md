# src/device_modes.c

This file chooses the format, the present mode and the alpha mode of the surface, from the capabilities that the adapter gives for it.

## The capabilities

`device_choose_modes` gets the capabilities with `wgpuSurfaceGetCapabilities`. It chooses the three modes, then frees the lists with `wgpuSurfaceCapabilitiesFreeMembers`.
If the call fails, or it gives no format, it writes the second reason of `no surface`: `docs/extern/webgpu_device.h.md`, *Messages*. Then it gives false.
That reason is a text of the module, because wgpu-native gives the cause only to its logger: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L4087-L4099.

## The modes

| Setting | The mode that the module asks for | The module uses it if | Else |
| --- | --- | --- | --- |
| `format` | `formats[0]` for `WEBGPU_FORMAT_PREFERRED`. Else the `WGPUTextureFormat` of the table `FORMATS`. | `formats` holds it. | `formats[0]` |
| `present` | The same value of `WGPUPresentMode`. | `presentModes` holds it. | `WGPUPresentMode_Fifo` |
| `alpha` | The value of `WGPUCompositeAlphaMode`, which is 1 less. | It is `WGPUCompositeAlphaMode_Auto`, or `alphaModes` holds it. | `WGPUCompositeAlphaMode_Auto` |

Each choice of the column Else writes its line at the level 4: `docs/extern/webgpu_device.h.md`, *Messages*.
The second name of such a line is the constant that the module uses in place of the setting, for example `PREFERRED`.
`webgpu.h` puts `WGPUPresentMode_Fifo` in each list `presentModes`, and never puts `WGPUCompositeAlphaMode_Auto` in `alphaModes`: https://github.com/webgpu-native/webgpu-headers/blob/673658bc2bd70ec39fc55ebe6bb0173cf6d0a603/webgpu.h#L3020-L3036.
`has_item` tells if a list holds a value. The three lists hold enums of `webgpu.h` with no value below 0, so it reads them as `uint32_t`.

## Functions

| Function | Result |
| --- | --- |
| `device_choose_modes` | Chooses the format, the present mode and the alpha mode. Gives false, after its line, when the adapter gives no format. |
| `choose_format` | Gives the format of the frame. |
| `choose_present` | Gives the present mode. |
| `choose_alpha` | Gives the alpha mode. |
| `has_item` | Gives true when a list of the capabilities holds a value. |
