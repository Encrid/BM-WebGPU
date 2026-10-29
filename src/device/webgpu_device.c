/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "device_inside.h"

[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]]
static const char WHO[] = MANIFEST;

static const char *const ENUMS[][2] = {
    { "webgpu:webgpu_present_t", webgpu_present_t_desc },
    { "webgpu:webgpu_format_t", webgpu_format_t_desc },
    { "webgpu:webgpu_alpha_t", webgpu_alpha_t_desc },
    { "webgpu:webgpu_power_t", webgpu_power_t_desc },
    { "webgpu:webgpu_backend_t", webgpu_backend_t_desc },
    { "webgpu:webgpu_feature_level_t", webgpu_feature_level_t_desc },
    { "webgpu:webgpu_samples_t", webgpu_samples_t_desc },
    { "webgpu:webgpu_depth_t", webgpu_depth_t_desc },
    { "webgpu:webgpu_limits_t", webgpu_limits_t_desc },
    { "webgpu:webgpu_features_t", webgpu_features_t_desc },
    { "webgpu:webgpu_surface_t", webgpu_surface_t_desc },
    { "webgpu:webgpu_frame_state_t", webgpu_frame_state_t_desc }
};

static device_t state;

static void declare_enums(const bridge_api_t *api)
{
    uint32_t name = 0;
    uint32_t desc = 0;

    for (size_t i = 0; i < sizeof(ENUMS) / sizeof(ENUMS[0]); i++) {
        name = (uint32_t)strlen(ENUMS[i][0]);
        desc = (uint32_t)strlen(ENUMS[i][1]);
        api->declare(api, BRIDGE_NATIVE_STATE(api, bridge_declaration_t,
            bridge_native_put(api, ENUMS[i][0], name), name, BRIDGE_ENUM,
            sizeof(uint32_t), alignof(uint32_t),
            bridge_native_put(api, ENUMS[i][1], desc), desc, BRIDGE_STRIP,
            BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE));
    }
}

BRIDGE_NATIVE(init)
{
    webgpu_frame_t none = { .state = WEBGPU_FRAME_NO_DEVICE };
    bridge_term_t frame[3] = { { SETTINGS, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
        { WINDOW, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
        { FRAME, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 } };
    bridge_term_t present[1] = { { FRAME, BRIDGE_READ, BRIDGE_OPTIONAL, 0 } };

    declare_enums(api);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:DeviceSettings",
        webgpu_device_settings_t, BRIDGE_STRIP, BRIDGE_SINGLETON,
        BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:Window", webgpu_window_t,
        BRIDGE_STRIP, BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:Frame", webgpu_frame_t,
        BRIDGE_STRIP, BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_ATTACH(api, 0, FRAME, none);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "frame", "frame", frame, 3, nullptr, 0,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "present", "present", present, 1, nullptr,
        0, nullptr, 0);
    return (0);
}

BRIDGE_NATIVE(shutdown)
{
    state.api = api;
    device_release(&state);
    device_size_room(&state, 0);
    return (0);
}

BRIDGE_NATIVE(reload)
{
    return (1);
}

device_t *device_get_state(void)
{
    return (&state);
}
