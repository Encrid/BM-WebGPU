/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "probe.h"

typedef void (*attach_t)(const bridge_api_t *api, uint32_t i,
    const item_t *it, uint64_t id);

static const float IDENTITY[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 1 };

static const uint8_t CORNERS[3][6][2] = {
    { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 1 }, { 1, 0 }, { 0, 0 } },
    { { 0, 1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 }, { 1, 0 } },
    { { 0, 0 }, { 1, 1 } }
};

static uint32_t get_stride(uint32_t layout)
{
    return (12 + ((layout & 1u) ? 12 : 0) + ((layout & 2u) ? 8 : 0)
        + ((layout & 4u) ? 4 : 0));
}

static uint32_t get_base(const item_t *it)
{
    if (it->verts)
        return (it->vert_count);
    if (it->primitive == 1)
        return (2);
    if (it->primitive == 2)
        return (1);
    return (6);
}

static uint32_t put_parts(uint8_t *out, const item_t *it, const float *pos,
    const float *normal, const float *uv, const uint8_t *rgba)
{
    uint32_t used = 3 * sizeof(float);

    memcpy(out, pos, used);
    if (it->layout & 1u)
        memcpy(out + used, normal, 3 * sizeof(float));
    used += (it->layout & 1u) ? 3 * sizeof(float) : 0;
    if (it->layout & 2u)
        memcpy(out + used, uv, 2 * sizeof(float));
    used += (it->layout & 2u) ? 2 * sizeof(float) : 0;
    if (it->layout & 4u)
        memcpy(out + used, rgba, 4);
    return (used + ((it->layout & 4u) ? 4 : 0));
}

static uint32_t put_vertex(uint8_t *out, const item_t *it, uint32_t k,
    const uint8_t *rgba)
{
    const uint8_t *c = CORNERS[it->clockwise ? 1 : 0][k];
    float pos[3] = { 0, 0, it->z };
    float normal[3] = { it->normal[0], it->normal[1], it->normal[2] };
    float uv[2] = { it->uv[0], it->uv[1] };

    if (it->verts) {
        pos[0] = it->verts[k][0] / HALF - 1;
        pos[1] = 1 - it->verts[k][1] / HALF;
        pos[2] = it->verts[k][2];
        return (put_parts(out, it, pos, normal, uv, rgba));
    }
    if (it->primitive == 1 || it->primitive == 2)
        c = CORNERS[2][k];
    pos[0] = it->at[2 * c[0]] / HALF - 1;
    pos[1] = 1 - it->at[1 + 2 * c[1]] / HALF;
    uv[0] = it->uv[2 * c[0]];
    uv[1] = it->uv[1 + 2 * c[1]];
    normal[2] = it->bend && c[0] ? -normal[2] : normal[2];
    return (put_parts(out, it, pos, normal, uv, rgba));
}

uint32_t probe_put_vertices(uint8_t *out, const item_t *it,
    const uint8_t *rgba)
{
    uint32_t base = get_base(it);
    uint32_t count = it->count > base ? it->count : base;
    uint32_t used = 0;

    for (uint32_t v = 0; v < count; v++)
        used += put_vertex(out + used, it, v % base, rgba);
    return (used);
}

static uint32_t get_bytes(const item_t *it)
{
    uint32_t base = get_base(it);

    if (it->kind == TEXTURE)
        return (it->size[0] * it->size[1] * 4);
    return (get_stride(it->layout) * (it->count > base ? it->count : base));
}

static void write_data(uint8_t *out, const item_t *it, bool altered)
{
    uint32_t count = it->size[0] * it->size[1];

    if (it->kind != TEXTURE) {
        probe_put_vertices(out, it, altered ? it->alter : it->rgba);
        return;
    }
    for (uint32_t i = 0; i < count; i++)
        memcpy(out + 4 * i, altered ? it->alter : it->texels[i < 4 ? i : 0],
            4);
}

static bridge_text_t put_data(const bridge_api_t *api, const item_t *it)
{
    uint32_t bytes = get_bytes(it);
    uint32_t at = api->alloc(api, bytes ? bytes : 8);

    if (!at)
        return ((bridge_text_t){});
    write_data(api->shared_base + at, it, false);
    return ((bridge_text_t){ at, bytes });
}

static bridge_text_t get_text(uint32_t i, const item_t *it)
{
    const probe_t *p = probe_get();
    bridge_text_t text = p->texts[i];
    enum bad bad = it->fix && p->round >= it->fix ? WHOLE : it->bad;

    if (bad == OUTSIDE)
        text.at = 0xFFFFFFF0u;
    if (bad == BELOW)
        text.at = 8;
    if (bad == WRAP)
        text.len = 0xFFFFFFF0u;
    if (bad == CUT)
        text.len -= 1;
    if (bad == SHORT)
        text.len -= 4;
    if (bad == EMPTY)
        text.len = 0;
    return (text);
}

static uint64_t get_ref(uint32_t k)
{
    if (k == PAST)
        return (0x00000001FFFFFFFFull);
    return (k ? probe_get()->ids[k] : 0);
}

static void attach_view(const bridge_api_t *api, [[maybe_unused]] uint32_t i,
    const item_t *it, uint64_t id)
{
    webgpu_view_t v = { .mask = it->mask, .order = it->order };

    memcpy(v.matrix, it->matrix ? it->matrix : IDENTITY, sizeof(v.matrix));
    memcpy(v.rect, it->rect, sizeof(v.rect));
    BRIDGE_NATIVE_ATTACH(api, id, probe_get()->slots[VIEWS], v);
}

static void attach_mesh(const bridge_api_t *api, uint32_t i,
    const item_t *it, uint64_t id)
{
    const probe_t *p = probe_get();
    webgpu_mesh_t m = { .data = get_text(i, it), .layout = it->layout,
        .primitive = p->prims[i], .gen = p->gens[i] };

    BRIDGE_NATIVE_ATTACH(api, id, p->slots[MESHES], m);
}

static void attach_texture(const bridge_api_t *api, uint32_t i,
    const item_t *it, uint64_t id)
{
    const probe_t *p = probe_get();
    webgpu_texture_t t = { .data = get_text(i, it), .width = it->size[0],
        .height = it->size[1], .gen = p->gens[i], .filter = it->filter };

    BRIDGE_NATIVE_ATTACH(api, id, p->slots[TEXTURES], t);
}

static void attach_draw(const bridge_api_t *api, [[maybe_unused]] uint32_t i,
    const item_t *it, uint64_t id)
{
    webgpu_draw_t d = { .mesh = get_ref(it->mesh),
        .texture = get_ref(it->texture), .shading = it->shading,
        .views = it->views, .order = it->order, .depth = it->depth,
        .bias = it->bias, .size = it->line };

    memcpy(d.model, it->matrix ? it->matrix : IDENTITY, sizeof(d.model));
    memcpy(d.color, it->color, sizeof(d.color));
    BRIDGE_NATIVE_ATTACH(api, id, probe_get()->slots[DRAWS], d);
}

static void attach_batch(const bridge_api_t *api, uint32_t i,
    const item_t *it, uint64_t id)
{
    const probe_t *p = probe_get();
    webgpu_batch_t b = { .data = get_text(i, it), .layout = it->layout,
        .primitive = p->prims[i], .views = it->views, .depth = it->depth,
        .order = it->order, .size = it->line };

    BRIDGE_NATIVE_ATTACH(api, id, p->slots[BATCHES], b);
}

static const attach_t ATTACH[KINDS] = { attach_view, attach_mesh,
    attach_texture, attach_draw, attach_batch };

static void spawn_item(const bridge_api_t *api, uint32_t i, const item_t *it)
{
    probe_t *p = probe_get();
    uint32_t copies = it->repeat ? it->repeat : 1;
    probe_mark_t mark = { i };
    char text[64];

    p->gens[i] = 1;
    p->prims[i] = it->primitive;
    p->texts[i] = (bridge_text_t){};
    if (it->kind == MESH || it->kind == TEXTURE || it->kind == BATCH)
        p->texts[i] = put_data(api, it);
    for (uint32_t c = 0; c < copies && c < COPIES; c++) {
        uint64_t id = api->spawn(api);

        p->ids[i] = c == 0 ? id : p->ids[i];
        p->copies[c] = it->repeat ? id : p->copies[c];
        BRIDGE_NATIVE_ATTACH(api, id, p->slots[MARK], mark);
        ATTACH[it->kind](api, i, it, id);
    }
    snprintf(text, sizeof(text), "item %u is %llu", i,
        (unsigned long long)p->ids[i]);
    if (it->show)
        probe_say(api, text, 0, 0);
}

static void destroy_item(const bridge_api_t *api, uint32_t i,
    const item_t *it)
{
    const probe_t *p = probe_get();
    uint32_t copies = it->repeat ? it->repeat : 1;

    for (uint32_t c = 0; c < copies && c < COPIES; c++)
        api->destroy(api, it->repeat ? p->copies[c] : p->ids[i]);
}

static void play_early(const bridge_api_t *api, uint32_t i, const item_t *it)
{
    probe_t *p = probe_get();
    uint32_t now = p->round;

    if (!it->late && (it->from ? it->from : 1) == now)
        spawn_item(api, i, it);
    if (it->kill == now)
        destroy_item(api, i, it);
    if (it->change == now)
        write_data(api->shared_base + p->texts[i].at, it, true);
    p->gens[i] += it->regen == now ? 1 : 0;
    p->prims[i] = it->reprim == now ? it->prim_to : p->prims[i];
    if (it->back == now || it->regen == now || it->fix == now
        || it->reprim == now)
        ATTACH[it->kind](api, i, it, p->ids[i]);
}

static void play_late(const bridge_api_t *api, uint32_t i, const item_t *it)
{
    const probe_t *p = probe_get();
    const uint32_t kinds[KINDS] = { p->slots[VIEWS], p->slots[MESHES],
        p->slots[TEXTURES], p->slots[DRAWS], p->slots[BATCHES] };

    if (it->late && (it->from ? it->from : 1) == p->round)
        spawn_item(api, i, it);
    if (it->end == p->round)
        destroy_item(api, i, it);
    if (it->leave == p->round)
        api->detach(api, p->ids[i], kinds[it->kind]);
}

void probe_play(const bridge_api_t *api, bool late)
{
    const scene_t *s = probe_get()->scene;

    for (uint32_t i = 0; i < s->item_count; i++) {
        if (late)
            play_late(api, i, &s->items[i]);
        else
            play_early(api, i, &s->items[i]);
    }
}
