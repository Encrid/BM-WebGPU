/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "device_inside.h"

static const WGPUBackendType BACKENDS[] = {
    [WEBGPU_BACKEND_ANY] = WGPUBackendType_Undefined,
    [WEBGPU_BACKEND_D3D12] = WGPUBackendType_D3D12,
    [WEBGPU_BACKEND_METAL] = WGPUBackendType_Metal,
    [WEBGPU_BACKEND_VULKAN] = WGPUBackendType_Vulkan,
    [WEBGPU_BACKEND_OPENGL] = WGPUBackendType_OpenGL,
    [WEBGPU_BACKEND_OPENGLES] = WGPUBackendType_OpenGLES
};

static const WGPUDeviceDescriptor DEVICE_SHAPE = {
    .label = { "webgpu:device", WGPU_STRLEN },
    .defaultQueue.label = { "webgpu:device queue", WGPU_STRLEN },
    .deviceLostCallbackInfo = { .mode = WGPUCallbackMode_AllowSpontaneous,
        .callback = device_keep_lost },
    .uncapturedErrorCallbackInfo.callback = device_keep_error
};

static void take_adapter(WGPURequestAdapterStatus status, WGPUAdapter adapter,
    WGPUStringView message, void *userdata1, [[maybe_unused]] void *userdata2)
{
    device_t *d = userdata1;

    d->gpu.waiting = false;
    if (status == WGPURequestAdapterStatus_Success) {
        d->gpu.adapter = adapter;
        d->gpu.next = NEXT_DEVICE;
        return;
    }
    device_say_no_device(d, "no adapter", &message);
    d->gpu.failed = true;
}

device_step_t device_ask_adapter(device_t *d,
    const webgpu_device_settings_t *s, const webgpu_window_t *w)
{
    WGPURequestAdapterOptions options = {
        .featureLevel = (WGPUFeatureLevel)s->feature_level,
        .powerPreference = (WGPUPowerPreference)s->power,
        .forceFallbackAdapter = s->fallback,
        .backendType = BACKENDS[s->backend] };
    WGPURequestAdapterCallbackInfo answer = {
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = take_adapter, .userdata1 = d };

    if (!device_make_surface(d, w)) {
        d->gpu.failed = true;
        return (DEVICE_MAKING);
    }
    options.compatibleSurface = d->gpu.surface;
    d->gpu.waiting = true;
    wgpuInstanceRequestAdapter(d->gpu.instance, &options, answer);
    return (DEVICE_MAKING);
}

static void take_device(WGPURequestDeviceStatus status, WGPUDevice device,
    WGPUStringView message, void *userdata1, [[maybe_unused]] void *userdata2)
{
    device_t *d = userdata1;

    d->gpu.waiting = false;
    if (status == WGPURequestDeviceStatus_Success) {
        d->gpu.device = device;
        d->gpu.next = NEXT_KEEP;
        return;
    }
    device_say_no_device(d, "the adapter refuses the device", &message);
    d->gpu.failed = true;
}

static void describe_device(device_t *d, const webgpu_device_settings_t *s,
    device_request_t *r)
{
    size_t count = s->features == WEBGPU_FEATURES_ADAPTER ? 0 : r->count;

    for (size_t i = 0; i < r->adapter.featureCount && count < MOST_FEATURES;
        i++) {
        switch ((uint32_t)r->adapter.features[i]) {
            case WGPUNativeFeature_RayQuery:
            case WGPUNativeFeature_CooperativeMatrix:
                continue;
        }
        r->features[count] = r->adapter.features[i];
        count++;
    }
    r->shape = DEVICE_SHAPE;
    r->shape.requiredFeatureCount = count;
    r->shape.requiredFeatures = r->features;
    r->shape.deviceLostCallbackInfo.userdata1 = d;
    r->shape.uncapturedErrorCallbackInfo.userdata1 = d;
    if (s->limits == WEBGPU_LIMITS_ADAPTER)
        r->shape.requiredLimits = &r->limits;
}

device_step_t device_ask_device(device_t *d, const webgpu_device_settings_t *s)
{
    device_request_t r = {};
    WGPURequestDeviceCallbackInfo answer = {
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = take_device, .userdata1 = d };

    if (!device_choose_modes(d, s)) {
        d->gpu.failed = true;
        return (DEVICE_MAKING);
    }
    device_size_room(d, s->error_room);
    r.count = device_choose_targets(d, s, r.features);
    wgpuAdapterGetLimits(d->gpu.adapter, &r.limits);
    if (s->features == WEBGPU_FEATURES_ADAPTER)
        wgpuAdapterGetFeatures(d->gpu.adapter, &r.adapter);
    describe_device(d, s, &r);
    d->gpu.waiting = true;
    wgpuAdapterRequestDevice(d->gpu.adapter, &r.shape, answer);
    wgpuSupportedFeaturesFreeMembers(r.adapter);
    return (DEVICE_MAKING);
}
