# src/device_lines.c

This file writes the lines that the steps of `frame` write: the three families of `docs/extern/webgpu_device.h.md`, *Messages*.
Each family has one function, so that each line of a family has one format.

## The families

| Function | Family | Level |
| --- | --- | --- |
| `device_say_no_device` | `no device` | `BRIDGE_LOG_ERR` |
| `device_say_no_surface` | `no surface` | `BRIDGE_LOG_ERR` |
| `device_say_fallback` | The line of a setting that the frame does not use. | `BRIDGE_LOG_WARN` |

Each function writes with `api` of the state, because a callback of a request gets no API: `docs/intern/device_inside.h.md`, *Types*.
`device_say_no_device` writes the text of wgpu-native with its length, because a `WGPUStringView` has no zero at its end.

## The names

`device_say_fallback` gets the setting with an item of `fallback_t`, and the two values of the line as values of the enum of the setting.
The table `FALLBACKS` gives the key and the descriptor of the enum of each item, for example `alpha` and `webgpu_alpha_t_desc`.
`find_name` finds the name of a value in that descriptor. Thus the text of the enum is the one text of the names.
Each constant of the text of an enum states its value: `docs/extern/webgpu_device.h.md`, *The C types*. Thus a name is the word before ` = <value>`.

## Functions

| Function | Result |
| --- | --- |
| `device_say_no_device` | Writes `no device` with its reason and the text of wgpu-native. |
| `device_say_no_surface` | Writes `no surface` with its reason. |
| `device_say_fallback` | Writes the line of a setting with the constant that the setting asks for, and the constant that the frame uses. |
| `find_name` | Gives the name of a value, and its length, from the descriptor of an enum. |
