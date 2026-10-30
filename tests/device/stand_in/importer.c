/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <webgpu/webgpu.h>

#include "bridge_native.h"

[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]]
static const char WHO[] = MANIFEST;

BRIDGE_NATIVE(init)
{
    WGPUInstance instance = wgpuCreateInstance(nullptr);

    bridge_native_say(api, BRIDGE_LOG_INFO, "instance %d", instance != nullptr);
    if (instance)
        wgpuInstanceRelease(instance);
    return (0);
}

BRIDGE_NATIVE(shutdown)
{
    return (0);
}

BRIDGE_NATIVE(reload)
{
    return (1);
}
