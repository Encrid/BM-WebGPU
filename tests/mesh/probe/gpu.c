/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "gpu.h"

constexpr int TRIES = 100;

typedef struct {
    bool answered;
    WGPUAdapter adapter;
    WGPUDevice device;
} answer_t;

WGPUStringView gpu_text(const char *text)
{
    return ((WGPUStringView){ text, strlen(text) });
}

static void say_error([[maybe_unused]] WGPUDevice const *device,
    WGPUErrorType type, WGPUStringView message,
    [[maybe_unused]] void *userdata1, [[maybe_unused]] void *userdata2)
{
    printf("probe error: %u %.*s\n", (unsigned)type, (int)message.length,
        message.data ? message.data : "");
    fflush(stdout);
}

static void lose([[maybe_unused]] WGPUDevice const *device,
    [[maybe_unused]] WGPUDeviceLostReason reason,
    [[maybe_unused]] WGPUStringView message,
    [[maybe_unused]] void *userdata1, [[maybe_unused]] void *userdata2)
{
}

static void take_adapter(WGPURequestAdapterStatus status, WGPUAdapter adapter,
    [[maybe_unused]] WGPUStringView message, void *userdata1,
    [[maybe_unused]] void *userdata2)
{
    answer_t *answer = userdata1;

    answer->answered = true;
    if (status == WGPURequestAdapterStatus_Success)
        answer->adapter = adapter;
}

static void take_device(WGPURequestDeviceStatus status, WGPUDevice device,
    [[maybe_unused]] WGPUStringView message, void *userdata1,
    [[maybe_unused]] void *userdata2)
{
    answer_t *answer = userdata1;

    answer->answered = true;
    if (status == WGPURequestDeviceStatus_Success)
        answer->device = device;
}

static void wait_answer(const gpu_t *gpu, const answer_t *answer)
{
    for (int i = 0; i < TRIES && !answer->answered; i++)
        wgpuInstanceProcessEvents(gpu->instance);
}

static WGPUDevice ask_device(const gpu_t *gpu, bool small)
{
    WGPULimits limits = WGPU_LIMITS_INIT;
    WGPUDeviceDescriptor desc = WGPU_DEVICE_DESCRIPTOR_INIT;
    answer_t answer = {};

    limits.maxTextureDimension2D = SIZE;
    limits.maxBufferSize = ROW * BAND;
    desc.label = gpu_text("probe");
    desc.requiredLimits = small ? &limits : nullptr;
    desc.deviceLostCallbackInfo = (WGPUDeviceLostCallbackInfo){
        .mode = WGPUCallbackMode_AllowSpontaneous, .callback = lose };
    desc.uncapturedErrorCallbackInfo = (WGPUUncapturedErrorCallbackInfo){
        .callback = say_error };
    wgpuAdapterRequestDevice(gpu->adapter, &desc,
        (WGPURequestDeviceCallbackInfo){
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = take_device, .userdata1 = &answer });
    wait_answer(gpu, &answer);
    return (answer.device);
}

bool gpu_open(gpu_t *gpu, bool small)
{
    WGPUInstanceExtras extras = { .chain = { .sType =
        (WGPUSType)WGPUSType_InstanceExtras },
        .backends = WGPUInstanceBackend_Primary };
    WGPUInstanceDescriptor desc = { .nextInChain = &extras.chain };
    WGPURequestAdapterOptions options = { .featureLevel =
        WGPUFeatureLevel_Core };
    answer_t answer = {};

    *gpu = (gpu_t){ .small = small };
    gpu->instance = wgpuCreateInstance(&desc);
    if (!gpu->instance)
        return (false);
    wgpuInstanceRequestAdapter(gpu->instance, &options,
        (WGPURequestAdapterCallbackInfo){
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = take_adapter, .userdata1 = &answer });
    wait_answer(gpu, &answer);
    gpu->adapter = answer.adapter;
    if (gpu->adapter)
        gpu->device = ask_device(gpu, small);
    if (gpu->device)
        gpu->queue = wgpuDeviceGetQueue(gpu->device);
    return (gpu->device != nullptr);
}

bool gpu_renew(gpu_t *gpu, bool small)
{
    WGPUDevice device = ask_device(gpu, small);

    if (!device)
        return (false);
    gpu_drop_frame(gpu);
    wgpuQueueRelease(gpu->queue);
    wgpuDeviceRelease(gpu->device);
    gpu->device = device;
    gpu->queue = wgpuDeviceGetQueue(device);
    return (true);
}

void gpu_close(gpu_t *gpu)
{
    gpu_drop_frame(gpu);
    if (gpu->queue)
        wgpuQueueRelease(gpu->queue);
    if (gpu->device)
        wgpuDeviceRelease(gpu->device);
    if (gpu->adapter)
        wgpuAdapterRelease(gpu->adapter);
    if (gpu->instance)
        wgpuInstanceRelease(gpu->instance);
    *gpu = (gpu_t){};
}
