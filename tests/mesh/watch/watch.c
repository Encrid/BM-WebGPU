/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_native.h"
#include "webgpu_mesh.h"

[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]]
static const char WHO[] = MANIFEST;

BRIDGE_NATIVE(init)
{
    bridge_term_t watch[1] = { { 0, BRIDGE_WRITE, BRIDGE_AND, 0 } };

    watch[0].slot = BRIDGE_NATIVE_COMPONENT(api, WATCHED_NAME, WATCHED,
        BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
#ifdef PHASE
    BRIDGE_NATIVE_ADD_SYSTEM(api, "write", PHASE, watch, 1, nullptr, 0,
        nullptr, 0);
#else
    BRIDGE_NATIVE_OBSERVER(api, "watch", BRIDGE_ON_LEAVE, watch, 1, nullptr,
        0, nullptr, 0);
#endif
    return (0);
}

BRIDGE_NATIVE(watch)
{
    return (0);
}

BRIDGE_NATIVE(write)
{
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
