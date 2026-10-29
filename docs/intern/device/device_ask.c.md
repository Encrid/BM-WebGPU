# src/device_ask.c

This file asks wgpu-native for the adapter and the device, and takes the two answers in callbacks.

## The adapter

`device_ask_adapter` makes the surface with `device_make_surface`. When it cannot, it sets `failed`, and asks for no adapter.
Else it calls `wgpuInstanceRequestAdapter` with these options:

| Option | Value |
| --- | --- |
| `featureLevel` | `feature_level`, which has the values of `webgpu.h`. |
| `powerPreference` | `power`, which has the values of `webgpu.h`. |
| `forceFallbackAdapter` | `fallback`. |
| `backendType` | The `WGPUBackendType` of `backend`, from the table `BACKENDS`. |
| `compatibleSurface` | The surface, so that the adapter can present to it. |

`BACKENDS` gives `WGPUBackendType_Undefined` for `WEBGPU_BACKEND_ANY`. For each other constant, it gives the `WGPUBackendType` of the same API: `D3D12`, `Metal`, `Vulkan`, `OpenGL` or `OpenGLES`.
The table holds no other value, because wgpu-native stops the process for a value that it does not know: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L2871-L2891.
`take_adapter` keeps the adapter, and sets `next` to `NEXT_DEVICE`. When the request fails, it writes `no device` with the reason `no adapter` and the message of wgpu-native, and sets `failed`.

## The device

`device_ask_device` does these steps:

1. It chooses the modes of the surface: `docs/intern/device_modes.c.md`. If it cannot, it sets `failed`.
2. It gives the room `error_room` bytes with `device_size_room`.
3. It gets the features of `device_choose_targets`, the limits of the adapter, and with `WEBGPU_FEATURES_ADAPTER` the features of the adapter.
4. `describe_device` copies `DEVICE_SHAPE` into the descriptor, then adds the features, the limits and the state, as the next table tells.
5. It calls `wgpuAdapterRequestDevice`. Then it frees the features of the adapter, if it got them.

`DEVICE_SHAPE` holds the two labels and the two callbacks of the next table. `device_request_t` holds the descriptor and the lists that it points to.

| Field of `WGPUDeviceDescriptor` | Value |
| --- | --- |
| `label` | `webgpu:device` |
| `requiredFeatures` | With `WEBGPU_FEATURES_ADAPTER`, each feature of `wgpuAdapterGetFeatures` but `WGPUNativeFeature_RayQuery` and `WGPUNativeFeature_CooperativeMatrix`. Else the features of `device_choose_targets`: `docs/intern/device_targets.c.md`. |
| `requiredLimits` | With `WEBGPU_LIMITS_ADAPTER`, the limits of `wgpuAdapterGetLimits`. Else none. |
| `defaultQueue` | The label `webgpu:device queue`. |
| `deviceLostCallbackInfo` | `device_keep_lost`, with the mode `WGPUCallbackMode_AllowSpontaneous`, and the state in `userdata1`. |
| `uncapturedErrorCallbackInfo` | `device_keep_error`, with the state in `userdata1`. |

The two features that the table leaves out are the experimental features that wgpu-native v29.0.1.1 knows: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L1330-L1332 and https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L1424-L1426.
wgpu-native gives each device `ExperimentalFeatures::disabled()`, and wgpu-core then refuses a device that asks for an experimental feature.
https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L497 and https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-core/src/instance.rs#L857-L866 show it.

With no limits, wgpu-native gives the limits of `docs/extern/webgpu_device.h.md`, *webgpu:webgpu_limits_t*: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/utils.rs#L52-L92.
The default callbacks of wgpu-native stop the process at the first error or loss: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L413-L443.
So the module always gives its two callbacks of the device: `docs/intern/device_errors.c.md`.
`take_device` keeps the device, and sets `next` to `NEXT_KEEP`. When the request fails, it writes `no device` with the reason `the adapter refuses the device` and the message of wgpu-native, and sets `failed`.
Both callbacks write the line with `device_say_no_device`: `docs/intern/device_lines.c.md`.

## The callbacks of the requests

`take_adapter` and `take_device` come on the thread of `frame`, so they write their line with `api` of the state. Each one clears `waiting`.
They keep the rules of `docs/intern/device_inside.h.md`, *Callbacks*.
The leak count reads argument 2 of `take_adapter` and of `take_device` as a take, of the kinds `webgpu_adapter` and `webgpu_device`.

## Functions

| Function | Result |
| --- | --- |
| `device_ask_adapter` | Makes the surface, then asks for the adapter. Gives `DEVICE_MAKING`. |
| `take_adapter` | Keeps the adapter, or writes its line and sets `failed`. |
| `device_ask_device` | Chooses the modes, then asks for the device. Gives `DEVICE_MAKING`. |
| `describe_device` | Copies `DEVICE_SHAPE`, then fills the features, the limits and the state of the descriptor of the device. |
| `take_device` | Keeps the device, or writes its line and sets `failed`. |
