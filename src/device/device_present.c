/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "device_inside.h"

static device_step_t skip_frame(device_t *d,
    WGPUSurfaceGetCurrentTextureStatus status)
{
    switch ((uint32_t)status) {
        case WGPUSurfaceGetCurrentTextureStatus_Timeout:
        case WGPUSurfaceGetCurrentTextureStatus_Outdated:
            d->gpu.configure = true;
            return (DEVICE_WAITING);
        case WGPUSurfaceGetCurrentTextureStatus_Lost:
            wgpuSurfaceRelease(d->gpu.surface);
            d->gpu.surface = nullptr;
            return (DEVICE_WAITING);
        case WGPUSurfaceGetCurrentTextureStatus_Occluded:
            return (DEVICE_WAITING);
        default:
            device_say_no_surface(d, "the surface gives an error");
            return (DEVICE_NO_FRAME);
    }
}

device_step_t device_take_frame(device_t *d)
{
    gpu_t *g = &d->gpu;
    WGPUSurfaceTexture got = {};
    WGPUTextureViewDescriptor seen = {
        .label = { "webgpu:device frame", WGPU_STRLEN },
        .mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED,
        .arrayLayerCount = WGPU_ARRAY_LAYER_COUNT_UNDEFINED,
        .usage = WGPUTextureUsage_RenderAttachment };

    device_release_frame(d);
    wgpuSurfaceGetCurrentTexture(g->surface, &got);
    g->frame.texture = got.texture;
    if (got.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal
        && got.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
        device_release_frame(d);
        return (skip_frame(d, got.status));
    }
    g->frame.view = wgpuTextureCreateView(got.texture, &seen);
    g->configure = got.status
        == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal;
    return (DEVICE_READY);
}

BRIDGE_NATIVE(present)
{
    device_t *d = device_get_state();
    const webgpu_frame_t *f = bridge_native_get_single(api, FRAME);

    if (f->state == WEBGPU_FRAME_READY)
        wgpuSurfacePresent(d->gpu.surface);
    device_release_frame(d);
    return (0);
}

void device_release_frame(device_t *d)
{
    gpu_t *g = &d->gpu;

    if (g->frame.view != nullptr)
        wgpuTextureViewRelease(g->frame.view);
    if (g->frame.texture != nullptr)
        wgpuTextureRelease(g->frame.texture);
    g->frame = (target_t){};
}

bool device_is_lost(const device_t *d)
{
    WGPUCommandEncoderDescriptor shape = {
        .label = { "webgpu:device check", WGPU_STRLEN } };

    wgpuCommandEncoderRelease(wgpuDeviceCreateCommandEncoder(d->gpu.device,
        &shape));
    return (atomic_load(&d->room.lost));
}
