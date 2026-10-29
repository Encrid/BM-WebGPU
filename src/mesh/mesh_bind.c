/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static void bind_buffer(mesh_pass_t *p, uint32_t i, WGPUBuffer buffer,
    uint64_t offset)
{
    if (p->buffers[i] == buffer && p->offsets[i] == offset)
        return;
    wgpuRenderPassEncoderSetVertexBuffer(p->pass, i, buffer, offset,
        WGPU_WHOLE_SIZE);
    p->buffers[i] = buffer;
    p->offsets[i] = offset;
}

static void bind_states(mesh_pass_t *p, const mesh_item_t *it)
{
    WGPURenderPipeline pipeline = mesh_get_pipeline(it->layout, it->shape,
        it->depth);

    if (pipeline != p->pipeline)
        wgpuRenderPassEncoderSetPipeline(p->pass, pipeline);
    if (it->group != p->group)
        wgpuRenderPassEncoderSetBindGroup(p->pass, 1, it->group, 0, nullptr);
    p->pipeline = pipeline;
    p->group = it->group;
}

static void bind_buffers(mesh_pass_t *p, const mesh_item_t *it)
{
    const mesh_state_t *s = mesh_get_state();
    WGPUBuffer from = it->data ? s->round_buffer : it->buffer;
    uint64_t at = it->data ? p->batches + it->offset : 0;
    uint64_t record = it->shape < SHAPE_WIDE_LINES ? 0 : it->record;
    bool line = it->shape == SHAPE_WIDE_LINES;

    bind_buffer(p, 0, from, at);
    bind_buffer(p, 1, s->round_buffer, p->records + record
        * sizeof(mesh_record_t));
    if (line)
        bind_buffer(p, 2, from, at + STRIDES[it->layout]);
    if (it->layout != 7)
        bind_buffer(p, 2 + line, s->defaults, 0);
}

void mesh_draw_item(mesh_pass_t *p, const mesh_item_t *it)
{
    bool line = it->shape == SHAPE_WIDE_LINES;

    bind_states(p, it);
    bind_buffers(p, it);
    if (it->shape < SHAPE_WIDE_LINES)
        wgpuRenderPassEncoderDraw(p->pass, it->count, 1, 0, it->record);
    else
        wgpuRenderPassEncoderDraw(p->pass, 6, it->count / (1 + line), 0, 0);
}
