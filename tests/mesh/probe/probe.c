/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "probe.h"

[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]]
static const char WHO[] = MANIFEST;

static probe_t probe;

probe_t *probe_get(void)
{
    return (&probe);
}

void probe_say(const bridge_api_t *api, const char *text, uint32_t a0,
    uint32_t a1)
{
    char said[160];
    bridge_line_t line = { said, 0, a0, a1 };

    snprintf(said, sizeof(said), "r%u %s", probe.round, text);
    line.len = (uint32_t)strlen(said);
    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
}

static void declare_frame(const bridge_api_t *api, uint32_t *s)
{
    s[SCENE_SLOT] = BRIDGE_NATIVE_COMPONENT(api, NAMESPACE ":ProbeScene",
        probe_scene_t, BRIDGE_STRIP, BRIDGE_SINGLETON,
        BRIDGE_ON_DESTROY_REMOVE);
    s[MARK] = BRIDGE_NATIVE_COMPONENT(api, NAMESPACE ":ProbeMark",
        probe_mark_t, BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    s[FRAME_STATE] = BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM,
        "webgpu:webgpu_frame_state_t", webgpu_frame_state_t, BRIDGE_STRIP,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    s[FRAME] = BRIDGE_NATIVE_COMPONENT(api, "webgpu:Frame", webgpu_frame_t,
        BRIDGE_STRIP, BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE);
    s[LOAD] = BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "webgpu:webgpu_load_t",
        webgpu_load_t, BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    s[COMPARE] = BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM,
        "webgpu:webgpu_compare_t", webgpu_compare_t, BRIDGE_STRIP,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    s[CULL] = BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "webgpu:webgpu_cull_t",
        webgpu_cull_t, BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    s[SETTINGS] = BRIDGE_NATIVE_COMPONENT(api, "webgpu:MeshSettings",
        webgpu_mesh_settings_t, BRIDGE_STRIP, BRIDGE_SINGLETON,
        BRIDGE_ON_DESTROY_REMOVE);
}

static void declare_items(const bridge_api_t *api, uint32_t *s)
{
    s[LIGHTS] = BRIDGE_NATIVE_COMPONENT(api, "webgpu:Scene", webgpu_scene_t,
        BRIDGE_STRIP, BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE);
    s[VIEWS] = BRIDGE_NATIVE_COMPONENT(api, "webgpu:View", webgpu_view_t,
        BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    s[MESHES] = BRIDGE_NATIVE_COMPONENT(api, "webgpu:Mesh", webgpu_mesh_t,
        BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    s[TEXTURES] = BRIDGE_NATIVE_COMPONENT(api, "webgpu:Texture",
        webgpu_texture_t, BRIDGE_STRIP, BRIDGE_TABLE,
        BRIDGE_ON_DESTROY_REMOVE);
    s[DRAWS] = BRIDGE_NATIVE_COMPONENT(api, "webgpu:Draw", webgpu_draw_t,
        BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    s[BATCHES] = BRIDGE_NATIVE_COMPONENT(api, "webgpu:Batch", webgpu_batch_t,
        BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
}

BRIDGE_NATIVE(init)
{
    uint32_t *s = probe.slots;
    webgpu_frame_t frame = { .state = WEBGPU_FRAME_NO_DEVICE };
    bridge_term_t drive[4] = { { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
        { 0, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 },
        { 0, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 },
        { 0, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 } };
    bridge_term_t check[1] = { { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 } };
    uint32_t changes[8];

    probe = (probe_t){};
    declare_frame(api, s);
    declare_items(api, s);
    drive[0].slot = s[SCENE_SLOT];
    drive[1].slot = s[FRAME];
    drive[2].slot = s[LIGHTS];
    drive[3].slot = s[SETTINGS];
    check[0].slot = s[SCENE_SLOT];
    memcpy(changes, (uint32_t[8]){ s[MARK], s[FRAME], s[LIGHTS], s[VIEWS],
        s[MESHES], s[TEXTURES], s[DRAWS], s[BATCHES] }, sizeof(changes));
    BRIDGE_NATIVE_ADD_SYSTEM(api, "drive", "drive", drive, 4, changes, 8,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "check", "check", check, 1, changes, 8,
        nullptr, 0);
    BRIDGE_NATIVE_ATTACH(api, 0, s[FRAME], frame);
    return (0);
}

BRIDGE_NATIVE(drive)
{
    probe.on = bridge_native_get_single(api, probe.slots[SCENE_SLOT]);
    probe.scene = probe.on ? probe_find_scene(probe.on->scene) : nullptr;
    probe.round++;
    if (!probe.scene)
        return (0);
    probe_drive_frame(api);
    probe_look(api, "drive");
    probe_play(api, false);
    return (0);
}

BRIDGE_NATIVE(check)
{
    if (!probe.scene)
        return (BRIDGE_END);
    probe_play(api, true);
    probe_show(api);
    probe_look(api, "check");
    return (probe.round < probe.scene->rounds ? 0 : BRIDGE_END);
}

BRIDGE_NATIVE(shutdown)
{
    gpu_counts_t c;

    if (probe.opened && probe.scene && probe.scene->counts) {
        gpu_count(&probe.gpu, &c);
        printf("end holds %zu buffers %zu textures %zu views %zu groups "
            "%zu samplers\n", c.buffers, c.textures, c.views, c.groups,
            c.samplers);
        fflush(stdout);
    }
    gpu_close(&probe.gpu);
    return (0);
}

BRIDGE_NATIVE(reload)
{
    return (1);
}
