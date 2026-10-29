/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static const char *const RULES[] = {
    "its data lies outside the persistent arena",
    "its data has a length of 0",
    "its length is not a multiple of its stride",
    "its length is not width times height times 4",
    "its primitive is above 2",
    "its layout is above 7",
    "its shading is above 2",
    "its filter is above 1",
    "it names no mesh",
    "its data is larger than a buffer of the device",
    "it is wider or taller than maxTextureDimension2D",
    "it has no part inside the frame",
    "the round is larger than a buffer of the device"
};

void mesh_say_no_room(uint64_t held, uint64_t bytes)
{
    mesh_state_t *s = mesh_get_state();

    if (s->said_room)
        return;
    s->said_room = true;
    bridge_native_say(s->api, BRIDGE_LOG_WARN, "no room: %llu bytes of "
        "gpu_memory are taken, and the item needs %llu more",
        (unsigned long long)held, (unsigned long long)bytes);
}

void mesh_say_left_out(uint64_t id, mesh_rule_t rule)
{
    mesh_state_t *s = mesh_get_state();

    if (s->said_rule)
        return;
    s->said_rule = true;
    bridge_native_say(s->api, BRIDGE_LOG_WARN, "left out %llu: %s",
        (unsigned long long)id, RULES[rule]);
}
