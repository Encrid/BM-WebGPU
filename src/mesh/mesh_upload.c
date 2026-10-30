/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static webgpu_mesh_gpu_t upload_mesh(uint64_t id, const webgpu_mesh_t *m)
{
    const mesh_state_t *s = mesh_get_state();
    mesh_rule_t rule = RULE_BIG;
    const uint8_t *data = nullptr;

    if (m->layout > 7 || m->primitive > 2)
        rule = m->layout > 7 ? RULE_LAYOUT : RULE_PRIMITIVE;
    else
        data = mesh_get_data(&m->data, STRIDES[m->layout], &rule);
    if (data && m->data.len > s->limits.maxBufferSize)
        data = nullptr;
    if (!data)
        mesh_say_left_out(id, rule);
    if (!data || !mesh_take_memory(m->data.len))
        return ((webgpu_mesh_gpu_t){ .gen = m->gen });
    return ((webgpu_mesh_gpu_t){ (uintptr_t)mesh_make_buffer(
        WGPUBufferUsage_Vertex, data, m->data.len), m->gen,
        m->data.len / STRIDES[m->layout], m->layout, m->primitive });
}

static webgpu_texture_gpu_t upload_texture(uint64_t id,
    const webgpu_texture_t *t)
{
    const mesh_state_t *s = mesh_get_state();
    uint32_t side = t->width > t->height ? t->width : t->height;
    mesh_rule_t rule = RULE_FILTER;
    const uint8_t *data = nullptr;
    bool refused = true;

    if (t->filter <= 1)
        data = mesh_get_data(&t->data, 1, &rule);
    if (data && (uint64_t)t->width * t->height * 4 != t->data.len)
        rule = RULE_PIXELS;
    else if (data && side > s->limits.maxTextureDimension2D)
        rule = RULE_WIDE;
    else
        refused = !data;
    if (refused)
        mesh_say_left_out(id, rule);
    if (refused || !mesh_take_memory(t->data.len))
        return ((webgpu_texture_gpu_t){ .gen = t->gen });
    return (mesh_make_texture(t, data));
}

void mesh_keep_mesh(uint64_t id, const void *value)
{
    const webgpu_mesh_t *m = value;
    webgpu_mesh_gpu_t *gpu = mesh_find(id, MESH_GPU);
    const bridge_api_t *api = mesh_get_state()->api;

    if (!gpu && mesh_take_change(CHANGE + sizeof(*gpu)))
        api->attach(api, id, MESH_GPU, BRIDGE_ZEROED);
    if (!gpu || (gpu->buffer && gpu->gen == m->gen))
        return;
    mesh_release_gpu(MESH_GPU, gpu, 0);
    *gpu = upload_mesh(id, m);
}

void mesh_keep_texture(uint64_t id, const void *value)
{
    const webgpu_texture_t *t = value;
    webgpu_texture_gpu_t *gpu = mesh_find(id, TEXTURE_GPU);
    const bridge_api_t *api = mesh_get_state()->api;

    if (!gpu && mesh_take_change(CHANGE + sizeof(*gpu)))
        api->attach(api, id, TEXTURE_GPU, BRIDGE_ZEROED);
    if (!gpu || (gpu->texture && gpu->gen == t->gen))
        return;
    mesh_release_gpu(TEXTURE_GPU, gpu, 0);
    *gpu = upload_texture(id, t);
}

bool mesh_take_memory(uint64_t bytes)
{
    mesh_state_t *s = mesh_get_state();
    const webgpu_mesh_settings_t *set = bridge_native_get_single(s->api,
        SETTINGS);

    if (s->held <= set->gpu_memory && bytes <= set->gpu_memory - s->held) {
        s->held += bytes;
        return (true);
    }
    mesh_say_no_room(s->held, bytes);
    return (false);
}
