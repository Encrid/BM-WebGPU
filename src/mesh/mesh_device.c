/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static const uint8_t DEFAULTS[24] = { [20] = 255, 255, 255, 255 };

static const uint8_t WHITE[4] = { 255, 255, 255, 255 };

static const webgpu_texture_t WHITE_TEXTURE = { .width = 1, .height = 1 };

static const WGPUAddressMode WRAPS[2] = { WGPUAddressMode_ClampToEdge,
    WGPUAddressMode_Repeat };

static const WGPUFilterMode FILTERS[2] = { WGPUFilterMode_Nearest,
    WGPUFilterMode_Linear };

static const WGPUSamplerDescriptor NEW_SAMPLER = {
    .label = { "webgpu:mesh:sampler", WGPU_STRLEN },
    .lodMaxClamp = 32,
    .maxAnisotropy = 1
};

static const WGPUTextureDescriptor NEW_TEXTURE = {
    .label = { "webgpu:mesh:texture", WGPU_STRLEN },
    .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
    .format = WGPUTextureFormat_RGBA8Unorm,
    .mipLevelCount = 1,
    .sampleCount = 1
};

static const WGPUTextureViewDescriptor NEW_VIEW = {
    .label = { "webgpu:mesh:view", WGPU_STRLEN },
    .mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED,
    .arrayLayerCount = WGPU_ARRAY_LAYER_COUNT_UNDEFINED
};

void mesh_make_device(const webgpu_frame_t *f)
{
    mesh_state_t *s = mesh_get_state();
    WGPUDevice device = (WGPUDevice)(uintptr_t)f->device;
    WGPUSamplerDescriptor sampler = NEW_SAMPLER;

    s->frame = *f;
    memcpy(&s->settings, bridge_native_get_single(s->api, SETTINGS),
        sizeof(s->settings));
    wgpuDeviceGetLimits(device, &s->limits);
    mesh_make_layouts();
    sampler.addressModeU = WRAPS[s->settings.repeat];
    sampler.addressModeV = WRAPS[s->settings.repeat];
    for (uint32_t i = 0; i < 2; i++) {
        sampler.magFilter = FILTERS[i];
        sampler.minFilter = FILTERS[i];
        s->samplers[i] = wgpuDeviceCreateSampler(device, &sampler);
    }
    s->defaults = mesh_make_buffer(WGPUBufferUsage_Vertex, DEFAULTS,
        sizeof(DEFAULTS));
    s->white = mesh_make_texture(&WHITE_TEXTURE, WHITE);
    s->held += sizeof(DEFAULTS) + sizeof(WHITE);
}

WGPUBuffer mesh_make_buffer(WGPUBufferUsage usage, const void *bytes,
    uint64_t size)
{
    const mesh_state_t *s = mesh_get_state();
    WGPUBuffer buffer = wgpuDeviceCreateBuffer((WGPUDevice)(uintptr_t)
        s->frame.device, &(WGPUBufferDescriptor){ .label = {
        "webgpu:mesh:buffer", WGPU_STRLEN },
        .usage = usage | WGPUBufferUsage_CopyDst, .size = size });

    if (bytes)
        wgpuQueueWriteBuffer((WGPUQueue)(uintptr_t)s->frame.queue, buffer, 0,
            bytes, size);
    return (buffer);
}

WGPUBindGroup mesh_make_group(uint32_t group, const WGPUBindGroupEntry *entries,
    uint32_t count)
{
    const mesh_state_t *s = mesh_get_state();
    WGPUBindGroupDescriptor desc = { .label = { "webgpu:mesh:group",
        WGPU_STRLEN }, .layout = s->groups[group], .entryCount = count,
        .entries = entries };

    return (wgpuDeviceCreateBindGroup((WGPUDevice)(uintptr_t)s->frame.device,
        &desc));
}

webgpu_texture_gpu_t mesh_make_texture(const webgpu_texture_t *t,
    const uint8_t *pixels)
{
    const mesh_state_t *s = mesh_get_state();
    WGPUTextureDescriptor desc = NEW_TEXTURE;
    WGPUBindGroupEntry entries[2] = { { .binding = 0 },
        { .binding = 1, .sampler = s->samplers[t->filter] } };
    WGPUTexture texture = nullptr;

    desc.size = (WGPUExtent3D){ t->width, t->height, 1 };
    texture = wgpuDeviceCreateTexture((WGPUDevice)(uintptr_t)s->frame.device,
        &desc);
    wgpuQueueWriteTexture((WGPUQueue)(uintptr_t)s->frame.queue,
        &(WGPUTexelCopyTextureInfo){ .texture = texture }, pixels,
        4ull * t->width * t->height, &(WGPUTexelCopyBufferLayout){
        .bytesPerRow = 4 * t->width, .rowsPerImage = t->height }, &desc.size);
    entries[0].textureView = wgpuTextureCreateView(texture, &NEW_VIEW);
    return ((webgpu_texture_gpu_t){ (uintptr_t)texture,
        (uintptr_t)entries[0].textureView,
        (uintptr_t)mesh_make_group(1, entries, 2), t->gen,
        4 * t->width * t->height });
}

void mesh_release_device(void)
{
    mesh_state_t *s = mesh_get_state();

    for (uint32_t i = 0; i < PIPELINES; i++)
        if (s->pipelines[i])
            wgpuRenderPipelineRelease(s->pipelines[i]);
    memset(s->pipelines, 0, sizeof(s->pipelines));
    if (s->frame.device) {
        wgpuShaderModuleRelease(s->shader);
        wgpuBindGroupLayoutRelease(s->groups[0]);
        wgpuBindGroupLayoutRelease(s->groups[1]);
        wgpuPipelineLayoutRelease(s->layout);
        wgpuSamplerRelease(s->samplers[0]);
        wgpuSamplerRelease(s->samplers[1]);
        wgpuBufferRelease(s->defaults);
        s->held -= sizeof(DEFAULTS);
    }
    mesh_release_gpu(TEXTURE_GPU, &s->white, 0);
    mesh_release_round();
    s->frame = (webgpu_frame_t){};
}
