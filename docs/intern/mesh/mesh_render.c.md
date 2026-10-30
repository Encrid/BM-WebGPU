# src/mesh/mesh_render.c

This file holds the systems `draws`, `batches` and `render`. They fill the round, and `render` encodes it.

## The items of a round

`draws` and `batches` run after `keep_textures`, so a draw finds each upload of its round: `docs/intern/mesh/mesh_systems.c.md`, *The order*.
Each one gives its kind of item to `add_items`:

```pseudo
FUNCTION add_items(system : U32, slot : U32, visit : Visit) RETURNS I32
   IF NOT state.ready THEN
      RETURN 0
   ENDIF
   IF NOT mesh_reserve_round(ITEMS, mesh_walk(system, slot, NONE)) THEN
      state.ready <- FALSE
      RETURN 1
   ENDIF
   CALL mesh_walk(system, slot, visit)
   RETURN 0
ENDFUNCTION
```

`draws` gives `DRAW` and `mesh_add_draw`, and `batches` gives `BATCH` and `mesh_add_batch`: `docs/intern/mesh/mesh_draw.c.md`.
`NONE` is no visit, so the first walk only counts the rows.
A round whose memory cannot grow sets `ready` to false, so the systems after it draw nothing.

## The pass of a round

`render` runs last:

```pseudo
FUNCTION bridge_render(f : Frame, s : Scene) RETURNS I32
   IF NOT state.ready THEN
      RETURN 0
   ENDIF
   IF NOT mesh_reserve_round(VIEWS, mesh_walk(RENDER, VIEW, NONE)) THEN
      RETURN 1
   ENDIF
   state.scene <- s
   CALL mesh_walk(RENDER, VIEW, mesh_add_view)
   CALL sort(state.round.views, compare_keys)
   CALL sort(state.round.items, compare_keys)
   CALL mesh_put_round()
   CALL mesh_encode(f)
   RETURN 0
ENDFUNCTION
```

`sort` is `qsort` of the C library. With no view, the round still encodes its pass, with no draw in it.
The blocks of the views and the clear of the pass read the scene that the state keeps. For a round with no `webgpu:Scene`, it is a constant scene of zeros.

## The order of the items

`compare_keys` orders two keys by `rank`, then by `order`, then by `id`.
A view has the rank 0. A draw has the rank 0, and a batch has the rank 1, so in each view the draws come before the batches.
Two items of one `order` keep the order of their entity ids, the same on each run.
`compare_keys` gives -1, 0 or 1, and does not subtract, so a large `order` cannot overflow.

## Functions

| Function | Result |
| --- | --- |
| `bridge_draws` | Adds each draw to the round. Gives 0, or 1 when the memory of the round cannot grow. |
| `bridge_batches` | Adds each batch to the round. Gives 0, or 1 when the memory of the round cannot grow. |
| `bridge_render` | Adds each view, sorts the round, writes it into the round buffer, and encodes its pass. Gives 0, or 1 when the memory cannot grow. |
| `add_items` | Grows the memory of the items of one kind, and gives each item to its visit. |
| `compare_keys` | Orders two sort keys by rank, then order, then entity. |
