/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "device_inside.h"

static bool make_instance(gpu_t *g, const webgpu_device_settings_t *s,
    device_source_t *source)
{
    device_set_instance(source, s);
    g->instance = wgpuCreateInstance(&source->instance);
    return (wgpuInstanceEnumerateAdapters(g->instance, nullptr, nullptr) > 0);
}

bool device_make_surface(device_t *d, const webgpu_device_settings_t *s,
    const webgpu_window_t *w)
{
    gpu_t *g = &d->gpu;
    device_source_t source = {};
    WGPUSurfaceDescriptor shape = { .nextInChain = &source.surface.chain,
        .label = { "webgpu:device surface", WGPU_STRLEN } };

    if (!device_set_source(&source, w)) {
        device_say_no_surface(d, "the window has no handle of this system");
        return (false);
    }
    if (g->instance == nullptr && !make_instance(g, s, &source))
        return (true);
    g->surface = wgpuInstanceCreateSurface(g->instance, &shape);
    g->configure = true;
    return (true);
}

static device_step_t keep_device(device_t *d,
    const webgpu_device_settings_t *s, const webgpu_window_t *w)
{
    gpu_t *g = &d->gpu;
    WGPULimits limits = {};

    if (g->queue == nullptr) {
        g->queue = wgpuDeviceGetQueue(g->device);
        wgpuDeviceGetLimits(g->device, &limits);
        g->max_side = limits.maxTextureDimension2D;
        return (DEVICE_MAKING);
    }
    if (!g->checked) {
        g->width = 1;
        g->height = 1;
        return (device_make_targets(d));
    }
    if (g->surface == nullptr && !device_make_surface(d, s, w))
        return (DEVICE_NO_FRAME);
    if (device_is_lost(d))
        return (DEVICE_GONE);
    return (DEVICE_READY);
}

device_step_t device_make_next(device_t *d, const webgpu_device_settings_t *s,
    const webgpu_window_t *w)
{
    device_step_t step = DEVICE_MAKING;

    if (d->gpu.instance != nullptr)
        wgpuInstanceProcessEvents(d->gpu.instance);
    if (!d->gpu.waiting && !d->gpu.failed) {
        switch (d->gpu.next) {
            case NEXT_ADAPTER:
                step = device_ask_adapter(d, s, w);
                break;
            case NEXT_DEVICE:
                step = device_ask_device(d, s);
                break;
            case NEXT_KEEP:
                step = keep_device(d, s, w);
        }
    }
    if (!d->gpu.failed)
        return (step);
    device_release(d);
    return (DEVICE_FAILED);
}

void device_release(device_t *d)
{
    gpu_t *g = &d->gpu;

    device_release_frame(d);
    device_release_targets(d);
    if (g->surface != nullptr)
        wgpuSurfaceRelease(g->surface);
    if (g->queue != nullptr)
        wgpuQueueRelease(g->queue);
    if (g->device != nullptr)
        wgpuDeviceRelease(g->device);
    if (g->adapter != nullptr)
        wgpuAdapterRelease(g->adapter);
    if (g->instance != nullptr)
        wgpuInstanceRelease(g->instance);
    d->gpu = (gpu_t){};
}
