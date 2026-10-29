/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "mesh_inside.h"

static const size_t SIZES[2][2] = {
    { sizeof(mesh_view_t), BLOCK },
    { sizeof(mesh_item_t), sizeof(mesh_record_t) }
};

bool mesh_reserve_round(uint32_t part, uint32_t more)
{
    mesh_round_t *r = &mesh_get_state()->round;
    uint32_t need = r->counts[part] + more;
    void *entries = nullptr;
    void *uploads = nullptr;

    if (need <= r->rooms[part])
        return (true);
    entries = realloc(r->entries[part], need * SIZES[part][0]);
    if (!entries)
        return (false);
    r->entries[part] = entries;
    uploads = realloc(r->uploads[part], need * SIZES[part][1]);
    if (!uploads)
        return (false);
    r->uploads[part] = uploads;
    r->rooms[part] = need;
    return (true);
}

static bool grow_round(uint64_t bytes)
{
    mesh_state_t *s = mesh_get_state();
    uint64_t size = 1;

    while (size < bytes)
        size *= 2;
    if (size > s->limits.maxBufferSize)
        mesh_say_left_out(0, RULE_ROUND);
    if (size > s->limits.maxBufferSize
        || !mesh_take_memory(size - s->round_size))
        return (false);
    s->held += s->round_size;
    mesh_release_round();
    s->round_buffer = mesh_make_buffer(WGPUBufferUsage_Uniform
        | WGPUBufferUsage_Vertex, nullptr, size);
    s->round_size = size;
    s->round_group = mesh_make_group(0, &(WGPUBindGroupEntry){
        .buffer = s->round_buffer, .size = sizeof(mesh_block_t) }, 1);
    return (true);
}

void mesh_put_round(void)
{
    mesh_state_t *s = mesh_get_state();
    mesh_round_t *r = &s->round;
    WGPUQueue queue = (WGPUQueue)(uintptr_t)s->frame.queue;
    uint64_t blocks = (uint64_t)r->counts[VIEWS] * BLOCK;
    uint64_t end = blocks + (uint64_t)r->counts[ITEMS] * sizeof(mesh_record_t);
    uint64_t size = end + r->batches;
    const mesh_item_t *it = r->entries[ITEMS];

    if (blocks && size > s->round_size && !grow_round(size))
        r->counts[VIEWS] = 0;
    if (!r->counts[VIEWS])
        return;
    wgpuQueueWriteBuffer(queue, s->round_buffer, 0, r->uploads[VIEWS], blocks);
    if (end > blocks)
        wgpuQueueWriteBuffer(queue, s->round_buffer, blocks, r->uploads[ITEMS],
            end - blocks);
    for (uint32_t i = 0; i < r->counts[ITEMS]; i++)
        if (it[i].data)
            wgpuQueueWriteBuffer(queue, s->round_buffer, end + it[i].offset,
                it[i].data, (uint64_t)it[i].count * STRIDES[it[i].layout]);
}

void mesh_release_round(void)
{
    mesh_state_t *s = mesh_get_state();

    if (s->round_buffer) {
        wgpuBindGroupRelease(s->round_group);
        wgpuBufferRelease(s->round_buffer);
    }
    s->held -= s->round_size;
    s->round_buffer = nullptr;
    s->round_group = nullptr;
    s->round_size = 0;
}

void mesh_free_round(void)
{
    mesh_round_t *r = &mesh_get_state()->round;

    for (uint32_t i = 0; i < 2; i++) {
        free(r->entries[i]);
        free(r->uploads[i]);
    }
    *r = (mesh_round_t){};
}
