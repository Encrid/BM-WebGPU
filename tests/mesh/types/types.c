/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <webgpu_mesh.h>

#define SIZE(type) printf("size %zu\nalignment %zu\n", sizeof(type), \
    alignof(type))
#define AT(type, field) printf("%s %zu\n", #field, offsetof(type, field))
#define VALUE(constant) printf("%s %u\n", #constant, (unsigned)(constant))

typedef struct {
    const char *name;
    void (*show)(void);
} show_mode_t;

static void show_settings(void)
{
    SIZE(webgpu_mesh_settings_t);
    AT(webgpu_mesh_settings_t, load);
    AT(webgpu_mesh_settings_t, blend);
    AT(webgpu_mesh_settings_t, clear_depth);
    AT(webgpu_mesh_settings_t, depth_compare);
    AT(webgpu_mesh_settings_t, cull);
    AT(webgpu_mesh_settings_t, clockwise);
    AT(webgpu_mesh_settings_t, repeat);
    AT(webgpu_mesh_settings_t, gpu_memory);
}

static void show_scene(void)
{
    SIZE(webgpu_scene_t);
    AT(webgpu_scene_t, clear);
    AT(webgpu_scene_t, light);
    AT(webgpu_scene_t, light_color);
    AT(webgpu_scene_t, ambient);
}

static void show_view(void)
{
    SIZE(webgpu_view_t);
    AT(webgpu_view_t, matrix);
    AT(webgpu_view_t, rect);
    AT(webgpu_view_t, mask);
    AT(webgpu_view_t, order);
}

static void show_mesh(void)
{
    SIZE(webgpu_mesh_t);
    AT(webgpu_mesh_t, data);
    AT(webgpu_mesh_t, layout);
    AT(webgpu_mesh_t, primitive);
    AT(webgpu_mesh_t, gen);
}

static void show_texture(void)
{
    SIZE(webgpu_texture_t);
    AT(webgpu_texture_t, data);
    AT(webgpu_texture_t, width);
    AT(webgpu_texture_t, height);
    AT(webgpu_texture_t, gen);
    AT(webgpu_texture_t, filter);
}

static void show_draw(void)
{
    SIZE(webgpu_draw_t);
    AT(webgpu_draw_t, mesh);
    AT(webgpu_draw_t, texture);
    AT(webgpu_draw_t, model);
    AT(webgpu_draw_t, color);
    AT(webgpu_draw_t, shading);
    AT(webgpu_draw_t, views);
    AT(webgpu_draw_t, order);
    AT(webgpu_draw_t, depth);
    AT(webgpu_draw_t, bias);
    AT(webgpu_draw_t, size);
}

static void show_batch(void)
{
    SIZE(webgpu_batch_t);
    AT(webgpu_batch_t, data);
    AT(webgpu_batch_t, layout);
    AT(webgpu_batch_t, primitive);
    AT(webgpu_batch_t, views);
    AT(webgpu_batch_t, depth);
    AT(webgpu_batch_t, order);
    AT(webgpu_batch_t, size);
}

static void show_load(void)
{
    SIZE(webgpu_load_t);
    VALUE(WEBGPU_LOAD_CLEAR);
    VALUE(WEBGPU_LOAD_KEEP);
}

static void show_compare(void)
{
    SIZE(webgpu_compare_t);
    VALUE(WEBGPU_COMPARE_NEVER);
    VALUE(WEBGPU_COMPARE_LESS);
    VALUE(WEBGPU_COMPARE_EQUAL);
    VALUE(WEBGPU_COMPARE_LESS_EQUAL);
    VALUE(WEBGPU_COMPARE_GREATER);
    VALUE(WEBGPU_COMPARE_NOT_EQUAL);
    VALUE(WEBGPU_COMPARE_GREATER_EQUAL);
    VALUE(WEBGPU_COMPARE_ALWAYS);
}

static void show_cull(void)
{
    SIZE(webgpu_cull_t);
    VALUE(WEBGPU_CULL_NONE);
    VALUE(WEBGPU_CULL_FRONT);
    VALUE(WEBGPU_CULL_BACK);
}

static void show_descriptors(void)
{
    printf("%s\n%s\n%s\n%s\n%s\n", webgpu_load_t_desc, webgpu_compare_t_desc,
        webgpu_cull_t_desc, webgpu_mesh_settings_t_desc, webgpu_scene_t_desc);
    printf("%s\n%s\n%s\n%s\n%s\n", webgpu_view_t_desc, webgpu_mesh_t_desc,
        webgpu_texture_t_desc, webgpu_draw_t_desc, webgpu_batch_t_desc);
}

int main(int argc, char **argv)
{
    static const show_mode_t MODES[] = {
        { "settings", show_settings }, { "scene", show_scene },
        { "view", show_view }, { "mesh", show_mesh },
        { "texture", show_texture }, { "draw", show_draw },
        { "batch", show_batch }, { "load", show_load },
        { "compare", show_compare }, { "cull", show_cull },
        { "descriptors", show_descriptors }
    };

    for (size_t i = 0; argc > 1 && i < sizeof(MODES) / sizeof(MODES[0]); i++)
        if (!strcmp(argv[1], MODES[i].name)) {
            MODES[i].show();
            return (0);
        }
    printf("usage: types <mode>\n");
    return (2);
}
