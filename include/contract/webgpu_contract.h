/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BL_WEBGPU_CONTRACT_H
    #define BL_WEBGPU_CONTRACT_H

    #include <stdint.h>

    #include "bridge_schema.h"

    #define WEBGPU_SURFACE_VALUES { WEBGPU_SURFACE_NONE = 1, \
        WEBGPU_SURFACE_WINDOWS = 2, WEBGPU_SURFACE_METAL = 3, \
        WEBGPU_SURFACE_X11 = 4, WEBGPU_SURFACE_WAYLAND = 5 }
    #define WEBGPU_FRAME_STATE_VALUES { WEBGPU_FRAME_READY = 1, \
        WEBGPU_FRAME_SKIPPED = 2, WEBGPU_FRAME_NO_DEVICE = 3 }
    #define WEBGPU_WINDOW_FIELDS { webgpu_surface_t surface; uint32_t width; \
        uint32_t height; uint64_t hinstance; uint64_t hwnd; uint64_t layer; \
        uint64_t x11_display; uint64_t x11_window; uint64_t wl_display; \
        uint64_t wl_surface; }
    #define WEBGPU_FRAME_FIELDS { webgpu_frame_state_t state; \
        uint32_t format; uint32_t depth_format; uint32_t samples; \
        uint32_t width; uint32_t height; uint64_t device; uint64_t queue; \
        uint64_t frame; uint64_t target; uint64_t depth; }

BRIDGE_SCHEMA_ENUM_TYPE(webgpu_surface_t, uint32_t, WEBGPU_SURFACE_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_surface_t, uint32_t, WEBGPU_SURFACE_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_frame_state_t, uint32_t,
    WEBGPU_FRAME_STATE_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_frame_state_t, uint32_t,
    WEBGPU_FRAME_STATE_VALUES);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_window_t, WEBGPU_WINDOW_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_window_t, WEBGPU_WINDOW_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_frame_t, WEBGPU_FRAME_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_frame_t, WEBGPU_FRAME_FIELDS);

#endif
