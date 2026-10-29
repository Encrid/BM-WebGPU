/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

bool mesh_release_gpu(uint32_t slot, void *value, uint64_t id)
{
    mesh_state_t *s = mesh_get_state();
    const webgpu_mesh_gpu_t *mesh = value;
    const webgpu_texture_gpu_t *t = value;
    bool is_mesh = slot == MESH_GPU;
    size_t size = is_mesh ? sizeof(*mesh) : sizeof(*t);
    bool room = !id || mesh_take_change(CHANGE);

    if (is_mesh && mesh->buffer)
        wgpuBufferRelease((WGPUBuffer)(uintptr_t)mesh->buffer);
    if (!is_mesh && t->texture) {
        wgpuBindGroupRelease((WGPUBindGroup)(uintptr_t)t->group);
        wgpuTextureViewRelease((WGPUTextureView)(uintptr_t)t->view);
        wgpuTextureRelease((WGPUTexture)(uintptr_t)t->texture);
    }
    s->held -= is_mesh ? (uint64_t)mesh->count * STRIDES[mesh->layout]
        : t->bytes;
    memset(value, 0, size);
    if (id && room)
        s->api->detach(s->api, id, slot);
    return (room);
}

bool mesh_sweep(uint32_t set, bool all)
{
    const bridge_api_t *api = mesh_get_state()->api;
    const bridge_header_t *h = bridge_native_get_header(api);
    const uint32_t *cells = bridge_native_get_sparse_index(api, set);
    const bridge_entity_t *index = (const bridge_entity_t *)(api->shared_base
        + h->entity_index);
    uint32_t item = set == MESH_GPU ? MESH : TEXTURE;
    uint64_t id = 0;
    bool gone = false;
    bool waits = false;

    for (uint32_t e = 0; e < h->entities; e++) {
        if (!cells[e])
            continue;
        id = ((uint64_t)index[e].generation << 32) | e;
        gone = !mesh_find(id, item);
        if (all || gone)
            waits |= !mesh_release_gpu(set, api->shared_base + cells[e],
                gone ? id : 0);
    }
    return (waits);
}

static int release_list(const bridge_api_t *api, uint32_t set)
{
    mesh_state_t *s = mesh_get_state();
    const bridge_header_t *h = bridge_native_get_header(api);
    const bridge_list_t *list = bridge_native_get_list(api, h->system);
    const bridge_trigger_t *heard = (const bridge_trigger_t *)(api->private_base
        + list->at);
    void *value = nullptr;

    s->api = api;
    for (uint32_t i = 0; i < list->count; i++) {
        value = mesh_find(heard[i].id, set);
        if (value && !mesh_release_gpu(set, value, heard[i].id))
            s->left[set - MESH_GPU] = true;
    }
    return (0);
}

BRIDGE_NATIVE(release_mesh)
{
    return (release_list(api, MESH_GPU));
}

BRIDGE_NATIVE(release_texture)
{
    return (release_list(api, TEXTURE_GPU));
}
