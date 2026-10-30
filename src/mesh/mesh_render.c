/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "mesh_inside.h"

static int compare_keys(const void *a, const void *b)
{
    const mesh_key_t *x = a;
    const mesh_key_t *y = b;

    if (x->rank != y->rank)
        return ((x->rank > y->rank) - (x->rank < y->rank));
    if (x->order != y->order)
        return ((x->order > y->order) - (x->order < y->order));
    return ((x->id > y->id) - (x->id < y->id));
}

static int add_items(uint32_t system, uint32_t slot,
    void (*visit)(uint64_t id, const void *value))
{
    mesh_state_t *s = mesh_get_state();

    if (!s->ready)
        return (0);
    if (!mesh_reserve_round(ITEMS, mesh_walk(system, slot, nullptr))) {
        s->ready = false;
        return (1);
    }
    mesh_walk(system, slot, visit);
    return (0);
}

BRIDGE_NATIVE(draws)
{
    mesh_get_state()->api = api;
    return (add_items(DRAWS, DRAW, mesh_add_draw));
}

BRIDGE_NATIVE(batches)
{
    mesh_get_state()->api = api;
    return (add_items(BATCHES, BATCH, mesh_add_batch));
}

BRIDGE_NATIVE(render)
{
    mesh_state_t *s = mesh_get_state();
    const webgpu_scene_t *scene = bridge_native_get_single(api, SCENE);

    s->api = api;
    if (!s->ready)
        return (0);
    if (!mesh_reserve_round(VIEWS, mesh_walk(RENDER, VIEW, nullptr)))
        return (1);
    s->scene = scene ? *scene : (webgpu_scene_t){};
    mesh_walk(RENDER, VIEW, mesh_add_view);
    if (s->round.counts[VIEWS])
        qsort(s->round.entries[VIEWS], s->round.counts[VIEWS],
            sizeof(mesh_view_t), compare_keys);
    if (s->round.counts[ITEMS])
        qsort(s->round.entries[ITEMS], s->round.counts[ITEMS],
            sizeof(mesh_item_t), compare_keys);
    mesh_put_round();
    mesh_encode(bridge_native_get_single(api, FRAME));
    return (0);
}
