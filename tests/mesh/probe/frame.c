/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "gpu.h"

static void make_texture(gpu_t *gpu, uint32_t format, uint32_t samples,
    WGPUTexture *texture, WGPUTextureView *view)
{
    WGPUTextureDescriptor desc = WGPU_TEXTURE_DESCRIPTOR_INIT;
    WGPUTextureUsage copy = samples > 1 ? WGPUTextureUsage_None
        : WGPUTextureUsage_CopySrc;

    desc.label = gpu_text("probe frame");
    desc.usage = WGPUTextureUsage_RenderAttachment | copy;
    desc.size = (WGPUExtent3D){ SIZE, SIZE, 1 };
    desc.format = (WGPUTextureFormat)format;
    desc.sampleCount = samples;
    *texture = wgpuDeviceCreateTexture(gpu->device, &desc);
    *view = wgpuTextureCreateView(*texture, nullptr);
    gpu->own_textures++;
    gpu->own_views++;
}

void gpu_make_frame(gpu_t *gpu, const gpu_spec_t *spec)
{
    gpu_frame_t *f = &gpu->frame;
    WGPUBufferDescriptor read = { .label = gpu_text("probe read"),
        .usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst,
        .size = ROW * BAND };

    gpu_drop_frame(gpu);
    f->spec = *spec;
    make_texture(gpu, spec->format, 1, &f->color, &f->color_view);
    if (spec->samples > 1)
        make_texture(gpu, spec->format, spec->samples, &f->multi,
            &f->multi_view);
    if (spec->depth)
        make_texture(gpu, spec->depth, spec->samples, &f->depth,
            &f->depth_view);
    f->read = wgpuDeviceCreateBuffer(gpu->device, &read);
    gpu->own_buffers++;
    read.size = ROW * SIZE;
    if (gpu_reads_depth(spec) && !gpu->small)
        f->read_depth = wgpuDeviceCreateBuffer(gpu->device, &read);
    gpu->own_buffers += f->read_depth ? 1 : 0;
}

void gpu_drop_frame(gpu_t *gpu)
{
    gpu_frame_t *f = &gpu->frame;
    WGPUTextureView views[3] = { f->color_view, f->multi_view, f->depth_view };
    WGPUTexture textures[3] = { f->color, f->multi, f->depth };

    for (int i = 0; i < 3; i++) {
        if (views[i])
            wgpuTextureViewRelease(views[i]);
        if (textures[i])
            wgpuTextureRelease(textures[i]);
        gpu->own_views -= views[i] ? 1 : 0;
        gpu->own_textures -= textures[i] ? 1 : 0;
    }
    if (f->read)
        wgpuBufferRelease(f->read);
    if (f->read_depth)
        wgpuBufferRelease(f->read_depth);
    gpu->own_buffers -= (f->read ? 1 : 0) + (f->read_depth ? 1 : 0);
    *f = (gpu_frame_t){};
}

static bool has_stencil(uint32_t format)
{
    return (format == WGPUTextureFormat_Depth24PlusStencil8
        || format == WGPUTextureFormat_Depth32FloatStencil8
        || format == WGPUTextureFormat_Stencil8);
}

void gpu_clear(gpu_t *gpu, const gpu_clear_t *clear)
{
    const gpu_frame_t *f = &gpu->frame;
    WGPURenderPassColorAttachment color = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    WGPURenderPassDepthStencilAttachment depth =
        WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
    WGPURenderPassDescriptor pass = { .colorAttachmentCount = 1,
        .colorAttachments = &color };
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(gpu->device,
        nullptr);
    WGPURenderPassEncoder render;
    WGPUCommandBuffer commands;

    color.view = f->multi_view ? f->multi_view : f->color_view;
    color.resolveTarget = f->multi_view ? f->color_view : nullptr;
    color.loadOp = WGPULoadOp_Clear;
    color.storeOp = WGPUStoreOp_Store;
    color.clearValue = (WGPUColor){ clear->color[0], clear->color[1],
        clear->color[2], clear->color[3] };
    depth.view = f->depth_view;
    depth.depthLoadOp = WGPULoadOp_Clear;
    depth.depthStoreOp = WGPUStoreOp_Store;
    depth.depthClearValue = clear->depth;
    depth.stencilLoadOp = has_stencil(f->spec.depth) ? WGPULoadOp_Clear
        : WGPULoadOp_Undefined;
    depth.stencilStoreOp = has_stencil(f->spec.depth) ? WGPUStoreOp_Store
        : WGPUStoreOp_Undefined;
    depth.stencilClearValue = clear->stencil;
    pass.depthStencilAttachment = f->depth_view ? &depth : nullptr;
    render = wgpuCommandEncoderBeginRenderPass(encoder, &pass);
    wgpuRenderPassEncoderEnd(render);
    commands = wgpuCommandEncoderFinish(encoder, nullptr);
    wgpuQueueSubmit(gpu->queue, 1, &commands);
    wgpuCommandBufferRelease(commands);
    wgpuRenderPassEncoderRelease(render);
    wgpuCommandEncoderRelease(encoder);
}
