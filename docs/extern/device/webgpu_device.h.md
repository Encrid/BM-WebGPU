# include/webgpu_device.h

`include/webgpu_device.h` is the header of the module `webgpu:device@1.0.0`, the WebGPU device of Bridge.
The module makes the device and the surface of one window, and gives the frame of each round to the render modules.
It covers the instance, the adapter, the device and the surface of `webgpu.h`. It draws nothing: each render module draws its own passes.

## The module

| Word of the manifest | Value |
| --- | --- |
| The identity | `webgpu:device@1.0.0` |
| `abi` | `1` |
| `config` | `webgpu:DeviceSettings` |
| `settings` | `present format alpha power fallback backend feature_level samples depth limits features` |
| `pinned` | `true`, because on macOS the module must make and configure the surface on the thread of the window. |
| `reloadable` | `false`, because the objects of each render module belong to its device. |

The manifest also gives a default for each of these keys of the module. A deployment that states one of them replaces its default.

| Key | Default | Reason |
| --- | --- | --- |
| `scratch` | `16384` | The declarations of the fifteen types of `init`, and the value of `webgpu:Frame` that it attaches. |
| `log_ring` | `2048` | Room for 64 records of 32 bytes: the errors of the device in one round. |
| `spans` | `4096` | A cell for each of the fifteen types in each table, and the spans of the two systems. |
| `spawns` | `"none"` | The module makes no entity. |
| `changes` | `88` | The attach of `webgpu:Frame` in `init`: 24 bytes, and its value of 64 bytes. |
| `persistent` | `"none"` | The module keeps no data in the arena. |
| `hot_reload` | `false` | The module cannot reload. |

The header `bridge.h` of Bridge tells the room that `spans` needs in a world with many tables.

The file of the module is `webgpu_device`, with the suffix of a library. It runs on the backend of BB-Native, and loads the wgpu-native library of BL-WGPU-Native, which sits beside it.
It needs no SDL3: the app gives it the window through `webgpu:Window`.

## The systems

| System | Phase | What it does |
| --- | --- | --- |
| `frame` | `frame` | Gets the frame of the round, and writes `webgpu:Frame`. *The frame* tells its steps. |
| `present` | `present` | If the state of `webgpu:Frame` is `WEBGPU_FRAME_READY`, it presents the frame. |

The settings come after `init`, so the device comes at the first run of `frame` with a window.
The full names of the systems for `[order]` are `webgpu:device:frame` and `webgpu:device:present`.

## The frame

`frame` does these steps at each run:

1. If no `webgpu:Window` is there, or its `surface` is `WEBGPU_SURFACE_NONE`, it releases the surface and the device.
   Then it writes the state `WEBGPU_FRAME_NO_DEVICE`, and stops.
2. If it has no device, it makes the instance, the surface, the adapter and the device, with the settings.
3. If the size of `webgpu:Window` changed, it configures the surface for the new size.
4. It gets the texture of the frame from the surface, and acts on its status: *The status of a frame*.
5. It writes `webgpu:Frame` with the state `WEBGPU_FRAME_READY`.

If a step gets no adapter, device or surface, it writes its line of *Messages* at the level 3. Then the system gives 1.
The system tries again at its next run, until `max_system_failures` stops the module.

## The status of a frame

| Status of `webgpu.h` | What `frame` does |
| --- | --- |
| `SuccessOptimal` | It gives the frame. |
| `SuccessSuboptimal` | It gives the frame, and configures the surface again at its next run. |
| `Timeout`, `Outdated` | It writes the state `WEBGPU_FRAME_SKIPPED`, and configures the surface again. |
| `Lost` | It writes the state `WEBGPU_FRAME_SKIPPED`, and makes the surface again at its next run. |
| `Error` | It writes `no frame: the surface gives an error` at the level 3, and the system gives 1. |

## A lost device

When the device is lost, the module writes `device lost: <reason>: <text>` at the level 3.
`<reason>` is `unknown`, `destroyed`, `callback cancelled` or `failed creation`, and `<text>` is the text of wgpu-native.
In that round, the state of `webgpu:Frame` is `WEBGPU_FRAME_NO_DEVICE`. At the next run of `frame`, the module makes a new device with the same settings.

## The errors

The module writes each error of the device that no scope catches: `webgpu error: <type>: <text>`, at the level 3.
`<type>` is `validation`, `out of memory`, `internal` or `unknown`. A render module can catch its own errors with a scope of `webgpu.h`.
Each object that the module makes has a label that starts with `webgpu:device`.
Each render module gives its objects a label that starts with its full name. Thus the text of an error tells which module made the object.

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
| `feature_level` | `webgpu_feature_level_t` | The level of WebGPU. The default is `WEBGPU_FEATURE_LEVEL_CORE`. |
| `samples` | `webgpu_samples_t` | The samples of each pixel of the frame. The default is `WEBGPU_SAMPLES_1`. |
| `depth` | `webgpu_depth_t` | The depth texture of the frame. The default is `WEBGPU_DEPTH_24_PLUS_STENCIL_8`. |
| `limits` | `webgpu_limits_t` | The limits that the device asks for. The default is `WEBGPU_LIMITS_ADAPTER`. |
| `features` | `webgpu_features_t` | The features that the device asks for. The default is `WEBGPU_FEATURES_ADAPTER`. |

If `fallback` is `true`, or `backend` is not `WEBGPU_BACKEND_ANY`, a system with no such adapter gives none: *Messages*.
If the surface or the device does not give the value of a setting, the module uses another value. It writes a line at the level 4.

Each enum of the settings is of `uint32_t`. A deployment states a constant by its name or by its value.

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
| `WEBGPU_BACKEND_D3D11` | 2 | Direct3D 11. |
| `WEBGPU_BACKEND_D3D12` | 3 | Direct3D 12. |
| `WEBGPU_BACKEND_METAL` | 4 | Metal. |
| `WEBGPU_BACKEND_VULKAN` | 5 | Vulkan. |
| `WEBGPU_BACKEND_OPENGL` | 6 | OpenGL. |
| `WEBGPU_BACKEND_OPENGLES` | 7 | OpenGL ES. |

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
| `WEBGPU_LIMITS_DEFAULT` | 2 | The minimum limits of WebGPU, which each device gives. A world that runs with them fits each WebGPU device. |

### webgpu:webgpu_features_t

| Constant | Value | The features of the device |
| --- | --- | --- |
| `WEBGPU_FEATURES_ADAPTER` | 1 | Each feature that the adapter gives. |
| `WEBGPU_FEATURES_DEFAULT` | 2 | Only the features that `samples` and `depth` need. |

## webgpu:Window

`webgpu:Window` is a singleton that the app writes at each round, from the window of the app, for example `sdl3:Window`.

| Field | C type | Value |
| --- | --- | --- |
| `surface` | `webgpu_surface_t` | The kind of the window. |
| `width`, `height` | `uint32_t` | The size of the frame, in pixels. |
| `hinstance`, `hwnd` | `uint64_t` | On Windows: the instance of the program, and the window. |
| `layer` | `uint64_t` | On macOS: the `CAMetalLayer` of the window. |
| `x11_display`, `x11_window` | `uint64_t` | On X11: the display, and the number of the window. |
| `wl_display`, `wl_surface` | `uint64_t` | On Wayland: the display, and the surface of the window. |

The module reads only the handles of the kind of `surface`. The handles of the other kinds are 0.

### webgpu:webgpu_surface_t

| Constant | Value | The window |
| --- | --- | --- |
| `WEBGPU_SURFACE_NONE` | 1 | No window. The module releases the surface and the device. |
| `WEBGPU_SURFACE_WINDOWS` | 2 | A window of Windows: `hinstance` and `hwnd`. |
| `WEBGPU_SURFACE_METAL` | 3 | A window of macOS: `layer`. |
| `WEBGPU_SURFACE_X11` | 4 | A window of X11: `x11_display` and `x11_window`. |
| `WEBGPU_SURFACE_WAYLAND` | 5 | A window of Wayland: `wl_display` and `wl_surface`. |

## webgpu:Frame

`webgpu:Frame` is a singleton that only the module writes. Each render module reads it.

| Field | C type | Value |
| --- | --- | --- |
| `state` | `webgpu_frame_state_t` | What a render module does in this round. |
| `format` | `uint32_t` | The `WGPUTextureFormat` of `frame` and of `target`. |
| `depth_format` | `uint32_t` | The `WGPUTextureFormat` of `depth`, or 0 with `WEBGPU_DEPTH_NONE`. |
| `samples` | `uint32_t` | The samples of each pixel of `target` and of `depth`. |
| `width`, `height` | `uint32_t` | The size of the frame, in pixels. |
| `device` | `uint64_t` | The `WGPUDevice`. |
| `queue` | `uint64_t` | The `WGPUQueue` of the device. |
| `frame` | `uint64_t` | The `WGPUTextureView` of the frame, which the surface presents. |
| `target` | `uint64_t` | The `WGPUTextureView` to draw into: `frame` with 1 sample, else the multisample texture. |
| `depth` | `uint64_t` | The `WGPUTextureView` of the depth texture, or 0 with `WEBGPU_DEPTH_NONE`. |

In `WEBGPU_FRAME_SKIPPED`, `device` and `queue` stay, and `frame`, `target` and `depth` are 0. In `WEBGPU_FRAME_NO_DEVICE`, each handle is 0.
With more than 1 sample, each render pass resolves `target` into `frame`.
A frame that no render module clears is black, because WebGPU clears each new frame to 0.

### webgpu:webgpu_frame_state_t

| Constant | Value | What a render module does |
| --- | --- | --- |
| `WEBGPU_FRAME_READY` | 1 | It draws into `target`. |
| `WEBGPU_FRAME_SKIPPED` | 2 | It draws nothing in this round, and keeps its objects. |
| `WEBGPU_FRAME_NO_DEVICE` | 3 | It releases its objects now, because the window closed or the device was lost. At the next `WEBGPU_FRAME_READY`, it makes them again. |

## The C types

`include/webgpu_device.h` gives the C type of each type of the module, and its descriptor for Bridge.

| Type of Bridge | C type | Descriptor |
| --- | --- | --- |
| `webgpu:webgpu_present_t` | `webgpu_present_t` | `webgpu_present_t_desc` |
| `webgpu:webgpu_format_t` | `webgpu_format_t` | `webgpu_format_t_desc` |
| `webgpu:webgpu_alpha_t` | `webgpu_alpha_t` | `webgpu_alpha_t_desc` |
| `webgpu:webgpu_power_t` | `webgpu_power_t` | `webgpu_power_t_desc` |
| `webgpu:webgpu_backend_t` | `webgpu_backend_t` | `webgpu_backend_t_desc` |
| `webgpu:webgpu_feature_level_t` | `webgpu_feature_level_t` | `webgpu_feature_level_t_desc` |
| `webgpu:webgpu_samples_t` | `webgpu_samples_t` | `webgpu_samples_t_desc` |
| `webgpu:webgpu_depth_t` | `webgpu_depth_t` | `webgpu_depth_t_desc` |
| `webgpu:webgpu_limits_t` | `webgpu_limits_t` | `webgpu_limits_t_desc` |
| `webgpu:webgpu_features_t` | `webgpu_features_t` | `webgpu_features_t_desc` |
| `webgpu:webgpu_surface_t` | `webgpu_surface_t` | `webgpu_surface_t_desc` |
| `webgpu:webgpu_frame_state_t` | `webgpu_frame_state_t` | `webgpu_frame_state_t_desc` |
| `webgpu:DeviceSettings` | `webgpu_device_settings_t` | `webgpu_device_settings_t_desc` |
| `webgpu:Window` | `webgpu_window_t` | `webgpu_window_t_desc` |
| `webgpu:Frame` | `webgpu_frame_t` | `webgpu_frame_t_desc` |

## The deployment

- `phases` lists `frame` after the phases of the app, then the phases of the render modules, then `present`.
- The app copies its window into `webgpu:Window` at each round, before `frame`, so it needs `write` with `webgpu:Window`.
- A render module reads `webgpu:Frame`, so it needs `read` with `webgpu:Frame`.
- A deployment gives the settings in `[module."webgpu:device@1.0.0".config]`.
- A deployment holds one wgpu-native library, beside the WebGPU modules, so that each module loads that one library.
- The wgpu-native library is no module, so `content.ignore` names it.
- The backend of BB-Native has no capability of a limit, so `work_limit`, `memory_limit` and the three timeouts of the module are `"none"`.

The readme of Bridge-Host tells each key of a deployment.

## Messages

The module writes these lines at the level 3. Then the system gives 1, and tries again at its next run:

- `no adapter: <reason>`: the request gives no adapter for the settings. `<reason>` is the text of wgpu-native.
- `no device: <reason>`: the adapter gives no device. `<reason>` is the text of wgpu-native.
- `no surface: <reason>`: the surface of the window cannot be made or configured. `<reason>` is the text of wgpu-native.
- `no frame: the surface gives an error`: *The status of a frame*.

The module writes these lines at the level 3, and the system goes on:

- `device lost: <reason>: <text>`: *A lost device*.
- `webgpu error: <type>: <text>`: *The errors*.

The module writes these lines at the level 4, when the surface or the device does not give the value of a setting:

- `no present mode <mode>: the surface presents with FIFO`
- `no format <format>: the frame uses <format>`
- `no alpha mode <mode>: the surface uses <mode>`
- `no <count> samples: the frame uses 4`
- `no depth 32_FLOAT_STENCIL_8: the depth uses 24_PLUS_STENCIL_8`
