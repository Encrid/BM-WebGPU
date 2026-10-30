# src/device/webgpu_device.c

This file holds the manifest of the module, its state, and its entries `init`, `shutdown` and `reload`.

## The manifest

The kind `wgpu_module` makes the module, and links the library by its bare name: `readme.md`, *wgpu_native*.
Thus each WebGPU module of a deployment loads the one copy of the library beside them.
The kind gives the words of the manifest as the macro `MANIFEST`.
The file puts that macro into the section of the target, with the line of `.toolchain/docs/presets.md`, *native_module*.

## The state

A native module gets only its API at each call. Thus the objects of WebGPU have no other place between two rounds.
The file keeps them in one static `device_t`: `docs/intern/device/device_inside.h.md`, *Types*.
A `[[style.exception]]` of the project excuses this struct from the rule G4, with that reason.
`device_get_state` gives the struct to the other sources.

## The start

`bridge_init` does these steps:

1. `declare_types` declares the 16 enums and the 2 bitmasks of `docs/intern/device/device_inside.h.md`, *The slots*, from the table `TYPES`.
2. It declares the singletons `webgpu:DeviceSettings`, `webgpu:Window` and `webgpu:Frame`.
3. It attaches a `webgpu:Frame` with the state `WEBGPU_FRAME_NO_DEVICE`, and each other field at 0.
4. It adds the system `frame` in the phase `frame`, and the system `present` in the phase `present`.

Each item of `TYPES` is a `declared_type_t`: `docs/intern/device/device_inside.h.md`, *The slots*.
Each enum and each bitmask is a `uint32_t`, so each declaration of the table has the size 4 and the alignment 4.
Each declaration states the kind, the storage layout, `on_unload` and `on_destroy` of `docs/extern/device/webgpu_device.h.md`, *The declarations*.
`frame` has three optional terms: it reads the settings and the window, and it writes the frame. `present` reads the frame as an optional term.
`init` makes no object of WebGPU, because the settings come after it: `docs/extern/device/webgpu_device.h.md`, *The systems*.

## The stop

`bridge_shutdown` writes its API into the state, because a release can call the callback of a request.
Then it releases each object with `device_release`, and frees the room with `device_size_room`.
The release blocks while the GPU ends its work: `docs/intern/device/device_make.c.md`, *The release*.

`bridge_reload` gives 1. Bridge needs the entry, but never calls it, because the manifest states `reloadable=false`.

## Functions

| Function | Result |
| --- | --- |
| `bridge_init` | Declares the 21 types, attaches `webgpu:Frame`, and adds the 2 systems. Gives 0. |
| `declare_types` | Declares the 16 enums and the 2 bitmasks that the module uses. |
| `bridge_shutdown` | Releases each object of WebGPU, then frees the room. Gives 0. |
| `bridge_reload` | Gives 1. |
| `device_get_state` | Gives the state of the module. |
