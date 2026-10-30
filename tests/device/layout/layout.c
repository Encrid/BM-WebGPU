/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "webgpu_device.h"

#define SIZE(type) printf("%s size %zu alignment %zu\n", #type, \
    sizeof(type), alignof(type))
#define VALUE(constant) printf("%s %u\n", #constant, (unsigned)(constant))
#define FIELD(field) printf("%s %zu\n", #field, \
    sizeof(((webgpu_device_settings_t *)nullptr)->field))
#define TEXT_OF(...) BRIDGE_SCHEMA_TEXT(__VA_ARGS__)

typedef struct {
    const char *name;
    void (*show)(void);
} show_mode_t;

static void show_modes(void)
{
    SIZE(webgpu_present_t);
    VALUE(WEBGPU_PRESENT_FIFO);
    VALUE(WEBGPU_PRESENT_FIFO_RELAXED);
    VALUE(WEBGPU_PRESENT_IMMEDIATE);
    VALUE(WEBGPU_PRESENT_MAILBOX);
    SIZE(webgpu_format_t);
    VALUE(WEBGPU_FORMAT_PREFERRED);
    VALUE(WEBGPU_FORMAT_BGRA8_UNORM);
    VALUE(WEBGPU_FORMAT_BGRA8_UNORM_SRGB);
    VALUE(WEBGPU_FORMAT_RGBA8_UNORM);
    VALUE(WEBGPU_FORMAT_RGBA8_UNORM_SRGB);
    VALUE(WEBGPU_FORMAT_RGB10A2_UNORM);
    VALUE(WEBGPU_FORMAT_RGBA16_FLOAT);
    SIZE(webgpu_alpha_t);
    VALUE(WEBGPU_ALPHA_AUTO);
    VALUE(WEBGPU_ALPHA_OPAQUE);
    VALUE(WEBGPU_ALPHA_PREMULTIPLIED);
    VALUE(WEBGPU_ALPHA_UNPREMULTIPLIED);
    VALUE(WEBGPU_ALPHA_INHERIT);
}

static void show_adapter(void)
{
    SIZE(webgpu_power_t);
    VALUE(WEBGPU_POWER_LOW_POWER);
    VALUE(WEBGPU_POWER_HIGH_PERFORMANCE);
    SIZE(webgpu_backend_t);
    VALUE(WEBGPU_BACKEND_ANY);
    VALUE(WEBGPU_BACKEND_D3D12);
    VALUE(WEBGPU_BACKEND_METAL);
    VALUE(WEBGPU_BACKEND_VULKAN);
    VALUE(WEBGPU_BACKEND_OPENGL);
    VALUE(WEBGPU_BACKEND_OPENGLES);
    SIZE(webgpu_feature_level_t);
    VALUE(WEBGPU_FEATURE_LEVEL_COMPATIBILITY);
    VALUE(WEBGPU_FEATURE_LEVEL_CORE);
}

static void show_targets(void)
{
    SIZE(webgpu_samples_t);
    VALUE(WEBGPU_SAMPLES_1);
    VALUE(WEBGPU_SAMPLES_2);
    VALUE(WEBGPU_SAMPLES_4);
    VALUE(WEBGPU_SAMPLES_8);
    VALUE(WEBGPU_SAMPLES_16);
    SIZE(webgpu_depth_t);
    VALUE(WEBGPU_DEPTH_NONE);
    VALUE(WEBGPU_DEPTH_16_UNORM);
    VALUE(WEBGPU_DEPTH_24_PLUS);
    VALUE(WEBGPU_DEPTH_24_PLUS_STENCIL_8);
    VALUE(WEBGPU_DEPTH_32_FLOAT);
    VALUE(WEBGPU_DEPTH_32_FLOAT_STENCIL_8);
}

static void show_device(void)
{
    SIZE(webgpu_limits_t);
    VALUE(WEBGPU_LIMITS_ADAPTER);
    VALUE(WEBGPU_LIMITS_DEFAULT);
    SIZE(webgpu_features_t);
    VALUE(WEBGPU_FEATURES_ADAPTER);
    VALUE(WEBGPU_FEATURES_DEFAULT);
}

static void show_instance(void)
{
    SIZE(webgpu_instance_backends_t);
    VALUE(WEBGPU_INSTANCE_BACKEND_ALL);
    VALUE(WEBGPU_INSTANCE_BACKEND_VULKAN);
    VALUE(WEBGPU_INSTANCE_BACKEND_GL);
    VALUE(WEBGPU_INSTANCE_BACKEND_METAL);
    VALUE(WEBGPU_INSTANCE_BACKEND_D3D12);
    SIZE(webgpu_instance_flags_t);
    VALUE(WEBGPU_INSTANCE_FLAG_EMPTY);
    VALUE(WEBGPU_INSTANCE_FLAG_DEBUG);
    VALUE(WEBGPU_INSTANCE_FLAG_VALIDATION);
    VALUE(WEBGPU_INSTANCE_FLAG_DISCARD_HAL_LABELS);
    VALUE(WEBGPU_INSTANCE_FLAG_ALLOW_UNDERLYING_NONCOMPLIANT_ADAPTER);
    VALUE(WEBGPU_INSTANCE_FLAG_GPU_BASED_VALIDATION);
    VALUE(WEBGPU_INSTANCE_FLAG_VALIDATION_INDIRECT_CALL);
    VALUE(WEBGPU_INSTANCE_FLAG_AUTOMATIC_TIMESTAMP_NORMALIZATION);
    VALUE(WEBGPU_INSTANCE_FLAG_DEFAULT);
    VALUE(WEBGPU_INSTANCE_FLAG_DEBUGGING);
    VALUE(WEBGPU_INSTANCE_FLAG_ADVANCED_DEBUGGING);
    VALUE(WEBGPU_INSTANCE_FLAG_WITH_ENV);
}

static void show_backends(void)
{
    SIZE(webgpu_d3d12_compiler_t);
    VALUE(WEBGPU_D3D12_COMPILER_AUTO);
    VALUE(WEBGPU_D3D12_COMPILER_FXC);
    VALUE(WEBGPU_D3D12_COMPILER_DXC);
    SIZE(webgpu_d3d12_swapchain_t);
    VALUE(WEBGPU_D3D12_SWAPCHAIN_HWND);
    VALUE(WEBGPU_D3D12_SWAPCHAIN_VISUAL);
    SIZE(webgpu_gles_version_t);
    VALUE(WEBGPU_GLES_VERSION_AUTO);
    VALUE(WEBGPU_GLES_VERSION_3_0);
    VALUE(WEBGPU_GLES_VERSION_3_1);
    VALUE(WEBGPU_GLES_VERSION_3_2);
    SIZE(webgpu_gl_fence_t);
    VALUE(WEBGPU_GL_FENCE_NORMAL);
    VALUE(WEBGPU_GL_FENCE_AUTO_FINISH);
}

static void show_settings(void)
{
    FIELD(present);
    FIELD(format);
    FIELD(alpha);
    FIELD(power);
    FIELD(fallback);
    FIELD(backend);
    FIELD(feature_level);
    FIELD(samples);
    FIELD(depth);
    FIELD(limits);
    FIELD(features);
    FIELD(error_room);
    FIELD(instance_backends);
    FIELD(instance_flags);
    FIELD(d3d12_compiler);
    FIELD(d3d12_swapchain);
    FIELD(gles_version);
    FIELD(gl_fence);
    FIELD(creation_threshold);
    FIELD(loss_threshold);
    FIELD(dxc_path);
}

static void show_descriptors(void)
{
    printf("%s\n%s\n%s\n%s\n%s\n%s\n", webgpu_present_t_desc,
        webgpu_format_t_desc, webgpu_alpha_t_desc, webgpu_power_t_desc,
        webgpu_backend_t_desc, webgpu_feature_level_t_desc);
    printf("%s\n%s\n%s\n%s\n", webgpu_samples_t_desc,
        webgpu_depth_t_desc, webgpu_limits_t_desc, webgpu_features_t_desc);
    printf("%s\n%s\n%s\n%s\n%s\n%s\n", webgpu_instance_backends_t_desc,
        webgpu_instance_flags_t_desc, webgpu_d3d12_compiler_t_desc,
        webgpu_d3d12_swapchain_t_desc, webgpu_gles_version_t_desc,
        webgpu_gl_fence_t_desc);
    printf("%s\n", webgpu_device_settings_t_desc);
}

static void show_texts(void)
{
    printf("%s\n%s\n%s\n%s\n%s\n%s\n", TEXT_OF(WEBGPU_PRESENT_VALUES),
        TEXT_OF(WEBGPU_FORMAT_VALUES), TEXT_OF(WEBGPU_ALPHA_VALUES),
        TEXT_OF(WEBGPU_POWER_VALUES), TEXT_OF(WEBGPU_BACKEND_VALUES),
        TEXT_OF(WEBGPU_FEATURE_LEVEL_VALUES));
    printf("%s\n%s\n%s\n%s\n", TEXT_OF(WEBGPU_SAMPLES_VALUES),
        TEXT_OF(WEBGPU_DEPTH_VALUES), TEXT_OF(WEBGPU_LIMITS_VALUES),
        TEXT_OF(WEBGPU_FEATURES_VALUES));
    printf("%s\n%s\n%s\n%s\n%s\n%s\n",
        TEXT_OF(WEBGPU_INSTANCE_BACKENDS_VALUES),
        TEXT_OF(WEBGPU_INSTANCE_FLAGS_VALUES),
        TEXT_OF(WEBGPU_D3D12_COMPILER_VALUES),
        TEXT_OF(WEBGPU_D3D12_SWAPCHAIN_VALUES),
        TEXT_OF(WEBGPU_GLES_VERSION_VALUES), TEXT_OF(WEBGPU_GL_FENCE_VALUES));
    printf("%s\n", TEXT_OF(WEBGPU_DEVICE_SETTINGS_FIELDS));
}

int main(int argc, char **argv)
{
    static const show_mode_t MODES[] = {
        { "modes", show_modes }, { "adapter", show_adapter },
        { "targets", show_targets }, { "device", show_device },
        { "instance", show_instance }, { "backends", show_backends },
        { "settings", show_settings }, { "descriptors", show_descriptors },
        { "texts", show_texts }
    };

    for (size_t i = 0; argc > 1 && i < sizeof(MODES) / sizeof(MODES[0]); i++)
        if (!strcmp(argv[1], MODES[i].name)) {
            MODES[i].show();
            return (0);
        }
    printf("usage: layout <mode>\n");
    return (2);
}
