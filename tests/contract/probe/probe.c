/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_native.h"
#include "webgpu_contract.h"

[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]]
static const char WHO[] = MANIFEST;

[[maybe_unused]] static uint32_t declare_surface(const bridge_api_t *api)
{
    return (BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "webgpu:webgpu_surface_t",
        webgpu_surface_t, BRIDGE_STRIP, BRIDGE_TABLE,
        BRIDGE_ON_DESTROY_REMOVE));
}

[[maybe_unused]] static uint32_t declare_frame_state(const bridge_api_t *api)
{
    return (BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM,
        "webgpu:webgpu_frame_state_t", webgpu_frame_state_t, BRIDGE_STRIP,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE));
}

[[maybe_unused]] static uint32_t declare_window(const bridge_api_t *api)
{
    return (BRIDGE_NATIVE_COMPONENT(api, "webgpu:Window", webgpu_window_t,
        BRIDGE_STRIP, BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE));
}

[[maybe_unused]] static uint32_t declare_frame(const bridge_api_t *api)
{
    return (BRIDGE_NATIVE_COMPONENT(api, "webgpu:Frame", webgpu_frame_t,
        BRIDGE_STRIP, BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE));
}

[[maybe_unused]] static void attach_window(const bridge_api_t *api)
{
    webgpu_window_t window = { .surface = WEBGPU_SURFACE_NONE };

    BRIDGE_NATIVE_ATTACH(api, 0, declare_window(api), window);
}

[[maybe_unused]] static void attach_frame(const bridge_api_t *api)
{
    webgpu_frame_t frame = { .state = WEBGPU_FRAME_NO_DEVICE };

    BRIDGE_NATIVE_ATTACH(api, 0, declare_frame(api), frame);
}

BRIDGE_NATIVE(init)
{
#if defined(ROLE_LATE)
    declare_window(api);
    declare_surface(api);
#elif defined(ROLE_APP)
    declare_surface(api);
    attach_window(api);
#elif defined(ROLE_RENDER)
    declare_frame_state(api);
    declare_frame(api);
#elif defined(ROLE_DEVICE)
    declare_surface(api);
    declare_frame_state(api);
    declare_window(api);
    attach_frame(api);
#else
    declare_surface(api);
    declare_frame_state(api);
    declare_window(api);
    declare_frame(api);
#endif
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
