/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static bool is_same_device(const webgpu_frame_t *kept,
    const webgpu_frame_t *f)
{
    return (kept->device == f->device && kept->format == f->format
        && kept->samples == f->samples
        && kept->depth_format == f->depth_format);
}

static void open_round(const webgpu_frame_t *f)
{
    mesh_state_t *s = mesh_get_state();
    webgpu_frame_state_t now = f ? f->state : WEBGPU_FRAME_NO_DEVICE;
    bool lost = now == WEBGPU_FRAME_NO_DEVICE;
    bool moved = now == WEBGPU_FRAME_READY && s->frame.device
        && !is_same_device(&s->frame, f);
    bool waits = s->left[0] || s->left[1];

    s->release = moved || (lost && (s->frame.device || waits));
    s->ready = now == WEBGPU_FRAME_READY && !moved;
    s->said_room = false;
    s->said_rule = false;
    s->round.counts[VIEWS] = 0;
    s->round.counts[ITEMS] = 0;
    s->round.batches = 0;
}

static void sweep_set(uint32_t set)
{
    mesh_state_t *s = mesh_get_state();
    bool *left = &s->left[set - MESH_GPU];

    if (s->release)
        *left = mesh_sweep(set, true);
    if (s->ready && *left)
        *left = mesh_sweep(set, false);
}

BRIDGE_NATIVE(keep_meshes)
{
    mesh_state_t *s = mesh_get_state();
    const webgpu_frame_t *f = bridge_native_get_single(api, FRAME);

    s->api = api;
    open_round(f);
    sweep_set(MESH_GPU);
    if (s->release)
        mesh_release_device();
    if (s->ready) {
        if (!s->frame.device)
            mesh_make_device(f);
        mesh_walk(KEEP_MESHES, MESH, mesh_keep_mesh);
    }
    return (0);
}

BRIDGE_NATIVE(keep_textures)
{
    mesh_state_t *s = mesh_get_state();

    s->api = api;
    sweep_set(TEXTURE_GPU);
    if (s->ready)
        mesh_walk(KEEP_TEXTURES, TEXTURE, mesh_keep_texture);
    return (0);
}
