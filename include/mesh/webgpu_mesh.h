/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BM_WEBGPU_MESH_H
    #define BM_WEBGPU_MESH_H

    #include <stdint.h>

    #include "bridge_abi.h"
    #include "bridge_schema.h"

    #define WEBGPU_LOAD_VALUES { WEBGPU_LOAD_CLEAR = 1, WEBGPU_LOAD_KEEP = 2 }
    #define WEBGPU_COMPARE_VALUES { WEBGPU_COMPARE_NEVER = 1, \
        WEBGPU_COMPARE_LESS = 2, WEBGPU_COMPARE_EQUAL = 3, \
        WEBGPU_COMPARE_LESS_EQUAL = 4, WEBGPU_COMPARE_GREATER = 5, \
        WEBGPU_COMPARE_NOT_EQUAL = 6, WEBGPU_COMPARE_GREATER_EQUAL = 7, \
        WEBGPU_COMPARE_ALWAYS = 8 }
    #define WEBGPU_CULL_VALUES { WEBGPU_CULL_NONE = 1, WEBGPU_CULL_FRONT = 2, \
        WEBGPU_CULL_BACK = 3 }
    #define WEBGPU_MESH_SETTINGS_FIELDS { webgpu_load_t load; bool blend; \
        float clear_depth; webgpu_compare_t depth_compare; \
        webgpu_cull_t cull; bool clockwise; bool repeat; \
        uint64_t gpu_memory; }
    #define WEBGPU_SCENE_FIELDS { float clear[4]; float light[3]; \
        float light_color[3]; float ambient[3]; }
    #define WEBGPU_VIEW_FIELDS { float matrix[16]; uint32_t rect[4]; \
        uint32_t mask; int32_t order; }
    #define WEBGPU_MESH_FIELDS { bridge_text_t data; uint32_t layout; \
        uint32_t primitive; uint32_t gen; }
    #define WEBGPU_TEXTURE_FIELDS { bridge_text_t data; uint32_t width; \
        uint32_t height; uint32_t gen; uint32_t filter; }
    #define WEBGPU_DRAW_FIELDS { uint64_t mesh; uint64_t texture; \
        float model[16]; float color[4]; uint32_t shading; uint32_t views; \
        int32_t order; uint32_t depth; float bias; float size; }
    #define WEBGPU_BATCH_FIELDS { bridge_text_t data; uint32_t layout; \
        uint32_t primitive; uint32_t views; uint32_t depth; int32_t order; \
        float size; }

BRIDGE_SCHEMA_ENUM_TYPE(webgpu_load_t, uint32_t, WEBGPU_LOAD_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_load_t, uint32_t, WEBGPU_LOAD_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_compare_t, uint32_t, WEBGPU_COMPARE_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_compare_t, uint32_t, WEBGPU_COMPARE_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_cull_t, uint32_t, WEBGPU_CULL_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_cull_t, uint32_t, WEBGPU_CULL_VALUES);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_mesh_settings_t, WEBGPU_MESH_SETTINGS_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_mesh_settings_t, WEBGPU_MESH_SETTINGS_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_scene_t, WEBGPU_SCENE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_scene_t, WEBGPU_SCENE_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_view_t, WEBGPU_VIEW_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_view_t, WEBGPU_VIEW_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_mesh_t, WEBGPU_MESH_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_mesh_t, WEBGPU_MESH_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_texture_t, WEBGPU_TEXTURE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_texture_t, WEBGPU_TEXTURE_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_draw_t, WEBGPU_DRAW_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_draw_t, WEBGPU_DRAW_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_batch_t, WEBGPU_BATCH_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_batch_t, WEBGPU_BATCH_FIELDS);

#endif
