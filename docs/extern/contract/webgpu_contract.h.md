# include/webgpu_contract.h

`include/webgpu_contract.h` gives the four types that the WebGPU modules of Bridge share: their C types, their constants and their descriptors.
Each module in C that writes or reads one of these types includes this header.

## The roles

- The app writes `webgpu:Window` at each round, from the window of the app, for example `sdl3:Window`.
- The device module, for example `webgpu:device`, reads `webgpu:Window`, and writes `webgpu:Frame`.
- Each render module, for example `webgpu:mesh`, reads `webgpu:Frame`.

Each of `webgpu:Window` and `webgpu:Frame` has one writer.
In each round, the app writes `webgpu:Window` before the device module reads it.
The device module writes `webgpu:Frame` before the render modules read it.

## webgpu:Window

`webgpu:Window` is a singleton. Its C type `webgpu_window_t` has 72 bytes, and an alignment of 8.

| Field | C type | Offset | Value |
| --- | --- | --- | --- |
| `surface` | `webgpu_surface_t` | 0 | The kind of the window. |
| `width` | `uint32_t` | 4 | The width of the frame, in pixels. |
| `height` | `uint32_t` | 8 | The height of the frame, in pixels. |
| `hinstance` | `uint64_t` | 16 | On Windows: the instance of the program. |
| `hwnd` | `uint64_t` | 24 | On Windows: the window. |
| `layer` | `uint64_t` | 32 | On macOS: the `CAMetalLayer` of the window. |
| `x11_display` | `uint64_t` | 40 | On X11: the display. |
| `x11_window` | `uint64_t` | 48 | On X11: the number of the window. |
| `wl_display` | `uint64_t` | 56 | On Wayland: the display. |
| `wl_surface` | `uint64_t` | 64 | On Wayland: the surface of the window. |

The bytes 12 to 15 hold no field, because `hinstance` starts at a multiple of 8.
The device module reads only the handles of the kind of `surface`. The app writes 0 in the handles of the other kinds.
The device module makes its surface from the handles one time. To give another window, the app writes `WEBGPU_SURFACE_NONE` for one round first.

### webgpu:webgpu_surface_t

`webgpu_surface_t` is an enum of `uint32_t`: 4 bytes, with an alignment of 4.

| Constant | Value | The window |
| --- | --- | --- |
| `WEBGPU_SURFACE_NONE` | 1 | No window. The device module releases its surface and its device. |
| `WEBGPU_SURFACE_WINDOWS` | 2 | A window of Windows: `hinstance` and `hwnd`. |
| `WEBGPU_SURFACE_METAL` | 3 | A window of macOS: `layer`. |
| `WEBGPU_SURFACE_X11` | 4 | A window of X11: `x11_display` and `x11_window`. |
| `WEBGPU_SURFACE_WAYLAND` | 5 | A window of Wayland: `wl_display` and `wl_surface`. |

## webgpu:Frame

`webgpu:Frame` is a singleton. Its C type `webgpu_frame_t` has 64 bytes, and an alignment of 8.

| Field | C type | Offset | Value |
| --- | --- | --- | --- |
| `state` | `webgpu_frame_state_t` | 0 | What a render module does in this round. |
| `format` | `uint32_t` | 4 | The `WGPUTextureFormat` of `frame` and of `target`. |
| `depth_format` | `uint32_t` | 8 | The `WGPUTextureFormat` of `depth`, or 0 when the frame has no depth texture. |
| `samples` | `uint32_t` | 12 | The samples of each pixel of `target` and of `depth`. |
| `width` | `uint32_t` | 16 | The width of the frame, in pixels. |
| `height` | `uint32_t` | 20 | The height of the frame, in pixels. |
| `device` | `uint64_t` | 24 | The `WGPUDevice`. |
| `queue` | `uint64_t` | 32 | The `WGPUQueue` of the device. |
| `frame` | `uint64_t` | 40 | The `WGPUTextureView` of the frame, which the surface presents. |
| `target` | `uint64_t` | 48 | The `WGPUTextureView` to draw into: `frame` with 1 sample, else the multisample texture. Its usage is `RenderAttachment`. |
| `depth` | `uint64_t` | 56 | The `WGPUTextureView` of the depth texture, or 0 when the frame has no depth texture. Its usages are `RenderAttachment` and `CopySrc`. |

In `WEBGPU_FRAME_SKIPPED`, `frame`, `target` and `depth` are 0, and `device` and `queue` keep their values while the device is there.
Before the device module has a device, and in `WEBGPU_FRAME_NO_DEVICE`, each field but `state` is 0.
With more than 1 sample, each render pass resolves `target` into `frame`.

### webgpu:webgpu_frame_state_t

`webgpu_frame_state_t` is an enum of `uint32_t`: 4 bytes, with an alignment of 4.

| Constant | Value | What a render module does |
| --- | --- | --- |
| `WEBGPU_FRAME_READY` | 1 | It draws into `target`. |
| `WEBGPU_FRAME_SKIPPED` | 2 | It draws nothing in this round, and keeps its objects. |
| `WEBGPU_FRAME_NO_DEVICE` | 3 | It releases its objects now, because the window closed or the device was lost. At the next `WEBGPU_FRAME_READY`, it makes them again. |

## The C types

The header includes `bridge_schema.h` of Bridge. It makes each C type and its descriptor from one text, with the macros of that header.

| Type of Bridge | C type | Descriptor | Macro of the text |
| --- | --- | --- | --- |
| `webgpu:webgpu_surface_t` | `webgpu_surface_t` | `webgpu_surface_t_desc` | `WEBGPU_SURFACE_VALUES` |
| `webgpu:webgpu_frame_state_t` | `webgpu_frame_state_t` | `webgpu_frame_state_t_desc` | `WEBGPU_FRAME_STATE_VALUES` |
| `webgpu:Window` | `webgpu_window_t` | `webgpu_window_t_desc` | `WEBGPU_WINDOW_FIELDS` |
| `webgpu:Frame` | `webgpu_frame_t` | `webgpu_frame_t_desc` | `WEBGPU_FRAME_FIELDS` |

Each macro of the text gives the braces of its type and the text between them.
A module in another language does not include the header. It states the descriptor of each type with these bytes, in the order of the table:

```text
uint32_t { WEBGPU_SURFACE_NONE = 1, WEBGPU_SURFACE_WINDOWS = 2, WEBGPU_SURFACE_METAL = 3, WEBGPU_SURFACE_X11 = 4, WEBGPU_SURFACE_WAYLAND = 5 }
uint32_t { WEBGPU_FRAME_READY = 1, WEBGPU_FRAME_SKIPPED = 2, WEBGPU_FRAME_NO_DEVICE = 3 }
{ webgpu_surface_t surface; uint32_t width; uint32_t height; uint64_t hinstance; uint64_t hwnd; uint64_t layer; uint64_t x11_display; uint64_t x11_window; uint64_t wl_display; uint64_t wl_surface; }
{ webgpu_frame_state_t state; uint32_t format; uint32_t depth_format; uint32_t samples; uint32_t width; uint32_t height; uint64_t device; uint64_t queue; uint64_t frame; uint64_t target; uint64_t depth; }
```

## The declarations

Each module that uses a type declares it in its `init`. A module in C gives the C type and the descriptor of this header.
Bridge compares the descriptors byte for byte. It also refuses a second module that states another kind, storage layout or `on_destroy`.

| Type of Bridge | Kind | Storage layout | `on_destroy` |
| --- | --- | --- | --- |
| `webgpu:webgpu_surface_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:webgpu_frame_state_t` | `BRIDGE_ENUM` | `BRIDGE_TABLE` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:Window` | `BRIDGE_STRUCT` | `BRIDGE_SINGLETON` | `BRIDGE_ON_DESTROY_REMOVE` |
| `webgpu:Frame` | `BRIDGE_STRUCT` | `BRIDGE_SINGLETON` | `BRIDGE_ON_DESTROY_REMOVE` |

A module declares an enum before the struct that names it. Bridge finds the type of a field only among the types that are already declared.
Bridge does not compare `on_unload`, so each module states its own.

## The grants

A module of the namespace `webgpu` needs no grant for these types.
A module of another namespace needs a grant for each type that it declares. The deployment gives it in `read` or `write` of the module.

| Role | `write` | `read` |
| --- | --- | --- |
| The app | `webgpu:Window` | `webgpu:webgpu_surface_t` |
| The device module | `webgpu:Frame` | `webgpu:Window`, `webgpu:webgpu_surface_t` and `webgpu:webgpu_frame_state_t` |
| A render module | No type. | `webgpu:Frame` and `webgpu:webgpu_frame_state_t` |

The document of `bridge_abi.h` of Bridge tells what each grant allows.
