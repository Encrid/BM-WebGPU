/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_native.h"
#include "webgpu_contract.h"

#define OTHER_FRAME_FIELDS { webgpu_frame_state_t state; }

BRIDGE_SCHEMA_STRUCT_TYPE(other_frame_t, OTHER_FRAME_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(other_frame_t, OTHER_FRAME_FIELDS);

[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]]
static const char WHO[] = MANIFEST;

BRIDGE_NATIVE(init)
{
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "webgpu:webgpu_frame_state_t",
        webgpu_frame_state_t, BRIDGE_STRIP, BRIDGE_TABLE,
        BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:Frame", other_frame_t, BRIDGE_STRIP,
        BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE);
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
