/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "gpu.h"

typedef struct {
    WGPUTexture texture;
    WGPUTextureAspect aspect;
    WGPUBuffer buffer;
    uint32_t bytes;
    uint32_t first;
    uint32_t rows;
} copy_t;

static void take_map([[maybe_unused]] WGPUMapAsyncStatus status,
    [[maybe_unused]] WGPUStringView message, void *userdata1,
    [[maybe_unused]] void *userdata2)
{
    *(bool *)userdata1 = true;
}

static const uint8_t *map_rows(gpu_t *gpu, const copy_t *c)
{
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(gpu->device,
        nullptr);
    WGPUTexelCopyTextureInfo from = { .texture = c->texture,
        .origin = { 0, c->first, 0 }, .aspect = c->aspect };
    WGPUTexelCopyBufferInfo into = { .layout = { .bytesPerRow = ROW,
        .rowsPerImage = c->rows }, .buffer = c->buffer };
    WGPUExtent3D size = { SIZE, c->rows, 1 };
    WGPUCommandBuffer commands;
    bool mapped = false;

    wgpuCommandEncoderCopyTextureToBuffer(encoder, &from, &into, &size);
    commands = wgpuCommandEncoderFinish(encoder, nullptr);
    wgpuQueueSubmit(gpu->queue, 1, &commands);
    wgpuCommandBufferRelease(commands);
    wgpuCommandEncoderRelease(encoder);
    wgpuBufferMapAsync(c->buffer, WGPUMapMode_Read, 0, ROW * c->rows,
        (WGPUBufferMapCallbackInfo){
        .mode = WGPUCallbackMode_AllowSpontaneous, .callback = take_map,
        .userdata1 = &mapped });
    wgpuDevicePoll(gpu->device, true, nullptr);
    return (mapped ? wgpuBufferGetConstMappedRange(c->buffer, 0,
        ROW * c->rows) : nullptr);
}

static void read_rows(gpu_t *gpu, const copy_t *c, uint8_t *out)
{
    const uint8_t *rows = map_rows(gpu, c);

    for (uint32_t y = 0; rows && y < c->rows; y++)
        memcpy(out + (c->first + y) * SIZE * c->bytes, rows + y * ROW,
            SIZE * c->bytes);
    if (rows)
        wgpuBufferUnmap(c->buffer);
}

bool gpu_reads_depth(const gpu_spec_t *spec)
{
    return (spec->samples == 1
        && (spec->depth == WGPUTextureFormat_Depth32Float
        || spec->depth == WGPUTextureFormat_Depth24PlusStencil8));
}

void gpu_read(gpu_t *gpu, gpu_image_t *image)
{
    const gpu_frame_t *f = &gpu->frame;
    bool depth = f->read_depth
        && f->spec.depth == WGPUTextureFormat_Depth32Float;

    *image = (gpu_image_t){};
    for (uint32_t first = 0; first < SIZE; first += BAND)
        read_rows(gpu, &(copy_t){ f->color, WGPUTextureAspect_All, f->read,
            4, first, BAND }, image->color);
    if (depth)
        read_rows(gpu, &(copy_t){ f->depth, WGPUTextureAspect_DepthOnly,
            f->read_depth, 4, 0, SIZE }, (uint8_t *)image->depth);
    if (f->read_depth && !depth)
        read_rows(gpu, &(copy_t){ f->depth, WGPUTextureAspect_StencilOnly,
            f->read_depth, 1, 0, SIZE }, image->stencil);
    for (uint32_t i = 0; f->spec.format == WGPUTextureFormat_BGRA8Unorm
        && i < SIZE * SIZE; i++) {
        uint8_t red = image->color[4 * i];

        image->color[4 * i] = image->color[4 * i + 2];
        image->color[4 * i + 2] = red;
    }
}

void gpu_count(const gpu_t *gpu, gpu_counts_t *counts)
{
    WGPUGlobalReport report = {};
    const WGPUHubReport *hub = &report.hub;

    wgpuGenerateReport(gpu->instance, &report);
    *counts = (gpu_counts_t){
        .buffers = hub->buffers.numAllocated - gpu->own_buffers,
        .textures = hub->textures.numAllocated - gpu->own_textures,
        .views = hub->textureViews.numAllocated - gpu->own_views,
        .groups = hub->bindGroups.numAllocated,
        .samplers = hub->samplers.numAllocated,
        .pipelines = hub->renderPipelines.numAllocated,
        .shaders = hub->shaderModules.numAllocated,
        .layouts = hub->bindGroupLayouts.numAllocated,
        .pipeline_layouts = hub->pipelineLayouts.numAllocated };
}
