/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef PROBE_H
    #define PROBE_H

    #include "bridge_native.h"
    #include "gpu.h"
    #include "webgpu_contract.h"
    #include "webgpu_mesh.h"

    #ifdef APP
        #define NAMESPACE "test"
    #else
        #define NAMESPACE "webgpu"
    #endif

    #define SCENE_FIELDS { char scene[32]; char module[512]; }
    #define MARK_FIELDS { uint32_t item; }
    #define COUNT(items) ((uint32_t)(sizeof(items) / sizeof((items)[0])))

BRIDGE_SCHEMA_STRUCT_TYPE(probe_scene_t, SCENE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(probe_scene_t, SCENE_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(probe_mark_t, MARK_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(probe_mark_t, MARK_FIELDS);

constexpr uint32_t ITEMS = 24;
constexpr uint32_t COPIES = 320;
constexpr uint32_t PAST = 1000;
constexpr float HALF = 32.0f;

enum kind : uint32_t { VIEW = 0, MESH, TEXTURE, DRAW, BATCH, KINDS };

enum bad : uint32_t { WHOLE = 0, OUTSIDE, BELOW, WRAP, CUT, EMPTY, SHORT };

enum slot : uint32_t {
    SCENE_SLOT = 0, MARK, FRAME_STATE, FRAME, LOAD, COMPARE, CULL, SETTINGS,
    LIGHTS, VIEWS, MESHES, TEXTURES, DRAWS, BATCHES, SLOTS
};

enum act : uint32_t {
    STATE = 0, GONE, BACK, DEVICE, FORMAT, SAMPLES, DEPTH, LOAD_TO,
    BLEND_TO, CLEAR_DEPTH_TO, COMPARE_TO, CULL_TO, CLOCKWISE_TO, REPEAT_TO,
    GPU_MEMORY_TO, RELOAD, LIE
};

typedef struct {
    enum kind kind;
    float at[4];
    float z;
    const float (*verts)[3];
    uint32_t vert_count;
    uint32_t count;
    float normal[3];
    float uv[4];
    uint8_t rgba[4];
    uint8_t alter[4];
    uint32_t layout;
    uint32_t primitive;
    bool clockwise;
    bool bend;
    enum bad bad;
    uint32_t from;
    bool late;
    uint32_t kill;
    uint32_t end;
    uint32_t leave;
    uint32_t back;
    uint32_t change;
    uint32_t regen;
    uint32_t fix;
    uint32_t reprim;
    uint32_t prim_to;
    uint32_t repeat;
    bool look;
    bool show;
    uint32_t size[2];
    uint8_t texels[4][4];
    uint32_t filter;
    uint32_t mesh;
    uint32_t texture;
    const float *matrix;
    float color[4];
    uint32_t shading;
    uint32_t views;
    uint32_t depth;
    int32_t order;
    float bias;
    float line;
    uint32_t rect[4];
    uint32_t mask;
} item_t;

typedef struct {
    const char *name;
    uint32_t rounds;
    gpu_spec_t spec;
    bool small;
    bool dark;
    bool none;
    bool counts;
    bool prefill;
    float prefill_depth;
    uint32_t prefill_stencil;
    const item_t *items;
    uint32_t item_count;
    uint32_t points[4][2];
    uint32_t point_count;
} scene_t;

typedef struct {
    const char *scene;
    uint32_t round;
    enum act act;
    int64_t value;
} step_t;

typedef struct {
    uint32_t round;
    const scene_t *scene;
    const probe_scene_t *on;
    uint32_t slots[SLOTS];
    gpu_t gpu;
    bool opened;
    bool gone;
    webgpu_frame_state_t state;
    uint32_t lie;
    uint64_t ids[ITEMS];
    uint64_t copies[COPIES];
    bridge_text_t texts[ITEMS];
    uint32_t gens[ITEMS];
    uint32_t prims[ITEMS];
} probe_t;

probe_t *probe_get(void);
const scene_t *probe_find_scene(const char *name);
const step_t *probe_get_steps(uint32_t *count);
void probe_say(const bridge_api_t *api, const char *text, uint32_t a0,
    uint32_t a1);
void probe_play(const bridge_api_t *api, bool late);
void probe_look(const bridge_api_t *api, const char *phase);
void probe_show(const bridge_api_t *api);
void probe_drive_frame(const bridge_api_t *api);
int32_t probe_reload(const char *path);
uint32_t probe_put_vertices(uint8_t *out, const item_t *it,
    const uint8_t *rgba);

#endif
