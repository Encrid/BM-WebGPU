/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef MESH_INSIDE_H
    #define MESH_INSIDE_H

    #include <webgpu/webgpu.h>

    #include "bridge_native.h"
    #include "webgpu_contract.h"
    #include "webgpu_mesh.h"

    #define WEBGPU_MESH_GPU_FIELDS { uint64_t buffer; uint32_t gen; \
        uint32_t count; uint32_t layout; uint32_t primitive; }
    #define WEBGPU_TEXTURE_GPU_FIELDS { uint64_t texture; uint64_t view; \
        uint64_t group; uint32_t gen; uint32_t bytes; }

BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_mesh_gpu_t, WEBGPU_MESH_GPU_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_mesh_gpu_t, WEBGPU_MESH_GPU_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_texture_gpu_t, WEBGPU_TEXTURE_GPU_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_texture_gpu_t, WEBGPU_TEXTURE_GPU_FIELDS);

constexpr uint32_t LOAD = 0;
constexpr uint32_t COMPARE = 1;
constexpr uint32_t CULL = 2;
constexpr uint32_t SETTINGS = 3;
constexpr uint32_t FRAME_STATE = 4;
constexpr uint32_t FRAME = 5;
constexpr uint32_t SCENE = 6;
constexpr uint32_t VIEW = 7;
constexpr uint32_t MESH = 8;
constexpr uint32_t TEXTURE = 9;
constexpr uint32_t DRAW = 10;
constexpr uint32_t BATCH = 11;
constexpr uint32_t MESH_GPU = 12;
constexpr uint32_t TEXTURE_GPU = 13;

constexpr uint32_t KEEP_MESHES = 0;
constexpr uint32_t KEEP_TEXTURES = 1;
constexpr uint32_t DRAWS = 2;
constexpr uint32_t BATCHES = 3;
constexpr uint32_t RENDER = 4;

constexpr uint32_t VIEWS = 0;
constexpr uint32_t ITEMS = 1;

constexpr uint32_t STRIDES[8] = { 12, 24, 20, 32, 16, 28, 24, 36 };
constexpr uint32_t CHANGE = 24;
constexpr uint32_t BLOCK = 256;
constexpr uint32_t PIPELINES = 160;

typedef enum mesh_shape : uint32_t {
    SHAPE_TRIANGLES = 0,
    SHAPE_LINES,
    SHAPE_POINTS,
    SHAPE_WIDE_LINES,
    SHAPE_WIDE_POINTS
} mesh_shape_t;

typedef enum mesh_rule : uint32_t {
    RULE_OUTSIDE = 0,
    RULE_EMPTY,
    RULE_STRIDE,
    RULE_PIXELS,
    RULE_PRIMITIVE,
    RULE_LAYOUT,
    RULE_SHADING,
    RULE_FILTER,
    RULE_NO_MESH,
    RULE_BIG,
    RULE_WIDE,
    RULE_NO_PART,
    RULE_ROUND
} mesh_rule_t;

typedef struct {
    uint32_t rank;
    int32_t order;
    uint64_t id;
} mesh_key_t;

typedef struct {
    mesh_key_t key;
    uint32_t mask;
    uint32_t rect[4];
    uint32_t block;
} mesh_view_t;

typedef struct {
    mesh_key_t key;
    uint32_t views;
    mesh_shape_t shape;
    uint32_t layout;
    uint32_t depth;
    WGPUBuffer buffer;
    const uint8_t *data;
    uint64_t offset;
    uint32_t count;
    uint32_t record;
    WGPUBindGroup group;
} mesh_item_t;

typedef struct {
    float model[16];
    float color[4];
    float bias;
    float size;
    uint32_t shading;
} mesh_record_t;

typedef struct {
    float matrix[16];
    webgpu_scene_t scene;
    float pad;
    float size[2];
} mesh_block_t;

typedef struct {
    void *entries[2];
    void *uploads[2];
    uint32_t counts[2];
    uint32_t rooms[2];
    uint64_t batches;
} mesh_round_t;

typedef struct {
    WGPURenderPassEncoder pass;
    const webgpu_frame_t *frame;
    uint64_t records;
    uint64_t batches;
    WGPUBuffer buffers[4];
    uint64_t offsets[4];
    WGPURenderPipeline pipeline;
    WGPUBindGroup group;
} mesh_pass_t;

typedef struct {
    const bridge_api_t *api;
    webgpu_frame_t frame;
    webgpu_mesh_settings_t settings;
    WGPULimits limits;
    WGPUShaderModule shader;
    WGPUBindGroupLayout groups[2];
    WGPUPipelineLayout layout;
    WGPUSampler samplers[2];
    WGPUBuffer defaults;
    webgpu_texture_gpu_t white;
    WGPURenderPipeline pipelines[PIPELINES];
    WGPUBuffer round_buffer;
    WGPUBindGroup round_group;
    uint64_t round_size;
    uint64_t held;
    mesh_round_t round;
    webgpu_scene_t scene;
    bool ready;
    bool release;
    bool left[2];
    bool said_room;
    bool said_rule;
} mesh_state_t;

mesh_state_t *mesh_get_state(void);
uint32_t mesh_walk(uint32_t system, uint32_t slot,
    void (*visit)(uint64_t id, const void *value));
void *mesh_find(uint64_t id, uint32_t slot);
const uint8_t *mesh_get_data(const bridge_text_t *text, uint32_t unit,
    mesh_rule_t *rule);
bool mesh_take_change(uint32_t bytes);
void mesh_keep_mesh(uint64_t id, const void *value);
void mesh_keep_texture(uint64_t id, const void *value);
bool mesh_take_memory(uint64_t bytes);
bool mesh_sweep(uint32_t set, bool all);
bool mesh_release_gpu(uint32_t slot, void *value, uint64_t id);
void mesh_add_view(uint64_t id, const void *value);
void mesh_add_draw(uint64_t id, const void *value);
void mesh_add_batch(uint64_t id, const void *value);
void mesh_encode(const webgpu_frame_t *frame);
void mesh_draw_item(mesh_pass_t *pass, const mesh_item_t *item);
void mesh_make_layouts(void);
WGPURenderPipeline mesh_get_pipeline(uint32_t layout, uint32_t shape,
    uint32_t depth);
WGPUVertexState mesh_set_buffers(WGPUVertexBufferLayout *buffers,
    WGPUVertexAttribute *parts, uint32_t layout, uint32_t shape);
void mesh_make_device(const webgpu_frame_t *frame);
WGPUBuffer mesh_make_buffer(WGPUBufferUsage usage, const void *bytes,
    uint64_t size);
WGPUBindGroup mesh_make_group(uint32_t group, const WGPUBindGroupEntry *entries,
    uint32_t count);
webgpu_texture_gpu_t mesh_make_texture(const webgpu_texture_t *texture,
    const uint8_t *pixels);
void mesh_release_device(void);
bool mesh_reserve_round(uint32_t part, uint32_t more);
void mesh_put_round(void);
void mesh_release_round(void);
void mesh_free_round(void);
void mesh_say_no_room(uint64_t held, uint64_t bytes);
void mesh_say_left_out(uint64_t id, mesh_rule_t rule);
void mesh_add_systems(const bridge_api_t *api);

#endif
