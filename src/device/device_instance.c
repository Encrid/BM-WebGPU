/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "device_inside.h"

static const uint8_t *get_threshold(const uint8_t *percent)
{
    return (*percent > 0 ? percent : nullptr);
}

static size_t get_path_length(const char *path, size_t room)
{
    const char *end = memchr(path, 0, room);

    return (end != nullptr ? (size_t)(end - path) : room);
}

void device_set_instance(device_source_t *src,
    const webgpu_device_settings_t *s)
{
    WGPUInstanceExtras *e = &src->extras;

    e->chain.sType = (WGPUSType)WGPUSType_InstanceExtras;
    e->backends = s->instance_backends;
    e->flags = s->instance_flags;
    e->dx12ShaderCompiler = (WGPUDx12Compiler)(s->d3d12_compiler - 1);
    e->gles3MinorVersion = (WGPUGles3MinorVersion)(s->gles_version - 1);
    e->glFenceBehaviour = (WGPUGLFenceBehaviour)(s->gl_fence - 1);
    e->dxcPath = (WGPUStringView){ s->dxc_path,
        get_path_length(s->dxc_path, sizeof(s->dxc_path)) };
    e->dx12PresentationSystem = (WGPUDx12SwapchainKind)s->d3d12_swapchain;
    e->budgetForDeviceCreation = get_threshold(&s->creation_threshold);
    e->budgetForDeviceLoss = get_threshold(&s->loss_threshold);
    src->instance.nextInChain = &e->chain;
}
