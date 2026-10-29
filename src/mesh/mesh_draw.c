/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static const mesh_record_t UNLIT = {
    .model = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
    .color = { 1, 1, 1, 1 }
};

static void add_item(const mesh_item_t *item, const mesh_record_t *record)
{
    mesh_round_t *r = &mesh_get_state()->round;
    uint32_t n = r->counts[ITEMS];
    mesh_item_t *at = (mesh_item_t *)r->entries[ITEMS] + n;

    *at = *item;
    at->record = n;
    at->depth = item->depth & 3;
    if (item->shape != SHAPE_TRIANGLES && record->size > 1)
        at->shape = item->shape + 2;
    ((mesh_record_t *)r->uploads[ITEMS])[n] = *record;
    r->counts[ITEMS]++;
}

static WGPUBindGroup get_group(uint64_t texture)
{
    const mesh_state_t *s = mesh_get_state();
    const webgpu_texture_gpu_t *gpu = mesh_find(texture, TEXTURE_GPU);

    if (gpu && gpu->texture)
        return ((WGPUBindGroup)(uintptr_t)gpu->group);
    if (!gpu && mesh_find(texture, TEXTURE))
        return (nullptr);
    return ((WGPUBindGroup)(uintptr_t)s->white.group);
}

void mesh_add_view(uint64_t id, const void *value)
{
    mesh_state_t *s = mesh_get_state();
    const webgpu_view_t *v = value;
    const webgpu_frame_t *f = bridge_native_get_single(s->api, FRAME);
    const uint32_t full[4] = { 0, 0, f->width, f->height };
    const uint32_t *rect = v->rect[2] ? v->rect : full;
    uint32_t n = s->round.counts[VIEWS];
    mesh_block_t *block = (mesh_block_t *)((uint8_t *)s->round.uploads[VIEWS]
        + n * BLOCK);
    bool wide = (rect[2] > rect[3] ? rect[2] : rect[3])
        > s->limits.maxTextureDimension2D;
    bool out = wide || rect[0] >= full[2] || rect[1] >= full[3] || !rect[3];

    *block = (mesh_block_t){ {}, s->scene, 0, { rect[2], rect[3] } };
    memcpy(block->matrix, v->matrix, sizeof(block->matrix));
    ((mesh_view_t *)s->round.entries[VIEWS])[n] = (mesh_view_t){ { 0, v->order,
        id }, v->mask, { rect[0], rect[1], rect[2], rect[3] }, n };
    s->round.counts[VIEWS] += !out;
    if (out)
        mesh_say_left_out(id, wide ? RULE_WIDE : RULE_NO_PART);
}

void mesh_add_draw(uint64_t id, const void *value)
{
    const webgpu_draw_t *d = value;
    const webgpu_mesh_gpu_t *gpu = mesh_find(d->mesh, MESH_GPU);
    WGPUBindGroup group = get_group(d->texture);
    mesh_key_t key = { 0, d->order, id };
    mesh_record_t record = { .bias = d->bias, .size = d->size,
        .shading = d->shading };

    if (d->shading > 2 || !mesh_find(d->mesh, MESH)) {
        mesh_say_left_out(id, d->shading > 2 ? RULE_SHADING : RULE_NO_MESH);
        return;
    }
    if (!gpu || !gpu->buffer || !group)
        return;
    memcpy(record.model, d->model, sizeof(record.model));
    memcpy(record.color, d->color, sizeof(record.color));
    add_item(&(mesh_item_t){ .key = key, .views = d->views,
        .shape = gpu->primitive, .layout = gpu->layout, .depth = d->depth,
        .buffer = (WGPUBuffer)(uintptr_t)gpu->buffer, .count = gpu->count,
        .group = group }, &record);
}

void mesh_add_batch(uint64_t id, const void *value)
{
    mesh_state_t *s = mesh_get_state();
    const webgpu_batch_t *b = value;
    mesh_rule_t rule = b->layout > 7 ? RULE_LAYOUT : RULE_PRIMITIVE;
    const uint8_t *data = nullptr;
    mesh_key_t key = { 1, b->order, id };
    mesh_record_t record = UNLIT;

    if (b->layout <= 7 && b->primitive <= 2)
        data = mesh_get_data(&b->data, STRIDES[b->layout], &rule);
    if (!data) {
        mesh_say_left_out(id, rule);
        return;
    }
    record.size = b->size;
    add_item(&(mesh_item_t){ .key = key, .views = b->views,
        .shape = b->primitive, .layout = b->layout, .depth = b->depth,
        .data = data, .offset = s->round.batches,
        .count = b->data.len / STRIDES[b->layout],
        .group = (WGPUBindGroup)(uintptr_t)s->white.group }, &record);
    s->round.batches += b->data.len;
}
