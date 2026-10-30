# include/webgpu_device.h

`include/webgpu_device.h` is the header of the module `webgpu:device@1.0.0`, the WebGPU device of Bridge.
The module makes the device and the surface of one window, and gives the frame of each round to the render modules.
It covers the instance, the adapter, the device and the surface of `webgpu.h`. It draws nothing: each render module draws its own passes.

## The module

| Word of the manifest | Value |
| --- | --- |
| The identity | `webgpu:device@1.0.0` |
| `abi` | `2` |
| `config` | `webgpu:DeviceSettings` |
| `settings` | `present format alpha power fallback backend feature_level samples depth limits features error_room instance_backends instance_flags d3d12_compiler d3d12_swapchain gles_version gl_fence creation_threshold loss_threshold dxc_path` |
| `pinned` | `true`, because on macOS the module must make and configure the surface on the thread of the window. |
| `reloadable` | `false`, because the objects of each render module belong to its device. |

The manifest also gives a default for each of these keys of the module. A deployment that states one of them replaces its default.

| Key | Default | Reason |
| --- | --- | --- |
| `scratch` | `16384` | The declarations of the 21 types of `init`, and the value of `webgpu:Frame` that it attaches. At a run of `frame`, the texts of its lines: the texts of the room of *The errors*, and the head of each line. With a larger `error_room`, make `scratch` larger too, or a line is cut to the room that is left. |
| `log_ring` | `2048` | Room for 64 records of 32 bytes: the errors of the device in one round. |
| `spans` | `4096` | A cell for each of the 21 types in each table, and the spans of the two systems. |
| `spawns` | `"none"` | The module makes no entity. |
| `changes` | `88` | The attach of `webgpu:Frame` in `init`: 24 bytes, and its value of 64 bytes. |
| `persistent` | `"none"` | The module keeps no data in the arena. |
| `hot_reload` | `false` | The module cannot reload. |

The header `bridge.h` of Bridge tells the room that `spans` needs in a world with many tables.

The file of the module is `webgpu_device`, with the suffix of a library. It runs on the backend of BB-Native, and loads the wgpu-native library of BL-WGPU-Native, which sits beside it.
It needs no SDL3: the app gives it the window through `webgpu:Window`.

## The contract

The module reads `webgpu:Window`, and writes `webgpu:Frame` for the render modules.
These singletons and their enums `webgpu:webgpu_surface_t` and `webgpu:webgpu_frame_state_t` are the contract of BL-WebGPU-Contract.
The document of `webgpu_contract.h` of that library tells their fields, their constants and the roles of the modules.

The module makes no surface for a kind of window of another system, for example `WEBGPU_SURFACE_METAL` on Windows: *Messages*.
It makes none for a value that names no kind, or for a handle of 0.
While *The make* takes its steps, each field of `webgpu:Frame` but `state` is 0.

## The systems

| System | Phase | What it does |
| --- | --- | --- |
| `frame` | `frame` | Gets the frame of the round, and writes `webgpu:Frame`. *The frame* tells its steps. |
| `present` | `present` | If the state of `webgpu:Frame` is `WEBGPU_FRAME_READY`, it presents the frame. |

The settings come after `init`, so the device comes at the first run of `frame` with a window.
The full names of the systems for `[order]` are `webgpu:device:frame` and `webgpu:device:present`.

## The frame

`frame` does these steps at each run:

1. It writes each text that waits in the room of *The errors*.
2. If no `webgpu:Window` is there, or its `surface` is `WEBGPU_SURFACE_NONE`, it releases the surface and the device.
   Then it writes the state `WEBGPU_FRAME_NO_DEVICE`, and stops.
3. If *The make* is not done, it takes its next step, writes the state `WEBGPU_FRAME_SKIPPED`, and stops.
4. It checks that the device is not lost: *A lost device*.
5. If the size of `webgpu:Window` changed, it configures the surface for the new size.
6. It gets the texture of the frame from the surface, and acts on its status: *The status of a frame*.
7. It writes `webgpu:Frame` with the state `WEBGPU_FRAME_READY`.

If a step gets no device or no surface, it writes its line of *Messages* at the level 3. Then the system gives 1.
The system tries again at its next run, until `max_system_failures` stops the module.

The module configures the surface only for a size from 1 to `maxTextureDimension2D` of the device on each side.
A window of another size, for example a minimized window, gives the state `WEBGPU_FRAME_SKIPPED`.
A change of a setting after the device is there acts at the next device, for example after a loss.

## The make

The module makes its objects one step at a time, one step at each run of `frame`. Thus a round never waits for an answer.

1. It makes the instance of *The instance*, and the surface of the window. Then it asks for an adapter.
2. It asks the adapter for a device.
3. It takes the queue and the limits of the device.
4. With 2, 8 or 16 samples, it checks that the device gives that count: *webgpu:webgpu_samples_t*.

Each run calls `wgpuInstanceProcessEvents` once, so that an answer can come before the step that needs it.
With wgpu-native v29.0.1.1, each request answers inside its call. Thus each step takes one run.

If the instance has no adapter, step 1 makes no surface, and asks for an adapter with no surface.
That request gives `no device: no adapter`, and the text of wgpu-native tells the cause of each backend.
The module makes no surface then, because wgpu-native v29.0.1.1 stops the process when no backend can make it.
https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L2805-L2810 and https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-core/src/instance.rs#L276-L281 show it.

## The instance

Step 1 of *The make* makes the instance with `WGPUInstanceExtras` of `wgpu.h`. The extras hold 9 settings of *webgpu:DeviceSettings*:
`instance_backends`, `instance_flags`, `d3d12_compiler`, `d3d12_swapchain`, `gles_version`, `gl_fence`, `creation_threshold`, `loss_threshold` and `dxc_path`.
With their defaults, wgpu-native makes the instance that it makes with no extras.
https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L371-L423 and https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-types/src/instance.rs#L67-L75 show it.
On Wayland, the extras also hold the display of the window.

A setting of a backend that the system does not have changes nothing. For example, `d3d12_compiler` changes nothing on Linux and macOS.
On each system, the module still reads each setting and gives it to wgpu-native, and Bridge still checks its value: *webgpu:DeviceSettings*.

wgpu-native tries to start each backend of `instance_backends` that its release has for the system. A backend that does not start gives no adapter.
For example, D3D12 does not start with `WEBGPU_D3D12_COMPILER_DXC` and a `dxc_path` that names no DXC library.
Then the adapter comes from another backend, or the request gives `no device: no adapter`.
The text of wgpu-native then names the cause, for example `dx12 drivers/libraries could not be loaded`.

## The status of a frame

| Status of `webgpu.h` | What `frame` does |
| --- | --- |
| `SuccessOptimal` | It gives the frame. |
| `SuccessSuboptimal` | It gives the frame, and configures the surface again at its next run. |
| `Timeout`, `Outdated` | It writes the state `WEBGPU_FRAME_SKIPPED`, and configures the surface again. |
| `Lost` | It writes the state `WEBGPU_FRAME_SKIPPED`, and makes the surface again at its next run. |
| `Occluded` of `wgpu.h` | It writes the state `WEBGPU_FRAME_SKIPPED`. The surface stays good while the window does not show. Only Metal gives this status. |
| `Error` | It writes `no surface: the surface gives an error` at the level 3, and the system gives 1. |

`present` does not read the status of `wgpuSurfacePresent`, because wgpu-native gives the cause of a failed present only to its own log.

## A lost device

When the device is lost, the module writes `device lost: <reason>: <text>` at the level 3.
`<reason>` names the code of `webgpu.h`: `unknown`, `destroyed`, `callback cancelled` or `failed creation`. `<text>` is the text of wgpu-native.
wgpu-native v29.0.1.1 gives `destroyed` for each loss, with the text `Device lost`.
In that round, the state of `webgpu:Frame` is `WEBGPU_FRAME_NO_DEVICE`. At the next run of `frame`, the module starts *The make* again, with the settings of that run.

wgpu-native v29.0.1.1 tells the module of a loss only inside a later call on the device. A loss alone, or `wgpuDeviceDestroy`, calls nothing.
Thus, before the configure and the texture of step 5 and step 6 of *The frame*, `frame` makes a command encoder and releases it.
The label of the encoder is `webgpu:device check`. On a lost device, that call tells the module of the loss, and `frame` acts on it in the same run.

## The errors

The module writes each error of the device that no scope catches: `webgpu error: <type>: <text>`, at the level 3.
`<type>` names the code of `webgpu.h`: `validation`, `out of memory`, `internal` or `unknown`. wgpu-native v29.0.1.1 gives only `validation` and `out of memory`.
A render module can catch its own errors with a scope of `webgpu.h`.

Each object of the module that takes a label has a label that starts with `webgpu:device`.
Each render module gives its objects a label that starts with its full name. Thus the text of an error tells which module made the object.

wgpu-native calls the callback of an error inside the call that finds it, which can be a call of a render module, on its thread.
That callback cannot write a line of this module. Thus it copies the text into a room of `error_room` bytes. The callback of a loss copies its text there too.
The next run of `frame` writes each text that waits. Then it writes the count of the texts that did not fit, if there are some.

## Limits

With wgpu-native v29.0.1.1, a lost device stops the process at these calls:

- `wgpuSurfaceConfigure` and `wgpuSurfaceGetCurrentTexture`. Thus `frame` checks the device before them: *A lost device*.
- The release of the texture of a frame that the surface did not present. Thus a loss from `frame` to `present` stops the process, because the module holds that texture then.
- `wgpuQueueSubmit`. Thus a render module that submits after a loss stops the process.

## webgpu:DeviceSettings

`webgpu:DeviceSettings` is the singleton of the settings. Bridge writes it from the deployment, or from the defaults of the manifest.

| Field | C type | Value |
| --- | --- | --- |
| `present` | `webgpu_present_t` | How the surface presents a frame. The default is `WEBGPU_PRESENT_FIFO`. |
| `format` | `webgpu_format_t` | The format of the frame. The default is `WEBGPU_FORMAT_PREFERRED`. |
| `alpha` | `webgpu_alpha_t` | How the alpha of the frame mixes with the screen. The default is `WEBGPU_ALPHA_AUTO`. |
| `power` | `webgpu_power_t` | The adapter to prefer. The default is `WEBGPU_POWER_HIGH_PERFORMANCE`. |
| `fallback` | `bool` | `true`: only a fallback adapter, for example a software one. The default is `false`. |
| `backend` | `webgpu_backend_t` | The API under WebGPU. The default is `WEBGPU_BACKEND_ANY`. |
| `feature_level` | `webgpu_feature_level_t` | The level of WebGPU. The default is `WEBGPU_FEATURE_LEVEL_CORE`. wgpu-native v29.0.1.1 does not read it, so it does not change the adapter. |
| `samples` | `webgpu_samples_t` | The samples of each pixel of the frame. The default is `WEBGPU_SAMPLES_1`. |
| `depth` | `webgpu_depth_t` | The depth texture of the frame. The default is `WEBGPU_DEPTH_24_PLUS_STENCIL_8`. |
| `limits` | `webgpu_limits_t` | The limits that the device asks for. The default is `WEBGPU_LIMITS_ADAPTER`. |
| `features` | `webgpu_features_t` | The features that the device asks for. The default is `WEBGPU_FEATURES_ADAPTER`. |
| `error_room` | `uint32_t` | The bytes of the room where the texts of the errors wait for `frame`: *The errors*. The default is 8192. |
| `instance_backends` | `webgpu_instance_backends_t` | The backends that the instance starts. The default is `WEBGPU_INSTANCE_BACKEND_ALL`. |
| `instance_flags` | `webgpu_instance_flags_t` | The flags of debug and of validation of the instance. The default is `WEBGPU_INSTANCE_FLAG_DEFAULT`. |
| `d3d12_compiler` | `webgpu_d3d12_compiler_t` | The compiler of the shaders of D3D12. The default is `WEBGPU_D3D12_COMPILER_AUTO`. |
| `d3d12_swapchain` | `webgpu_d3d12_swapchain_t` | The swapchain of a surface of D3D12. The default is `WEBGPU_D3D12_SWAPCHAIN_HWND`. |
| `gles_version` | `webgpu_gles_version_t` | The version of OpenGL ES that the GL backend asks for. The default is `WEBGPU_GLES_VERSION_AUTO`. |
| `gl_fence` | `webgpu_gl_fence_t` | How the fences of the GL backend tell that the GPU did its work. The default is `WEBGPU_GL_FENCE_NORMAL`. |
| `creation_threshold` | `uint8_t` | The percent of the memory budget from which a creation gives an error of memory: *The thresholds*. The default is 0. |
| `loss_threshold` | `uint8_t` | The percent of the memory budget from which the device is lost: *The thresholds*. The default is 0. |
| `dxc_path` | `char[1024]` | The path of the library that `WEBGPU_D3D12_COMPILER_DXC` loads. A deployment gives at most 1023 bytes and the zero after them. All 1024 bytes count when a system writes no zero. The default is `dxcompiler.dll`. |

If `fallback` is `true`, or `backend` is not `WEBGPU_BACKEND_ANY`, a system with no such adapter gives none: *Messages*.
If the surface or the device does not give the value of a setting, the module uses another value. It writes a line at the level 4.

Each enum of the settings is of `uint32_t`. A deployment states a constant by its name or by its value.
Each bitmask of the settings is of `uint32_t` too. A deployment states it by its value: the sum of the values of its constants.
A deployment can state `dxc_path` as `{ path = "<path>" }`: a path from the folder of the file, as the readme of Bridge-Host tells.

Bridge refuses a value that is not valid, and then the module does not load. The record of Bridge names the setting, for example:
`did not load: its setting 'd3d12_compiler' is 4, which is no constant of the enum of the field it names`.
Bridge refuses a value that is no constant of an enum, and a bit that no constant of a bitmask has.
It also refuses a number that the field cannot hold, and a text of 1024 bytes or more.
The header `bridge.h` of Bridge tells each of these reasons.

### The thresholds

`creation_threshold` and `loss_threshold` are percents of the memory budget that the system gives to the adapter.
Only D3D12, and Vulkan with `VK_EXT_memory_budget`, read them.
The value 0 sets no threshold. The values from 1 to 255 set the threshold at that percent.
Over `creation_threshold`, the creation of a buffer, a texture, a query set or an acceleration structure gives an error `out of memory`: *The errors*.
Over `loss_threshold`, wgpu-native loses the device after a submit or a poll: *A lost device*.

wgpu-native v29.0.1.1 gives `creation_threshold` to both thresholds, and does not read `loss_threshold`:
https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L393-L394.
Thus with v29.0.1.1, a device is also lost over `creation_threshold`, and `loss_threshold` changes nothing.

### webgpu:webgpu_present_t

The values are those of `webgpu.h`.

| Constant | Value | The present |
| --- | --- | --- |
| `WEBGPU_PRESENT_FIFO` | 1 | Waits for the screen, and never tears. Each surface gives it. |
| `WEBGPU_PRESENT_FIFO_RELAXED` | 2 | Waits for the screen, but presents a late frame at once, so it can tear. |
| `WEBGPU_PRESENT_IMMEDIATE` | 3 | Does not wait, and can tear. |
| `WEBGPU_PRESENT_MAILBOX` | 4 | Does not tear, and does not hold the rounds to the rate of the screen. |

If the surface does not give the mode, it presents with `WEBGPU_PRESENT_FIFO`.

### webgpu:webgpu_format_t

| Constant | Value | The format of the frame |
| --- | --- | --- |
| `WEBGPU_FORMAT_PREFERRED` | 1 | The first format that the surface gives. |
| `WEBGPU_FORMAT_BGRA8_UNORM` | 2 | `BGRA8Unorm` |
| `WEBGPU_FORMAT_BGRA8_UNORM_SRGB` | 3 | `BGRA8UnormSrgb` |
| `WEBGPU_FORMAT_RGBA8_UNORM` | 4 | `RGBA8Unorm` |
| `WEBGPU_FORMAT_RGBA8_UNORM_SRGB` | 5 | `RGBA8UnormSrgb` |
| `WEBGPU_FORMAT_RGB10A2_UNORM` | 6 | `RGB10A2Unorm` |
| `WEBGPU_FORMAT_RGBA16_FLOAT` | 7 | `RGBA16Float` |

If the surface does not give the format, the frame uses `WEBGPU_FORMAT_PREFERRED`.

### webgpu:webgpu_alpha_t

| Constant | Value | The alpha of the frame |
| --- | --- | --- |
| `WEBGPU_ALPHA_AUTO` | 1 | wgpu-native chooses between opaque and inherit. |
| `WEBGPU_ALPHA_OPAQUE` | 2 | The screen does not use the alpha. |
| `WEBGPU_ALPHA_PREMULTIPLIED` | 3 | The colors are already multiplied by their alpha. |
| `WEBGPU_ALPHA_UNPREMULTIPLIED` | 4 | The screen multiplies the colors by their alpha. |
| `WEBGPU_ALPHA_INHERIT` | 5 | The system of the window decides. |

If the surface does not give the mode, it uses `WEBGPU_ALPHA_AUTO`.

### webgpu:webgpu_power_t

The values are those of `webgpu.h`.

| Constant | Value | The adapter |
| --- | --- | --- |
| `WEBGPU_POWER_LOW_POWER` | 1 | An adapter that uses less power, for example an integrated GPU. |
| `WEBGPU_POWER_HIGH_PERFORMANCE` | 2 | The fastest adapter, for example a discrete GPU. |

### webgpu:webgpu_backend_t

| Constant | Value | The API under WebGPU |
| --- | --- | --- |
| `WEBGPU_BACKEND_ANY` | 1 | wgpu-native chooses the API of the system. |
| `WEBGPU_BACKEND_D3D12` | 2 | Direct3D 12. |
| `WEBGPU_BACKEND_METAL` | 3 | Metal. |
| `WEBGPU_BACKEND_VULKAN` | 4 | Vulkan. |
| `WEBGPU_BACKEND_OPENGL` | 5 | OpenGL. |
| `WEBGPU_BACKEND_OPENGLES` | 6 | OpenGL ES. |

wgpu-native v29.0.1.1 gives its one GL backend for `WEBGPU_BACKEND_OPENGL` and for `WEBGPU_BACKEND_OPENGLES`. It has no backend of Direct3D 11.

### webgpu:webgpu_feature_level_t

The values are those of `webgpu.h`.

| Constant | Value | The level |
| --- | --- | --- |
| `WEBGPU_FEATURE_LEVEL_COMPATIBILITY` | 1 | The level that OpenGL ES 3.1 and Direct3D 11 can give. |
| `WEBGPU_FEATURE_LEVEL_CORE` | 2 | The level that Vulkan, Metal and Direct3D 12 give. |

### webgpu:webgpu_samples_t

The value of each constant is its count of samples.

| Constant | Value | The samples |
| --- | --- | --- |
| `WEBGPU_SAMPLES_1` | 1 | No multisampling. |
| `WEBGPU_SAMPLES_2` | 2 | 2 samples. |
| `WEBGPU_SAMPLES_4` | 4 | 4 samples. Each WebGPU device gives them. |
| `WEBGPU_SAMPLES_8` | 8 | 8 samples. |
| `WEBGPU_SAMPLES_16` | 16 | 16 samples. |

For 2, 8 and 16 samples, the device asks for the feature `TextureAdapterSpecificFormatFeatures` of `wgpu.h`.
If the device does not give the count for the format of the frame, the frame uses `WEBGPU_SAMPLES_4`.
With more than 1 sample, the module makes a multisample texture of the size of the frame.

### webgpu:webgpu_depth_t

| Constant | Value | The depth texture |
| --- | --- | --- |
| `WEBGPU_DEPTH_NONE` | 1 | No depth texture. |
| `WEBGPU_DEPTH_16_UNORM` | 2 | `Depth16Unorm` |
| `WEBGPU_DEPTH_24_PLUS` | 3 | `Depth24Plus` |
| `WEBGPU_DEPTH_24_PLUS_STENCIL_8` | 4 | `Depth24PlusStencil8` |
| `WEBGPU_DEPTH_32_FLOAT` | 5 | `Depth32Float`. A copy to a buffer can read it, but not `Depth24Plus`. |
| `WEBGPU_DEPTH_32_FLOAT_STENCIL_8` | 6 | `Depth32FloatStencil8`. The device asks for its feature. |

If the device does not give `Depth32FloatStencil8`, the depth uses `WEBGPU_DEPTH_24_PLUS_STENCIL_8`.
The depth texture has the samples of the frame. Each render pass chooses to use it or not, and a module can make depth textures of its own.

### webgpu:webgpu_limits_t

| Constant | Value | The limits of the device |
| --- | --- | --- |
| `WEBGPU_LIMITS_ADAPTER` | 1 | Each limit that the adapter gives. |
| `WEBGPU_LIMITS_DEFAULT` | 2 | The minimum limits of WebGPU, which each device gives. A world that runs with them fits each WebGPU device. On an adapter that cannot give them, wgpu-native gives lower limits. |

### webgpu:webgpu_features_t

| Constant | Value | The features of the device |
| --- | --- | --- |
| `WEBGPU_FEATURES_ADAPTER` | 1 | Each feature that the adapter gives, but the experimental features. |
| `WEBGPU_FEATURES_DEFAULT` | 2 | Only the features that `samples` and `depth` need. |

An adapter can give the experimental features `RayQuery` and `CooperativeMatrix` of `wgpu.h`.
wgpu-native v29.0.1.1 refuses a device that asks for one of them, because it does not enable the experimental features. Thus the module does not ask for them.

### webgpu:webgpu_instance_backends_t

`webgpu_instance_backends_t` is a bitmask. The values are those of `WGPUInstanceBackend` of `wgpu.h`.

| Constant | Value | The backend |
| --- | --- | --- |
| `WEBGPU_INSTANCE_BACKEND_ALL` | 0 | Each backend of wgpu-native. |
| `WEBGPU_INSTANCE_BACKEND_VULKAN` | 1 | Vulkan. |
| `WEBGPU_INSTANCE_BACKEND_GL` | 2 | OpenGL and OpenGL ES. |
| `WEBGPU_INSTANCE_BACKEND_METAL` | 4 | Metal. |
| `WEBGPU_INSTANCE_BACKEND_D3D12` | 8 | Direct3D 12. |

`backend` chooses the backend of the adapter among the backends that the instance starts.
The bit `WGPUInstanceBackend_BrowserWebGPU` of `wgpu.h` is no constant, because only a page in a browser has that backend.

### webgpu:webgpu_instance_flags_t

`webgpu_instance_flags_t` is a bitmask. The values are those of `WGPUInstanceFlag` of `wgpu.h`.

| Constant | Value | The flag |
| --- | --- | --- |
| `WEBGPU_INSTANCE_FLAG_EMPTY` | 0 | No flag. |
| `WEBGPU_INSTANCE_FLAG_DEBUG` | 1 | Debug data in the shaders and in the objects. |
| `WEBGPU_INSTANCE_FLAG_VALIDATION` | 2 | The validation layer of the API under WebGPU, if the system has it. |
| `WEBGPU_INSTANCE_FLAG_DISCARD_HAL_LABELS` | 4 | wgpu-native gives no label to the API under WebGPU. |
| `WEBGPU_INSTANCE_FLAG_ALLOW_UNDERLYING_NONCOMPLIANT_ADAPTER` | 8 | Also an adapter of a Vulkan driver that is not conformant. |
| `WEBGPU_INSTANCE_FLAG_GPU_BASED_VALIDATION` | 16 | The validation on the GPU, with D3D12 and Vulkan. It includes the validation layer. |
| `WEBGPU_INSTANCE_FLAG_VALIDATION_INDIRECT_CALL` | 32 | The check of the arguments of each indirect draw and dispatch. Without it, wrong arguments have no defined result. |
| `WEBGPU_INSTANCE_FLAG_AUTOMATIC_TIMESTAMP_NORMALIZATION` | 64 | The timestamps of a query set are in nanoseconds, and the timestamp period of the queue is 1. |
| `WEBGPU_INSTANCE_FLAG_DEFAULT` | 16777216 | The flags of the build of wgpu-native. The release of wgpu-native sets only `WEBGPU_INSTANCE_FLAG_VALIDATION_INDIRECT_CALL`. |
| `WEBGPU_INSTANCE_FLAG_DEBUGGING` | 33554432 | The debug data, the validation layer and the check of the indirect calls. |
| `WEBGPU_INSTANCE_FLAG_ADVANCED_DEBUGGING` | 67108864 | `WEBGPU_INSTANCE_FLAG_DEBUGGING` and the validation on the GPU. |
| `WEBGPU_INSTANCE_FLAG_WITH_ENV` | 134217728 | wgpu-native also sets each flag whose variable of the environment has a value that is not 0, for example `WGPU_VALIDATION`. |

https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/conv.rs#L318-L359 and https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-types/src/instance.rs#L234-L319 show what each flag sets.

### webgpu:webgpu_d3d12_compiler_t

| Constant | Value | The compiler of the shaders of D3D12 |
| --- | --- | --- |
| `WEBGPU_D3D12_COMPILER_AUTO` | 1 | DXC from `dxcompiler.dll` in the search path of the libraries of Windows. If no such library is there, FXC. |
| `WEBGPU_D3D12_COMPILER_FXC` | 2 | FXC, the library `d3dcompiler_47.dll` of Windows. Its shader model is 5.1, so D3D12 gives no `RayQuery`. |
| `WEBGPU_D3D12_COMPILER_DXC` | 3 | DXC from the library at `dxc_path`. If wgpu-native cannot load it, D3D12 does not start: *The instance*. |

The value of each constant is 1 more than its value of `WGPUDx12Compiler` of `wgpu.h`.
With DXC, the shader model of D3D12 is the lower one of the device and of the DXC library.
If a DXC library loads but does not start, D3D12 does not start either, also with `WEBGPU_D3D12_COMPILER_AUTO`.
https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/dx12/instance.rs#L51-L97 shows the three compilers.

### webgpu:webgpu_d3d12_swapchain_t

The values are those of `WGPUDx12SwapchainKind` of `wgpu.h`.

| Constant | Value | The swapchain of a surface of D3D12 |
| --- | --- | --- |
| `WEBGPU_D3D12_SWAPCHAIN_HWND` | 1 | A swapchain of DXGI on the window. Its surface gives only the alpha mode opaque. |
| `WEBGPU_D3D12_SWAPCHAIN_VISUAL` | 2 | A swapchain of DXGI on a visual of DirectComposition over the window. Its surface gives each alpha mode of `webgpu.h`, also premultiplied. |

https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/dx12/adapter.rs#L1296-L1308 shows the alpha modes of each swapchain.

### webgpu:webgpu_gles_version_t

| Constant | Value | The version of OpenGL ES |
| --- | --- | --- |
| `WEBGPU_GLES_VERSION_AUTO` | 1 | The highest version that the driver gives. |
| `WEBGPU_GLES_VERSION_3_0` | 2 | OpenGL ES 3.0. |
| `WEBGPU_GLES_VERSION_3_1` | 3 | OpenGL ES 3.1. |
| `WEBGPU_GLES_VERSION_3_2` | 4 | OpenGL ES 3.2. |

The value of each constant is 1 more than its value of `WGPUGles3MinorVersion` of `wgpu.h`.
The GL backend reads it only for a context of OpenGL ES of EGL. It makes one when EGL cannot make a context of OpenGL.
If the driver does not give the version, the GL backend does not start.
https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/gles/egl.rs#L467-L479 and https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/gles/egl.rs#L583-L591 show it.

### webgpu:webgpu_gl_fence_t

| Constant | Value | The fences of the GL backend |
| --- | --- | --- |
| `WEBGPU_GL_FENCE_NORMAL` | 1 | A fence tells that the GPU did the work of a submit. |
| `WEBGPU_GL_FENCE_AUTO_FINISH` | 2 | A fence tells it at once. Thus the callback of `wgpuQueueOnSubmittedWorkDone` comes at the next poll of the device. |

The value of each constant is 1 more than its value of `WGPUGLFenceBehaviour` of `wgpu.h`.
https://github.com/gfx-rs/wgpu/blob/v29.0.1/wgpu-hal/src/gles/fence.rs#L37-L45 shows it.

## The C types

`include/webgpu_device.h` gives the C type of each type of the module, and its descriptor for Bridge.
It includes `webgpu_contract.h`, which gives the C types of *The contract*.
The header includes `bridge_schema.h` of Bridge. It makes each C type and its descriptor from one text, with the macros of that header.

| Type of Bridge | C type | Descriptor | Macro of the text |
| --- | --- | --- | --- |
| `webgpu:webgpu_present_t` | `webgpu_present_t` | `webgpu_present_t_desc` | `WEBGPU_PRESENT_VALUES` |
| `webgpu:webgpu_format_t` | `webgpu_format_t` | `webgpu_format_t_desc` | `WEBGPU_FORMAT_VALUES` |
| `webgpu:webgpu_alpha_t` | `webgpu_alpha_t` | `webgpu_alpha_t_desc` | `WEBGPU_ALPHA_VALUES` |
| `webgpu:webgpu_power_t` | `webgpu_power_t` | `webgpu_power_t_desc` | `WEBGPU_POWER_VALUES` |
| `webgpu:webgpu_backend_t` | `webgpu_backend_t` | `webgpu_backend_t_desc` | `WEBGPU_BACKEND_VALUES` |
| `webgpu:webgpu_feature_level_t` | `webgpu_feature_level_t` | `webgpu_feature_level_t_desc` | `WEBGPU_FEATURE_LEVEL_VALUES` |
| `webgpu:webgpu_samples_t` | `webgpu_samples_t` | `webgpu_samples_t_desc` | `WEBGPU_SAMPLES_VALUES` |
| `webgpu:webgpu_depth_t` | `webgpu_depth_t` | `webgpu_depth_t_desc` | `WEBGPU_DEPTH_VALUES` |
| `webgpu:webgpu_limits_t` | `webgpu_limits_t` | `webgpu_limits_t_desc` | `WEBGPU_LIMITS_VALUES` |
| `webgpu:webgpu_features_t` | `webgpu_features_t` | `webgpu_features_t_desc` | `WEBGPU_FEATURES_VALUES` |
| `webgpu:webgpu_instance_backends_t` | `webgpu_instance_backends_t` | `webgpu_instance_backends_t_desc` | `WEBGPU_INSTANCE_BACKENDS_VALUES` |
| `webgpu:webgpu_instance_flags_t` | `webgpu_instance_flags_t` | `webgpu_instance_flags_t_desc` | `WEBGPU_INSTANCE_FLAGS_VALUES` |
| `webgpu:webgpu_d3d12_compiler_t` | `webgpu_d3d12_compiler_t` | `webgpu_d3d12_compiler_t_desc` | `WEBGPU_D3D12_COMPILER_VALUES` |
| `webgpu:webgpu_d3d12_swapchain_t` | `webgpu_d3d12_swapchain_t` | `webgpu_d3d12_swapchain_t_desc` | `WEBGPU_D3D12_SWAPCHAIN_VALUES` |
| `webgpu:webgpu_gles_version_t` | `webgpu_gles_version_t` | `webgpu_gles_version_t_desc` | `WEBGPU_GLES_VERSION_VALUES` |
| `webgpu:webgpu_gl_fence_t` | `webgpu_gl_fence_t` | `webgpu_gl_fence_t_desc` | `WEBGPU_GL_FENCE_VALUES` |
| `webgpu:DeviceSettings` | `webgpu_device_settings_t` | `webgpu_device_settings_t_desc` | `WEBGPU_DEVICE_SETTINGS_FIELDS` |

Each macro of the text gives the braces of its type and the text between them.
A module in another language does not include the header. It states the descriptor of each type with these bytes, in the order of the table:

```text
uint32_t { WEBGPU_PRESENT_FIFO = 1, WEBGPU_PRESENT_FIFO_RELAXED = 2, WEBGPU_PRESENT_IMMEDIATE = 3, WEBGPU_PRESENT_MAILBOX = 4 }
uint32_t { WEBGPU_FORMAT_PREFERRED = 1, WEBGPU_FORMAT_BGRA8_UNORM = 2, WEBGPU_FORMAT_BGRA8_UNORM_SRGB = 3, WEBGPU_FORMAT_RGBA8_UNORM = 4, WEBGPU_FORMAT_RGBA8_UNORM_SRGB = 5, WEBGPU_FORMAT_RGB10A2_UNORM = 6, WEBGPU_FORMAT_RGBA16_FLOAT = 7 }
uint32_t { WEBGPU_ALPHA_AUTO = 1, WEBGPU_ALPHA_OPAQUE = 2, WEBGPU_ALPHA_PREMULTIPLIED = 3, WEBGPU_ALPHA_UNPREMULTIPLIED = 4, WEBGPU_ALPHA_INHERIT = 5 }
uint32_t { WEBGPU_POWER_LOW_POWER = 1, WEBGPU_POWER_HIGH_PERFORMANCE = 2 }
uint32_t { WEBGPU_BACKEND_ANY = 1, WEBGPU_BACKEND_D3D12 = 2, WEBGPU_BACKEND_METAL = 3, WEBGPU_BACKEND_VULKAN = 4, WEBGPU_BACKEND_OPENGL = 5, WEBGPU_BACKEND_OPENGLES = 6 }
uint32_t { WEBGPU_FEATURE_LEVEL_COMPATIBILITY = 1, WEBGPU_FEATURE_LEVEL_CORE = 2 }
uint32_t { WEBGPU_SAMPLES_1 = 1, WEBGPU_SAMPLES_2 = 2, WEBGPU_SAMPLES_4 = 4, WEBGPU_SAMPLES_8 = 8, WEBGPU_SAMPLES_16 = 16 }
uint32_t { WEBGPU_DEPTH_NONE = 1, WEBGPU_DEPTH_16_UNORM = 2, WEBGPU_DEPTH_24_PLUS = 3, WEBGPU_DEPTH_24_PLUS_STENCIL_8 = 4, WEBGPU_DEPTH_32_FLOAT = 5, WEBGPU_DEPTH_32_FLOAT_STENCIL_8 = 6 }
uint32_t { WEBGPU_LIMITS_ADAPTER = 1, WEBGPU_LIMITS_DEFAULT = 2 }
uint32_t { WEBGPU_FEATURES_ADAPTER = 1, WEBGPU_FEATURES_DEFAULT = 2 }
uint32_t { WEBGPU_INSTANCE_BACKEND_ALL = 0, WEBGPU_INSTANCE_BACKEND_VULKAN = 1, WEBGPU_INSTANCE_BACKEND_GL = 2, WEBGPU_INSTANCE_BACKEND_METAL = 4, WEBGPU_INSTANCE_BACKEND_D3D12 = 8 }
uint32_t { WEBGPU_INSTANCE_FLAG_EMPTY = 0, WEBGPU_INSTANCE_FLAG_DEBUG = 1, WEBGPU_INSTANCE_FLAG_VALIDATION = 2, WEBGPU_INSTANCE_FLAG_DISCARD_HAL_LABELS = 4, WEBGPU_INSTANCE_FLAG_ALLOW_UNDERLYING_NONCOMPLIANT_ADAPTER = 8, WEBGPU_INSTANCE_FLAG_GPU_BASED_VALIDATION = 16, WEBGPU_INSTANCE_FLAG_VALIDATION_INDIRECT_CALL = 32, WEBGPU_INSTANCE_FLAG_AUTOMATIC_TIMESTAMP_NORMALIZATION = 64, WEBGPU_INSTANCE_FLAG_DEFAULT = 16777216, WEBGPU_INSTANCE_FLAG_DEBUGGING = 33554432, WEBGPU_INSTANCE_FLAG_ADVANCED_DEBUGGING = 67108864, WEBGPU_INSTANCE_FLAG_WITH_ENV = 134217728 }
uint32_t { WEBGPU_D3D12_COMPILER_AUTO = 1, WEBGPU_D3D12_COMPILER_FXC = 2, WEBGPU_D3D12_COMPILER_DXC = 3 }
uint32_t { WEBGPU_D3D12_SWAPCHAIN_HWND = 1, WEBGPU_D3D12_SWAPCHAIN_VISUAL = 2 }
uint32_t { WEBGPU_GLES_VERSION_AUTO = 1, WEBGPU_GLES_VERSION_3_0 = 2, WEBGPU_GLES_VERSION_3_1 = 3, WEBGPU_GLES_VERSION_3_2 = 4 }
uint32_t { WEBGPU_GL_FENCE_NORMAL = 1, WEBGPU_GL_FENCE_AUTO_FINISH = 2 }
{ webgpu_present_t present; webgpu_format_t format; webgpu_alpha_t alpha; webgpu_power_t power; bool fallback; webgpu_backend_t backend; webgpu_feature_level_t feature_level; webgpu_samples_t samples; webgpu_depth_t depth; webgpu_limits_t limits; webgpu_features_t features; uint32_t error_room; webgpu_instance_backends_t instance_backends; webgpu_instance_flags_t instance_flags; webgpu_d3d12_compiler_t d3d12_compiler; webgpu_d3d12_swapchain_t d3d12_swapchain; webgpu_gles_version_t gles_version; webgpu_gl_fence_t gl_fence; uint8_t creation_threshold; uint8_t loss_threshold; char dxc_path[1024]; }
```

## The declarations

Each module that reads or writes a type of the module declares it in its `init`. A module in C gives the C type and the descriptor of the header.
Bridge compares the descriptors byte for byte. It also refuses a second module that states another kind, storage layout or `on_destroy`.

| Type of Bridge | Kind | Storage layout | `on_destroy` |
| --- | --- | --- | --- |
| `webgpu:webgpu_present_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_format_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_alpha_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_power_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_backend_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_feature_level_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_samples_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_depth_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_limits_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_features_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_instance_backends_t` | `BRIDGE_BITMASK` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_instance_flags_t` | `BRIDGE_BITMASK` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_d3d12_compiler_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_d3d12_swapchain_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_gles_version_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_gl_fence_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:DeviceSettings` | `BRIDGE_STRUCT` | `BRIDGE_SINGLETON` | `BRIDGE_ON_DESTROY_REMOVE` |

A module declares the 14 enums and the 2 bitmasks before `webgpu:DeviceSettings`, which names them. Bridge finds the type of a field only among the types that are already declared.
The module states `BRIDGE_STRIP` as the `on_unload` of each type, so its unload destroys no entity. Bridge does not compare `on_unload`, so each module states its own.
The module also declares the four types of *The contract*, as the document of `webgpu_contract.h` tells.

## The deployment

- `phases` lists `frame` after the phases of the app, then the phases of the render modules, then `present`.
- The app copies its window into `webgpu:Window` at each round, before `frame`. Thus it needs `write` with `webgpu:Window`, and `read` with `webgpu:webgpu_surface_t`.
- A render module reads `webgpu:Frame`. Thus it needs `read` with `webgpu:Frame` and `webgpu:webgpu_frame_state_t`.
- The module is of the namespace `webgpu`, so it needs no grant.
- A deployment gives the settings in `[module."webgpu:device@1.0.0".config]`.
- A deployment holds one wgpu-native library, beside the WebGPU modules, so that each module loads that one library.
- The wgpu-native library is no module, so `content.ignore` names it.
- On `aarch64-windows-gnu`, the wgpu-native library needs the Microsoft Visual C++ Redistributable for arm64 on the machine: the readme of BL-WGPU-Native.
- The backend of BB-Native has no capability of a limit, so `work_limit`, `memory_limit` and the three timeouts of the module are `"none"`.

The readme of Bridge-Host tells each key of a deployment.

## Messages

Each `<text>` is the text of wgpu-native, which can hold line ends. Then one line of the module takes more than one line of the output of the host.

The module writes these lines at the level 3. Then the system gives 1, and tries again at its next run:

- `no device: <reason>: <text>`: a request of *The make* gives no adapter or no device. `<reason>` is `no adapter` or `the adapter refuses the device`, and `<text>` is the text of wgpu-native.
- `no surface: <reason>`: the module has no surface, or the surface gives no frame. `<reason>` is `the window has no handle of this system`, `the adapter gives no format for the window` or `the surface gives an error`.

The module writes these lines at the level 3, and the system goes on:

- `device lost: <reason>: <text>`: *A lost device*.
- `webgpu error: <type>: <text>`: *The errors*.
- `webgpu error: <count> texts did not fit in error_room`: *The errors*.

The module writes this line at the level 4, when the surface or the device does not give the value of a setting:

- `no <setting> <value>: the frame uses <value>`: `<setting>` is `present`, `format`, `alpha`, `samples` or `depth`. The first `<value>` is the name of the constant of the setting. The second one is the name of the constant that the frame uses in its place.
