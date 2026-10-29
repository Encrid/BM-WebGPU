/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "device_inside.h"

static const WGPUTextureFormat FORMATS[] = {
    [WEBGPU_FORMAT_PREFERRED] = WGPUTextureFormat_Undefined,
    [WEBGPU_FORMAT_BGRA8_UNORM] = WGPUTextureFormat_BGRA8Unorm,
    [WEBGPU_FORMAT_BGRA8_UNORM_SRGB] = WGPUTextureFormat_BGRA8UnormSrgb,
    [WEBGPU_FORMAT_RGBA8_UNORM] = WGPUTextureFormat_RGBA8Unorm,
    [WEBGPU_FORMAT_RGBA8_UNORM_SRGB] = WGPUTextureFormat_RGBA8UnormSrgb,
    [WEBGPU_FORMAT_RGB10A2_UNORM] = WGPUTextureFormat_RGB10A2Unorm,
    [WEBGPU_FORMAT_RGBA16_FLOAT] = WGPUTextureFormat_RGBA16Float
};

static bool has_item(const uint32_t *items, size_t count, uint32_t value)
{
    for (size_t i = 0; i < count; i++)
        if (items[i] == value)
            return (true);
    return (false);
}

static WGPUTextureFormat choose_format(const device_t *d,
    const webgpu_device_settings_t *s, const WGPUSurfaceCapabilities *caps)
{
    const uint32_t *formats = (const uint32_t *)caps->formats;
    WGPUTextureFormat asked = FORMATS[s->format];

    if (has_item(formats, caps->formatCount, asked))
        return (asked);
    if (s->format != WEBGPU_FORMAT_PREFERRED)
        device_say_fallback(d, FALLBACK_FORMAT, s->format,
            WEBGPU_FORMAT_PREFERRED);
    return (caps->formats[0]);
}

static WGPUPresentMode choose_present(const device_t *d,
    const webgpu_device_settings_t *s, const WGPUSurfaceCapabilities *caps)
{
    const uint32_t *modes = (const uint32_t *)caps->presentModes;

    if (has_item(modes, caps->presentModeCount, s->present))
        return ((WGPUPresentMode)s->present);
    device_say_fallback(d, FALLBACK_PRESENT, s->present, WEBGPU_PRESENT_FIFO);
    return (WGPUPresentMode_Fifo);
}

static WGPUCompositeAlphaMode choose_alpha(const device_t *d,
    const webgpu_device_settings_t *s, const WGPUSurfaceCapabilities *caps)
{
    const uint32_t *modes = (const uint32_t *)caps->alphaModes;
    WGPUCompositeAlphaMode asked = (WGPUCompositeAlphaMode)(s->alpha - 1);

    if (asked == WGPUCompositeAlphaMode_Auto
        || has_item(modes, caps->alphaModeCount, asked))
        return (asked);
    device_say_fallback(d, FALLBACK_ALPHA, s->alpha, WEBGPU_ALPHA_AUTO);
    return (WGPUCompositeAlphaMode_Auto);
}

bool device_choose_modes(device_t *d, const webgpu_device_settings_t *s)
{
    gpu_t *g = &d->gpu;
    WGPUSurfaceCapabilities caps = {};

    wgpuSurfaceGetCapabilities(g->surface, g->adapter, &caps);
    if (caps.formatCount > 0) {
        g->format = choose_format(d, s, &caps);
        g->present = choose_present(d, s, &caps);
        g->alpha = choose_alpha(d, s, &caps);
    } else {
        device_say_no_surface(d, "the adapter gives no format for the window");
    }
    wgpuSurfaceCapabilitiesFreeMembers(caps);
    return (caps.formatCount > 0);
}
