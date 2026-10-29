/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static mesh_state_t state;

mesh_state_t *mesh_get_state(void)
{
    return (&state);
}

uint32_t mesh_walk(uint32_t system, uint32_t slot,
    void (*visit)(uint64_t id, const void *value))
{
    const bridge_api_t *api = state.api;
    const uint8_t *p = api->private_base;
    const bridge_header_t *h = bridge_native_get_header(api);
    const bridge_query_t *q = bridge_native_get_query(api, system);
    const uint32_t *col = (const uint32_t *)(p + h->column_base);
    const uint32_t *blocks = (const uint32_t *)(p + h->blocks);
    uint32_t stride = ((const uint32_t *)(p + h->stride))[slot];
    const uint32_t *span = nullptr;
    const uint64_t *ids = nullptr;
    uint32_t n = 0;

    for (uint32_t s = 0; s < q->spans; s++) {
        span = bridge_native_get_span(api, q, s, 1);
        ids = (const uint64_t *)(api->shared_base + blocks[span[0]]);
        for (uint32_t r = 0; visit && r < span[1]; r++)
            visit(ids[r], api->shared_base + col[span[0] * h->types + slot]
                + r * stride);
        n += span[1];
    }
    return (n);
}

void *mesh_find(uint64_t id, uint32_t slot)
{
    const bridge_api_t *api = state.api;
    const bridge_header_t *h = bridge_native_get_header(api);
    const uint32_t *kind = (const uint32_t *)(api->private_base + h->kind);
    const uint32_t *cells = (const uint32_t *)(api->private_base
        + h->column_base);
    const uint32_t *stride = (const uint32_t *)(api->private_base + h->stride);
    const bridge_entity_t *index = (const bridge_entity_t *)(api->shared_base
        + h->entity_index);
    uint32_t e = (uint32_t)id;
    uint32_t at = 0;

    if (e >= h->entities || index[e].generation != (uint32_t)(id >> 32))
        return (nullptr);
    at = cells[index[e].table * h->types + slot];
    if (kind[slot] != BRIDGE_SPARSE_SET)
        return (at ? api->shared_base + at + index[e].row * stride[slot]
            : nullptr);
    at = bridge_native_get_sparse_index(api, slot)[e];
    return (at ? api->shared_base + at : nullptr);
}

const uint8_t *mesh_get_data(const bridge_text_t *text, uint32_t unit,
    mesh_rule_t *rule)
{
    const bridge_header_t *h = bridge_native_get_header(state.api);

    if (text->at < h->arena_lo || text->at > h->arena_hi
        || text->len > h->arena_hi - text->at) {
        *rule = RULE_OUTSIDE;
        return (nullptr);
    }
    if (!text->len || text->len % unit) {
        *rule = text->len ? RULE_STRIDE : RULE_EMPTY;
        return (nullptr);
    }
    return (state.api->shared_base + text->at);
}

bool mesh_take_change(uint32_t bytes)
{
    return (bridge_native_get_header(state.api)->changes_left >= bytes);
}
