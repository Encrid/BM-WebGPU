# src/device_inside.h

This private header is shared by the sources of the module.
It includes `include/webgpu_device.h`, the headers `webgpu.h` and `wgpu.h` of BL-WGPU-Native, and the headers of BB-Native.

## The slots

Bridge numbers the types of a module from 0, in the order of their declarations.
`bridge_init` declares the 10 enums of `docs/extern/webgpu_device.h.md`, *The C types*, in the order of that table.
Then it declares `webgpu:webgpu_surface_t` and `webgpu:webgpu_frame_state_t` of the contract, and the 3 singletons.
The header gives the slots of the singletons as `constexpr`: `SETTINGS` is 12, `WINDOW` is 13 and `FRAME` is 14.

## Types

`device_t` is the state of the module:

| Field | Content |
| --- | --- |
| `api` | The API of the entry that runs. `bridge_frame` and `bridge_shutdown` write it first, so that a callback can write a line. |
| `gpu` | A `gpu_t`: the objects of WebGPU, the modes of the frame and the steps of the make. `device_release` sets it to 0. |
| `room` | A `room_t`: the room of the error texts, from the first request of a device to `bridge_shutdown`. |

`gpu_t` holds these fields:

| Field | Content |
| --- | --- |
| `instance`, `surface`, `adapter`, `device`, `queue` | The objects of WebGPU, or 0. |
| `frame`, `color`, `depth` | The texture of the frame of the round, and the two targets. A `target_t` holds a texture and its view, or 0 and 0. |
| `format`, `present`, `alpha` | The modes of the surface, as values of `webgpu.h`. |
| `samples`, `checked` | The samples of the targets, and `true` when the device gives them. |
| `depth_format` | The `WGPUTextureFormat` of the depth texture, or 0. |
| `width`, `height` | The size of the targets and of the configured surface. |
| `max_side` | `maxTextureDimension2D` of the limits of the device. |
| `next` | The next step of the make: `NEXT_ADAPTER` for step 1, `NEXT_DEVICE` for step 2, and `NEXT_KEEP` after them. `NEXT_ADAPTER` is 0, so a `gpu` of zeros starts at step 1. |
| `waiting` | `true` from a request to its callback. |
| `failed` | `true` when a step of the make or a callback failed and wrote its line. |
| `configure` | `true` when the surface needs a configure at the next run. |

`docs/intern/device_errors.c.md`, *The room*, gives the fields of `room_t`.
`docs/extern/webgpu_device.h.md`, *The make*, gives the steps that `next` names.

A step of `frame` gives a value of `device_step_t`. `bridge_frame` turns it into the state of `webgpu:Frame`, the result of the system, and the fields that `write_frame` writes:

| Value | State | Result | Fields | The run |
| --- | --- | --- | --- | --- |
| `DEVICE_READY` | `WEBGPU_FRAME_READY` | 0 | Each field. | It has a frame. |
| `DEVICE_MAKING` | `WEBGPU_FRAME_SKIPPED` | 0 | Only `state`. | It took a step of the make. |
| `DEVICE_FAILED` | `WEBGPU_FRAME_SKIPPED` | 1 | Only `state`. | A step of the make failed. |
| `DEVICE_WAITING` | `WEBGPU_FRAME_SKIPPED` | 0 | Each field but the views. | The size or the status gives no frame. |
| `DEVICE_NO_FRAME` | `WEBGPU_FRAME_SKIPPED` | 1 | Each field but the views. | The surface gave an error, or it cannot be made again. |
| `DEVICE_GONE` | `WEBGPU_FRAME_NO_DEVICE` | 0 | Only `state`. | The window went, or the device was lost. |

## Callbacks

wgpu-native calls five functions of the module: `take_adapter`, `take_device`, `take_scope`, `device_keep_error` and `device_keep_lost`.
Each one has the five parameters of its type in `webgpu.h`. A `[[style.exception]]` excuses each one from the rule F5, with that reason.
No callback releases the device that it gets, because that device belongs to wgpu-native.

No callback calls a function of `webgpu.h`. The answer of a request can come inside a call of `webgpu.h`: https://github.com/webgpu-native/webgpu-headers/blob/673658bc2bd70ec39fc55ebe6bb0173cf6d0a603/webgpu.h#L476.
wgpu-native runs the three other callbacks while it holds the lock of the errors of the device.
https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L494-L643 and https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L2709-L2753 show it.

## Functions

The module writes its lines with `bridge_native_say` of BB-Native, at the level `BRIDGE_LOG_ERR` or `BRIDGE_LOG_WARN`.
The steps write the three families of lines through `src/device_lines.c`, and `device_drain_room` writes the lines of the room.
`[test] writers` names `bridge_native_say`, so that the check of the messages reads each format: `docs/contributing/tests.md`, *The messages of the code*.
The header gives no function of its own. It declares the functions that the sources share:

| Function | Source |
| --- | --- |
| `device_get_state` | `src/webgpu_device.c` |
| `device_make_next`, `device_make_surface`, `device_release` | `src/device_make.c` |
| `device_ask_adapter`, `device_ask_device` | `src/device_ask.c` |
| `device_choose_modes` | `src/device_modes.c` |
| `device_choose_targets`, `device_make_targets`, `device_release_targets` | `src/device_targets.c` |
| `device_take_frame`, `device_release_frame`, `device_is_lost` | `src/device_present.c` |
| `device_keep_error`, `device_keep_lost`, `device_drain_room`, `device_size_room` | `src/device_errors.c` |
| `device_say_no_device`, `device_say_no_surface`, `device_say_fallback` | `src/device_lines.c` |
