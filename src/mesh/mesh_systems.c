/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static const bridge_term_t KEEP_MESH_TERMS[4] = {
    { MESH, BRIDGE_READ, BRIDGE_AND, 0 },
    { MESH_GPU, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 },
    { FRAME, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
    { SETTINGS, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }
};

static const bridge_term_t KEEP_TEXTURE_TERMS[3] = {
    { TEXTURE, BRIDGE_READ, BRIDGE_AND, 0 },
    { TEXTURE_GPU, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 },
    { SETTINGS, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }
};

static const bridge_term_t DRAW_TERMS[3] = {
    { DRAW, BRIDGE_READ, BRIDGE_AND, 0 },
    { MESH_GPU, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
    { TEXTURE_GPU, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }
};

static const bridge_term_t BATCH_TERMS[1] = {
    { BATCH, BRIDGE_READ, BRIDGE_AND, 0 }
};

static const bridge_term_t RENDER_TERMS[4] = {
    { VIEW, BRIDGE_READ, BRIDGE_AND, 0 },
    { FRAME, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
    { SCENE, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
    { SETTINGS, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }
};

static const bridge_term_t RELEASE_MESH_TERMS[2] = {
    { MESH, BRIDGE_READ, BRIDGE_AND, 0 },
    { MESH_GPU, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 }
};

static const bridge_term_t RELEASE_TEXTURE_TERMS[2] = {
    { TEXTURE, BRIDGE_READ, BRIDGE_AND, 0 },
    { TEXTURE_GPU, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 }
};

static const uint32_t GPU_SETS[2] = { MESH_GPU, TEXTURE_GPU };

static const uint32_t CHAIN[4] = { KEEP_MESHES, KEEP_TEXTURES, DRAWS, BATCHES };

void mesh_add_systems(const bridge_api_t *api)
{
    BRIDGE_NATIVE_ADD_SYSTEM(api, "keep_meshes", "render", KEEP_MESH_TERMS, 4,
        &GPU_SETS[0], 1, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "keep_textures", "render",
        KEEP_TEXTURE_TERMS, 3, &GPU_SETS[1], 1, &CHAIN[0], 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "draws", "render", DRAW_TERMS, 3, nullptr,
        0, &CHAIN[1], 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "batches", "render", BATCH_TERMS, 1, nullptr,
        0, &CHAIN[2], 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "render", "render", RENDER_TERMS, 4, nullptr,
        0, &CHAIN[3], 1);
    BRIDGE_NATIVE_OBSERVER(api, "release_mesh", BRIDGE_ON_LEAVE,
        RELEASE_MESH_TERMS, 2, &GPU_SETS[0], 1, nullptr, 0);
    BRIDGE_NATIVE_OBSERVER(api, "release_texture", BRIDGE_ON_LEAVE,
        RELEASE_TEXTURE_TERMS, 2, &GPU_SETS[1], 1, nullptr, 0);
}
