/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

constexpr int TRIES = 100;

typedef struct {
    bool answered;
    WGPUAdapter adapter;
} answer_t;

static void take_adapter(WGPURequestAdapterStatus status, WGPUAdapter adapter,
    WGPUStringView message, void *userdata1, [[maybe_unused]] void *userdata2)
{
    answer_t *answer = userdata1;

    answer->answered = true;
    if (status == WGPURequestAdapterStatus_Success) {
        answer->adapter = adapter;
        return;
    }
    printf("no adapter: %.*s\n", (int)message.length, message.data);
}

static int say_adapter(WGPUAdapter adapter)
{
    WGPUAdapterInfo info = {};

    if (wgpuAdapterGetInfo(adapter, &info) != WGPUStatus_Success)
        return (1);
    printf("adapter %u %u\n", (unsigned)info.backendType,
        (unsigned)info.adapterType);
    wgpuAdapterInfoFreeMembers(info);
    return (0);
}

static int ask_adapter(WGPUInstance instance)
{
    answer_t answer = {};
    WGPURequestAdapterCallbackInfo info = {
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = take_adapter, .userdata1 = &answer };
    int status = 1;

    wgpuInstanceRequestAdapter(instance, nullptr, info);
    for (int i = 0; i < TRIES && !answer.answered; i++)
        wgpuInstanceProcessEvents(instance);
    if (answer.adapter) {
        status = say_adapter(answer.adapter);
        wgpuAdapterRelease(answer.adapter);
    }
    return (status);
}

int main(int argc, char **argv)
{
    WGPUInstance instance;
    int status;

    if (argc > 1 && !strcmp(argv[1], "version")) {
        printf("version %08x\n", wgpuGetVersion());
        return (0);
    }
    instance = wgpuCreateInstance(nullptr);
    if (!instance)
        return (1);
    if (argc > 1 && !strcmp(argv[1], "wait")) {
        wgpuInstanceWaitAny(instance, 0, nullptr, 0);
        printf("waited\n");
    }
    status = ask_adapter(instance);
    wgpuInstanceRelease(instance);
    return (status);
}
