# src/device_errors.c

This file holds the room where the error texts of the device wait for `frame`.
It also holds the two callbacks that write into the room, and the function of `frame` that reads it.

## The room

`docs/extern/webgpu_device.h.md`, *The errors*, tells why the two callbacks of the device copy their texts into the room.
They keep the rules of `docs/intern/device_inside.h.md`, *Callbacks*. `room_t` has these fields:

| Field | Content |
| --- | --- |
| `bytes`, `size` | The bytes of the room, and their count. |
| `used` | An atomic count of the bytes that the texts took. |
| `missed` | An atomic count of the texts that found no room. |
| `lost` | An atomic flag: the device was lost since the last run of `frame`. |

`device_size_room` frees the bytes, then allocates the new size. `device_ask_device` gives it `error_room` before each request, and `bridge_shutdown` gives it 0.
A size of 0 allocates nothing. If the allocation fails, the room has 0 bytes, so each text counts as a text that did not fit.

## A writer

`device_keep_error` and `device_keep_lost` give their message to `keep_text`. Then `device_keep_lost` sets `lost`.
A text is a head of `TEXT_HEAD` bytes, then the message. The head holds the kind `TEXT_ERROR` or `TEXT_LOST`, the code of `webgpu.h`, and the length.

```pseudo
FUNCTION keep_text(d : device_t, head : text_head_t, text : WGPUStringView) RETURNS BOOLEAN
   need <- TEXT_HEAD + head.len
   at <- atomic_load(d.room.used)
   REPEAT
      IF need > d.room.size - at THEN
         CALL atomic_fetch_add(d.room.missed, 1)
         RETURN FALSE
      ENDIF
   UNTIL atomic_compare_exchange_weak(d.room.used, at, at + need)
   CALL memcpy(d.room.bytes[at], head, TEXT_HEAD)
   CALL memcpy(d.room.bytes[at + TEXT_HEAD], text.data, head.len)
   RETURN TRUE
ENDFUNCTION
```

When two threads write at once, both read `used`, and only one exchange succeeds. The other one gets the new `used` in `at`, and tries again after the first text.
Thus each writer gets its own bytes, from `at` to `at + need`, and `used` never passes `size`.

## The reader

`device_drain_room` runs at the start of each run of `frame`:

1. It writes a line at the level 3 for each text below `used`, with `bridge_native_say`.
2. A text of `TEXT_LOST` gives `device lost: <reason>: <text>`, else `webgpu error: <type>: <text>`.
3. If `missed` is more than 0, it writes the count: `docs/extern/webgpu_device.h.md`, *Messages*.
4. It sets `used` and `missed` to 0, and gives the old value of `lost`.

The text of each line goes into the scratch arena. `docs/extern/webgpu_device.h.md`, *The module*, gives the room that `scratch` needs for it.
The tables `ERROR_NAMES` and `LOST_NAMES` give the name of each code. A code that a table does not hold gives `unknown`.
wgpu-native `v29.0.1.1` gives only the types `validation` and `out of memory`, and only the reason `destroyed`: https://github.com/gfx-rs/wgpu-native/blob/v29.0.1.1/src/lib.rs#L511-L569.

`frame` reads the room safely, because no writer runs during `frame`:

- A writer runs only inside a call of WebGPU on an object of the device.
- The module makes its own calls on the thread of `frame`, after the read, or in `present`.
- Each other call comes from a render module, which reads `webgpu:Frame`: `docs/extern/webgpu_device.h.md`, *The deployment*.
- `frame` writes `webgpu:Frame`, so Bridge never runs a system that reads it at the same time: the section Conflicts of `bridge_abi.h`.
- The phases run one after another, and the observers run at a barrier, after the systems of the phase.

Thus each writer ended its copy before `frame` reads `used`. The atomics only keep two writers apart.

## Functions

| Function | Result |
| --- | --- |
| `device_keep_error` | The callback of an error that no scope caught. Keeps its message with the kind `TEXT_ERROR`. |
| `device_keep_lost` | The callback of a lost device. Keeps its message with the kind `TEXT_LOST`, then sets `lost`. |
| `keep_text` | Takes bytes in the room, and copies a text into them. Gives false, and counts the text, when it does not fit. |
| `device_drain_room` | Writes the lines of the room, empties it, and gives true when the device was lost. |
| `device_size_room` | Frees the room, then allocates it with a new size. |
