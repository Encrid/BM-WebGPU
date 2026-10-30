/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "device_inside.h"

constexpr WGPUFeatureName SAMPLES_FEATURE =
    (WGPUFeatureName)WGPUNativeFeature_TextureAdapterSpecificFormatFeatures;

static const WGPUTextureFormat DEPTHS[] = {
    [WEBGPU_DEPTH_NONE] = WGPUTextureFormat_Undefined,
    [WEBGPU_DEPTH_16_UNORM] = WGPUTextureFormat_Depth16Unorm,
    [WEBGPU_DEPTH_24_PLUS] = WGPUTextureFormat_Depth24Plus,
    [WEBGPU_DEPTH_24_PLUS_STENCIL_8] = WGPUTextureFormat_Depth24PlusStencil8,
    [WEBGPU_DEPTH_32_FLOAT] = WGPUTextureFormat_Depth32Float,
    [WEBGPU_DEPTH_32_FLOAT_STENCIL_8] = WGPUTextureFormat_Depth32FloatStencil8
};

size_t device_choose_targets(device_t *d, const webgpu_device_settings_t *s,
    WGPUFeatureName *features)
{
    gpu_t *g = &d->gpu;
    size_t count = 0;

    g->samples = s->samples;
    g->checked = s->samples == WEBGPU_SAMPLES_1
        || s->samples == WEBGPU_SAMPLES_4;
    features[count] = SAMPLES_FEATURE;
    if (!g->checked && wgpuAdapterHasFeature(g->adapter, features[count]))
        count++;
    g->depth_format = DEPTHS[s->depth];
    if (g->depth_format != WGPUTextureFormat_Depth32FloatStencil8)
        return (count);
    features[count] = WGPUFeatureName_Depth32FloatStencil8;
    if (wgpuAdapterHasFeature(g->adapter, features[count]))
        return (count + 1);
    device_say_fallback(d, FALLBACK_DEPTH, s->depth,
        WEBGPU_DEPTH_24_PLUS_STENCIL_8);
    g->depth_format = WGPUTextureFormat_Depth24PlusStencil8;
    return (count);
}

static target_t make_target(const device_t *d, const char *label,
    WGPUTextureFormat format, WGPUTextureUsage usage)
{
    WGPUTextureDescriptor shape = { .label = { label, WGPU_STRLEN },
        .usage = usage, .dimension = WGPUTextureDimension_2D,
        .size = { d->gpu.width, d->gpu.height, 1 }, .format = format,
        .mipLevelCount = 1, .sampleCount = d->gpu.samples };
    WGPUTextureViewDescriptor seen = { .label = { label, WGPU_STRLEN },
        .mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED,
        .arrayLayerCount = WGPU_ARRAY_LAYER_COUNT_UNDEFINED };
    target_t made = {};

    made.texture = wgpuDeviceCreateTexture(d->gpu.device, &shape);
    made.view = wgpuTextureCreateView(made.texture, &seen);
    return (made);
}

static void take_scope([[maybe_unused]] WGPUPopErrorScopeStatus status,
    WGPUErrorType type, [[maybe_unused]] WGPUStringView message,
    void *userdata1, [[maybe_unused]] void *userdata2)
{
    device_t *d = userdata1;

    d->gpu.checked = true;
    d->gpu.waiting = false;
    if (type == WGPUErrorType_NoError)
        return;
    device_say_fallback(d, FALLBACK_SAMPLES, d->gpu.samples,
        WEBGPU_SAMPLES_4);
    d->gpu.samples = WEBGPU_SAMPLES_4;
}

device_step_t device_make_targets(device_t *d)
{
    gpu_t *g = &d->gpu;
    WGPUPopErrorScopeCallbackInfo answer = {
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = take_scope, .userdata1 = d };

    device_release_targets(d);
    if (!g->checked)
        wgpuDevicePushErrorScope(g->device, WGPUErrorFilter_Validation);
    if (g->samples > 1)
        g->color = make_target(d, "webgpu:device color", g->format,
            WGPUTextureUsage_RenderAttachment);
    if (g->depth_format != WGPUTextureFormat_Undefined)
        g->depth = make_target(d, "webgpu:device depth", g->depth_format,
            WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc);
    if (g->checked)
        return (DEVICE_READY);
    g->waiting = true;
    wgpuDevicePopErrorScope(g->device, answer);
    return (DEVICE_MAKING);
}

void device_release_targets(device_t *d)
{
    gpu_t *g = &d->gpu;

    if (g->color.view != nullptr)
        wgpuTextureViewRelease(g->color.view);
    if (g->color.texture != nullptr)
        wgpuTextureRelease(g->color.texture);
    if (g->depth.view != nullptr)
        wgpuTextureViewRelease(g->depth.view);
    if (g->depth.texture != nullptr)
        wgpuTextureRelease(g->depth.texture);
    g->color = (target_t){};
    g->depth = (target_t){};
}
