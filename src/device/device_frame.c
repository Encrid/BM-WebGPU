/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "device_inside.h"

static const webgpu_frame_state_t STATES[] = {
    [DEVICE_READY] = WEBGPU_FRAME_READY,
    [DEVICE_MAKING] = WEBGPU_FRAME_SKIPPED,
    [DEVICE_FAILED] = WEBGPU_FRAME_SKIPPED,
    [DEVICE_WAITING] = WEBGPU_FRAME_SKIPPED,
    [DEVICE_NO_FRAME] = WEBGPU_FRAME_SKIPPED,
    [DEVICE_GONE] = WEBGPU_FRAME_NO_DEVICE
};

static const int RESULTS[] = {
    [DEVICE_READY] = 0,
    [DEVICE_MAKING] = 0,
    [DEVICE_FAILED] = 1,
    [DEVICE_WAITING] = 0,
    [DEVICE_NO_FRAME] = 1,
    [DEVICE_GONE] = 0
};

static const bool HAS_DEVICE[] = {
    [DEVICE_READY] = true,
    [DEVICE_MAKING] = false,
    [DEVICE_FAILED] = false,
    [DEVICE_WAITING] = true,
    [DEVICE_NO_FRAME] = true,
    [DEVICE_GONE] = false
};

static bool has_window(const webgpu_window_t *w)
{
    return (w != nullptr && w->surface != WEBGPU_SURFACE_NONE);
}

static device_step_t configure(device_t *d, const webgpu_window_t *w)
{
    gpu_t *g = &d->gpu;
    WGPUSurfaceConfiguration shape = { .device = g->device,
        .format = g->format, .usage = WGPUTextureUsage_RenderAttachment,
        .width = w->width, .height = w->height, .alphaMode = g->alpha,
        .presentMode = g->present };

    g->width = w->width;
    g->height = w->height;
    g->configure = false;
    wgpuSurfaceConfigure(g->surface, &shape);
    return (device_make_targets(d));
}

static device_step_t keep_surface(device_t *d, const webgpu_window_t *w)
{
    const gpu_t *g = &d->gpu;

    if (w->width == 0 || w->height == 0 || w->width > g->max_side
        || w->height > g->max_side)
        return (DEVICE_WAITING);
    if (g->configure || w->width != g->width || w->height != g->height)
        return (configure(d, w));
    return (DEVICE_READY);
}

static void write_frame(webgpu_frame_t *f, const device_t *d,
    device_step_t step)
{
    const gpu_t *g = &d->gpu;

    *f = (webgpu_frame_t){ .state = STATES[step] };
    if (!HAS_DEVICE[step])
        return;
    f->format = g->format;
    f->depth_format = g->depth_format;
    f->samples = g->samples;
    f->width = g->width;
    f->height = g->height;
    f->device = (uintptr_t)g->device;
    f->queue = (uintptr_t)g->queue;
    if (step != DEVICE_READY)
        return;
    f->frame = (uintptr_t)g->frame.view;
    f->target = (uintptr_t)(g->samples > 1 ? g->color.view : g->frame.view);
    f->depth = (uintptr_t)g->depth.view;
}

BRIDGE_NATIVE(frame)
{
    device_t *d = device_get_state();
    const webgpu_window_t *w = bridge_native_get_single(api, WINDOW);
    device_step_t step = DEVICE_GONE;

    d->api = api;
    if (!device_drain_room(d) && has_window(w))
        step = device_make_next(d, bridge_native_get_single(api, SETTINGS), w);
    if (step == DEVICE_READY)
        step = keep_surface(d, w);
    if (step == DEVICE_READY)
        step = device_take_frame(d);
    if (step == DEVICE_GONE) {
        device_drain_room(d);
        device_release(d);
    }
    write_frame(bridge_native_get_single(api, FRAME), d, step);
    return (RESULTS[step]);
}
