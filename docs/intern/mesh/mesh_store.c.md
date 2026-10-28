# src/mesh_store.c

This file holds the state of the module, and reads the storage of Bridge.
It also reads the room that is left in `changes`.

## The state

A native module gets only its API at each call, so its state has no other place between two rounds.
The file keeps it in one static struct, and `mesh_get_state` gives it to the other sources.
A `[[style.exception]]` of the project excuses this struct from the rule G4, with that reason.
Bridge runs the systems and the observers of one module one after another: `bridge.h.md` of Bridge, in its section bridge_run_phase.
Thus no two calls change the state at the same time.

## The walk

`mesh_walk` gives each row of one component to a visit, and gives the count of the rows. It reads the spans of the query of `render`:

```pseudo
FUNCTION mesh_walk(slot : U32, visit : Visit) RETURNS U32
   n <- 0
   FOR s <- 0 TO query.spans - 1
      table <- query.span[s].table
      base <- column_base[table * types + slot]
      rows <- query.span[s].rows
      IF base = 0 THEN
         rows <- 0
      ENDIF
      FOR row <- 0 TO rows - 1
         CALL visit(blocks[table].id[row], base + row * stride[slot])
      NEXT row
      n <- n + rows
   NEXT s
   RETURN n
ENDFUNCTION
```

A `base` of 0 tells that the table does not carry the component. With no visit, `mesh_walk` only counts the rows.

## The value of a component

`mesh_find` gives where the value of a component of an entity is, or 0. It does these steps:

1. It gives 0 for an id whose index is not below `entities`.
2. It gives 0 when the generation of the id is not the generation in the entity index.
3. For a component of a table, it reads the cell of the table and the row of the entity.
4. For a sparse set, it reads the cell of the entity in the index of the set.

`kind` of the header gives the storage of each slot. A set whose kept cell is still 0 gives 0, for example before the first barrier.
A husk keeps its generation, so its id finds its kept components, and no other component.
A value stays good until the next barrier, so no source keeps it longer.

## The data rules

`mesh_get_data` gives the bytes of a field `bridge_text_t`, or 0. It applies `docs/extern/webgpu_mesh.h.md`, *The data rules*.
The bytes must lie from `arena_lo` to `arena_hi` of the header. The test does not add the place and the length, so a long field cannot wrap.
The length must not be 0, and must be a multiple of the unit that the caller gives. The unit is the stride of a mesh or of a batch, or 4 for a texture.

## The room of changes

Each change takes 24 bytes of `changes`, and an attach also takes its value: `bridge.h.md` of Bridge, in its section The reservations.
A change that finds no room fails the invocation that makes it.
`changes_left` of the header tells the room that is left: `bridge_abi.h.md` of Bridge, in its section bridge_header_t.
Thus `mesh_take_change` reads it before each attach and each detach of the module, and gives true when the change fits.

The observers of a barrier and the systems of the next phase take from the same room.
With less room, more changes wait for a later round, and no invocation fails.

## Functions

| Function | Result |
| --- | --- |
| `mesh_get_state` | Gives the state of the module. |
| `mesh_walk` | Gives each row of a component to a visit, and gives the count of the rows. |
| `mesh_find` | Gives where the value of a component of an entity is, or 0. |
| `mesh_get_data` | Gives the bytes of a field of data, or 0 when a data rule refuses them. |
| `mesh_take_change` | Gives true when the room that is left in `changes` holds one change of that many bytes. |
