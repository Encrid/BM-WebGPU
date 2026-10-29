/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BM_WEBGPU_DEVICE_H
    #define BM_WEBGPU_DEVICE_H

    #include <stdint.h>

    #include "bridge_schema.h"
    #include "webgpu_contract.h"

    #define WEBGPU_PRESENT_VALUES { WEBGPU_PRESENT_FIFO = 1, \
        WEBGPU_PRESENT_FIFO_RELAXED = 2, WEBGPU_PRESENT_IMMEDIATE = 3, \
        WEBGPU_PRESENT_MAILBOX = 4 }
    #define WEBGPU_FORMAT_VALUES { WEBGPU_FORMAT_PREFERRED = 1, \
        WEBGPU_FORMAT_BGRA8_UNORM = 2, WEBGPU_FORMAT_BGRA8_UNORM_SRGB = 3, \
        WEBGPU_FORMAT_RGBA8_UNORM = 4, WEBGPU_FORMAT_RGBA8_UNORM_SRGB = 5, \
        WEBGPU_FORMAT_RGB10A2_UNORM = 6, WEBGPU_FORMAT_RGBA16_FLOAT = 7 }
    #define WEBGPU_ALPHA_VALUES { WEBGPU_ALPHA_AUTO = 1, \
        WEBGPU_ALPHA_OPAQUE = 2, WEBGPU_ALPHA_PREMULTIPLIED = 3, \
        WEBGPU_ALPHA_UNPREMULTIPLIED = 4, WEBGPU_ALPHA_INHERIT = 5 }
    #define WEBGPU_POWER_VALUES { WEBGPU_POWER_LOW_POWER = 1, \
        WEBGPU_POWER_HIGH_PERFORMANCE = 2 }
    #define WEBGPU_BACKEND_VALUES { WEBGPU_BACKEND_ANY = 1, \
        WEBGPU_BACKEND_D3D12 = 2, WEBGPU_BACKEND_METAL = 3, \
        WEBGPU_BACKEND_VULKAN = 4, WEBGPU_BACKEND_OPENGL = 5, \
        WEBGPU_BACKEND_OPENGLES = 6 }
    #define WEBGPU_FEATURE_LEVEL_VALUES { \
        WEBGPU_FEATURE_LEVEL_COMPATIBILITY = 1, WEBGPU_FEATURE_LEVEL_CORE = 2 }
    #define WEBGPU_SAMPLES_VALUES { WEBGPU_SAMPLES_1 = 1, \
        WEBGPU_SAMPLES_2 = 2, WEBGPU_SAMPLES_4 = 4, WEBGPU_SAMPLES_8 = 8, \
        WEBGPU_SAMPLES_16 = 16 }
    #define WEBGPU_DEPTH_VALUES { WEBGPU_DEPTH_NONE = 1, \
        WEBGPU_DEPTH_16_UNORM = 2, WEBGPU_DEPTH_24_PLUS = 3, \
        WEBGPU_DEPTH_24_PLUS_STENCIL_8 = 4, WEBGPU_DEPTH_32_FLOAT = 5, \
        WEBGPU_DEPTH_32_FLOAT_STENCIL_8 = 6 }
    #define WEBGPU_LIMITS_VALUES { WEBGPU_LIMITS_ADAPTER = 1, \
        WEBGPU_LIMITS_DEFAULT = 2 }
    #define WEBGPU_FEATURES_VALUES { WEBGPU_FEATURES_ADAPTER = 1, \
        WEBGPU_FEATURES_DEFAULT = 2 }
    #define WEBGPU_DEVICE_SETTINGS_FIELDS { webgpu_present_t present; \
        webgpu_format_t format; webgpu_alpha_t alpha; webgpu_power_t power; \
        bool fallback; webgpu_backend_t backend; \
        webgpu_feature_level_t feature_level; webgpu_samples_t samples; \
        webgpu_depth_t depth; webgpu_limits_t limits; \
        webgpu_features_t features; uint32_t error_room; }

BRIDGE_SCHEMA_ENUM_TYPE(webgpu_present_t, uint32_t, WEBGPU_PRESENT_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_present_t, uint32_t, WEBGPU_PRESENT_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_format_t, uint32_t, WEBGPU_FORMAT_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_format_t, uint32_t, WEBGPU_FORMAT_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_alpha_t, uint32_t, WEBGPU_ALPHA_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_alpha_t, uint32_t, WEBGPU_ALPHA_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_power_t, uint32_t, WEBGPU_POWER_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_power_t, uint32_t, WEBGPU_POWER_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_backend_t, uint32_t, WEBGPU_BACKEND_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_backend_t, uint32_t, WEBGPU_BACKEND_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_feature_level_t, uint32_t,
    WEBGPU_FEATURE_LEVEL_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_feature_level_t, uint32_t,
    WEBGPU_FEATURE_LEVEL_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_samples_t, uint32_t, WEBGPU_SAMPLES_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_samples_t, uint32_t, WEBGPU_SAMPLES_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_depth_t, uint32_t, WEBGPU_DEPTH_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_depth_t, uint32_t, WEBGPU_DEPTH_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_limits_t, uint32_t, WEBGPU_LIMITS_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_limits_t, uint32_t, WEBGPU_LIMITS_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(webgpu_features_t, uint32_t, WEBGPU_FEATURES_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(webgpu_features_t, uint32_t, WEBGPU_FEATURES_VALUES);
BRIDGE_SCHEMA_STRUCT_TYPE(webgpu_device_settings_t,
    WEBGPU_DEVICE_SETTINGS_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(webgpu_device_settings_t,
    WEBGPU_DEVICE_SETTINGS_FIELDS);

#endif
