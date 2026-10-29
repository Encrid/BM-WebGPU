# src/device_targets.c

This file chooses the samples and the depth of the frame, and makes the two targets: the multisample texture and the depth texture.

## The choice

`device_choose_targets` runs before the request of the device. It gives the features that the targets need, for `WEBGPU_FEATURES_DEFAULT`:

1. It keeps `samples`. `checked` is `true` for 1 and 4 samples: `docs/extern/webgpu_device.h.md`, *webgpu:webgpu_samples_t*.
2. For 2, 8 and 16 samples, it adds `SAMPLES_FEATURE`, which is `WGPUNativeFeature_TextureAdapterSpecificFormatFeatures`, if the adapter has it.
3. It keeps the `WGPUTextureFormat` of `depth`, from the table `DEPTHS`. `WEBGPU_DEPTH_NONE` gives 0.
4. For `Depth32FloatStencil8`, it adds its feature if the adapter has it. Else it writes the line of `depth`, and uses `Depth24PlusStencil8`.

## The targets

`device_make_targets` releases the targets, then makes them at the size in `gpu`:

| Target | When | Format | Usages | Label |
| --- | --- | --- | --- | --- |
| `color` | `samples` is more than 1. | The format of the frame. | `RenderAttachment` | `webgpu:device color` |
| `depth` | `depth_format` is not 0. | `depth_format` | `RenderAttachment` and `CopySrc` | `webgpu:device depth` |

These usages are the usages that the contract gives to `target` and to `depth` of `webgpu:Frame`.
`make_target` makes the texture with the samples of the frame, then its view with the same label.
A view of the module states its counts of mip levels and of layers as undefined, because a count of 0 stops the process.
https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L4288-L4334 shows it.

## The check of the samples

Step 4 of the make checks 2, 8 or 16 samples, because wgpu-native gives no call that tells the sample counts of a format: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/ffi/wgpu.h#L1476-L1549.
`keep_device` sets the size to 1 by 1, then calls `device_make_targets`. While `checked` is `false`, it makes the targets in an error scope:

```pseudo
FUNCTION device_make_targets(d : device_t) RETURNS U32
   CALL device_release_targets(d)
   IF NOT d.gpu.checked THEN
      CALL wgpuDevicePushErrorScope(d.gpu.device, WGPUErrorFilter_Validation)
   ENDIF
   IF d.gpu.samples > 1 THEN
      CALL make_target(d, COLOR, d.gpu.format, d.gpu.color)
   ENDIF
   IF d.gpu.depth_format <> 0 THEN
      CALL make_target(d, DEPTH, d.gpu.depth_format, d.gpu.depth)
   ENDIF
   IF d.gpu.checked THEN
      RETURN DEVICE_READY
   ENDIF
   d.gpu.waiting <- TRUE
   CALL wgpuDevicePopErrorScope(d.gpu.device, take_scope)
   RETURN DEVICE_MAKING
ENDFUNCTION
```

`take_scope` gets the answer in the mode `WGPUCallbackMode_AllowProcessEvents`, as a request does. It sets `checked`, and clears `waiting`.
After an error, it writes the line of `samples`, and keeps 4 samples. The first configure of the surface then makes the targets at the size of the window.
`device_choose_targets` and `take_scope` write the line of a setting with `device_say_fallback`: `docs/intern/device_lines.c.md`.
`take_scope` keeps the rules of `docs/intern/device_inside.h.md`, *Callbacks*.

## Functions

| Function | Result |
| --- | --- |
| `device_choose_targets` | Keeps the samples and the depth format, and gives the features that they need. |
| `device_make_targets` | Makes the targets again. Gives `DEVICE_MAKING` while the device checks the samples, else `DEVICE_READY`. |
| `make_target` | Makes the texture of a target and its view. |
| `device_release_targets` | Releases each texture and each view of the targets that is not 0. |
| `take_scope` | Takes the answer of the check, and keeps 4 samples after an error. |
