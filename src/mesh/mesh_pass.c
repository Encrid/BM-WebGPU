/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static const WGPUCommandEncoderDescriptor ENCODER = {
    .label = { "webgpu:mesh:encoder", WGPU_STRLEN }
};

static const WGPUCommandBufferDescriptor COMMANDS = {
    .label = { "webgpu:mesh:commands", WGPU_STRLEN }
};

static WGPURenderPassColorAttachment get_color_attachment(
    const webgpu_frame_t *f, WGPULoadOp load)
{
    const float *c = mesh_get_state()->scene.clear;
    WGPUTextureView resolve = f->samples > 1 ? (WGPUTextureView)(uintptr_t)
        f->frame : nullptr;
    WGPURenderPassColorAttachment color = { .view = (WGPUTextureView)
        (uintptr_t)f->target, .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .resolveTarget = resolve, .loadOp = load,
        .storeOp = WGPUStoreOp_Store, .clearValue = { c[0], c[1], c[2],
        c[3] } };

    return (color);
}

static WGPURenderPassDepthStencilAttachment get_depth_attachment(
    const webgpu_frame_t *f, WGPULoadOp load, float clear)
{
    bool stencil = f->depth_format == WGPUTextureFormat_Depth24PlusStencil8
        || f->depth_format == WGPUTextureFormat_Depth32FloatStencil8;
    float low = clear < 0 ? 0 : clear;
    WGPURenderPassDepthStencilAttachment depth = { .view = (WGPUTextureView)
        (uintptr_t)f->depth, .depthLoadOp = load,
        .depthStoreOp = WGPUStoreOp_Store, .depthClearValue = low > 1 ? 1
        : low, .stencilLoadOp = stencil ? load : WGPULoadOp_Undefined,
        .stencilStoreOp = stencil ? WGPUStoreOp_Store
        : WGPUStoreOp_Undefined };

    return (depth);
}

static void draw_view(mesh_pass_t *p, const mesh_view_t *v)
{
    const mesh_state_t *s = mesh_get_state();
    const mesh_item_t *items = s->round.entries[ITEMS];
    uint64_t right = (uint64_t)v->rect[0] + v->rect[2];
    uint64_t bottom = (uint64_t)v->rect[1] + v->rect[3];
    uint64_t width = right < p->frame->width ? right : p->frame->width;
    uint64_t height = bottom < p->frame->height ? bottom : p->frame->height;
    uint32_t offset = v->block * BLOCK;

    wgpuRenderPassEncoderSetViewport(p->pass, v->rect[0], v->rect[1],
        v->rect[2], v->rect[3], 0, 1);
    wgpuRenderPassEncoderSetScissorRect(p->pass, v->rect[0], v->rect[1],
        width - v->rect[0], height - v->rect[1]);
    wgpuRenderPassEncoderSetBindGroup(p->pass, 0, s->round_group, 1, &offset);
    for (uint32_t i = 0; i < s->round.counts[ITEMS]; i++)
        if (items[i].views & v->mask)
            mesh_draw_item(p, &items[i]);
}

static WGPURenderPassEncoder begin_pass(WGPUCommandEncoder encoder,
    const webgpu_frame_t *f)
{
    const mesh_state_t *s = mesh_get_state();
    const webgpu_mesh_settings_t *set = bridge_native_get_single(s->api,
        SETTINGS);
    WGPULoadOp load = set->load == WEBGPU_LOAD_CLEAR ? WGPULoadOp_Clear
        : WGPULoadOp_Load;
    WGPURenderPassColorAttachment color = get_color_attachment(f, load);
    WGPURenderPassDepthStencilAttachment depth = get_depth_attachment(f, load,
        set->clear_depth);
    WGPURenderPassDescriptor desc = { .label = { "webgpu:mesh:pass",
        WGPU_STRLEN }, .colorAttachmentCount = 1, .colorAttachments = &color,
        .depthStencilAttachment = f->depth ? &depth : nullptr };

    return (wgpuCommandEncoderBeginRenderPass(encoder, &desc));
}

void mesh_encode(const webgpu_frame_t *f)
{
    const mesh_state_t *s = mesh_get_state();
    uint64_t records = (uint64_t)s->round.counts[VIEWS] * BLOCK;
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)
        (uintptr_t)f->device, &ENCODER);
    mesh_pass_t pass = { .pass = begin_pass(encoder, f), .frame = f,
        .records = records, .batches = records
        + (uint64_t)s->round.counts[ITEMS] * sizeof(mesh_record_t) };
    const mesh_view_t *views = s->round.entries[VIEWS];
    WGPUCommandBuffer commands = nullptr;

    for (uint32_t i = 0; i < s->round.counts[VIEWS]; i++)
        draw_view(&pass, &views[i]);
    wgpuRenderPassEncoderEnd(pass.pass);
    commands = wgpuCommandEncoderFinish(encoder, &COMMANDS);
    wgpuQueueSubmit((WGPUQueue)(uintptr_t)f->queue, 1, &commands);
    wgpuCommandBufferRelease(commands);
    wgpuRenderPassEncoderRelease(pass.pass);
    wgpuCommandEncoderRelease(encoder);
}
