/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]]
static const char WHO[] = MANIFEST;

static void declare_settings(const bridge_api_t *api)
{
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "webgpu:webgpu_load_t",
        webgpu_load_t, BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "webgpu:webgpu_compare_t",
        webgpu_compare_t, BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "webgpu:webgpu_cull_t",
        webgpu_cull_t, BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:MeshSettings", webgpu_mesh_settings_t,
        BRIDGE_STRIP, BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "webgpu:webgpu_frame_state_t",
        webgpu_frame_state_t, BRIDGE_STRIP, BRIDGE_TABLE,
        BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:Frame", webgpu_frame_t, BRIDGE_STRIP,
        BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:Scene", webgpu_scene_t, BRIDGE_STRIP,
        BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE);
}

static void declare_tables(const bridge_api_t *api)
{
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:View", webgpu_view_t, BRIDGE_STRIP,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:Mesh", webgpu_mesh_t, BRIDGE_STRIP,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:Texture", webgpu_texture_t,
        BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:Draw", webgpu_draw_t, BRIDGE_STRIP,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:Batch", webgpu_batch_t, BRIDGE_STRIP,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:MeshGpu", webgpu_mesh_gpu_t,
        BRIDGE_STRIP, BRIDGE_SPARSE_SET, BRIDGE_ON_DESTROY_KEEP);
    BRIDGE_NATIVE_COMPONENT(api, "webgpu:TextureGpu", webgpu_texture_gpu_t,
        BRIDGE_STRIP, BRIDGE_SPARSE_SET, BRIDGE_ON_DESTROY_KEEP);
}

BRIDGE_NATIVE(init)
{
    *mesh_get_state() = (mesh_state_t){};
    declare_settings(api);
    declare_tables(api);
    mesh_add_systems(api);
    return (0);
}

BRIDGE_NATIVE(shutdown)
{
    mesh_state_t *s = mesh_get_state();

    s->api = api;
    if (s->frame.device) {
        mesh_sweep(MESH_GPU, true);
        mesh_sweep(TEXTURE_GPU, true);
    }
    mesh_release_device();
    mesh_free_round();
    return (0);
}

BRIDGE_NATIVE(reload)
{
    return (1);
}
