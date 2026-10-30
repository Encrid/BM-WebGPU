# src/mesh/mesh_lines.c

This file writes the two lines of the module: `docs/extern/mesh/webgpu_mesh.h.md`, *Messages*.

## The lines

`mesh_say_no_room` writes the line of `gpu_memory`, and `mesh_say_left_out` writes the line of a rule.
Each one writes its line only if its flag of the state is not set, and then sets the flag.
`mesh_say_no_room` has the flag `said_room`, and `mesh_say_left_out` has the flag `said_rule`. `open_round` of `src/mesh/mesh_round.c` clears the two flags at the start of each round.
Thus each line comes at most once in a round, and the line of a rule tells of the first item that a rule refuses.

`mesh_say_left_out` gets the id of the entity and an item of `mesh_rule_t`. A table of the file gives the text of each rule, in the order of the enum.
The round has the id 0.

Each one writes with `bridge_native_say` of BB-Native, at the level `BRIDGE_LOG_WARN`, with the api that the system that runs keeps in the state.
Its format is one string literal, with `%llu` for each number of 64 bits, and a cast of each number to `unsigned long long`.
A macro such as `PRIu64` would cut the literal. The check of `.toolchain/docs/said.py.md`, *The format*, joins only the strings that follow each other.
The check of the messages reads these formats, because `[test] writers` names the writer: `docs/contributing/tests.md`, *The messages of the code*.

## Functions

| Function | Result |
| --- | --- |
| `mesh_say_no_room` | Writes the line of `gpu_memory` with the bytes that the module holds and the bytes that the item needs, once in a round. |
| `mesh_say_left_out` | Writes the line of a rule with the id of the entity and the text of the rule, once in a round. |
