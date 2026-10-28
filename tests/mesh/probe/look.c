/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "probe.h"

typedef int32_t (*export_t)(const bridge_api_t *api);

#ifdef _WIN32
[[gnu::dllimport]] void *LoadLibraryA(const char *name);
[[gnu::dllimport]] export_t GetProcAddress(void *module, const char *name);
[[gnu::dllimport]] int FreeLibrary(void *module);
#else
    #include <dlfcn.h>
#endif

int32_t probe_reload(const char *path)
{
    export_t reload = nullptr;
    int32_t got = -1;
#ifdef _WIN32
    void *lib = LoadLibraryA(path);

    if (lib)
        reload = GetProcAddress(lib, "bridge_reload");
    if (reload)
        got = reload(nullptr);
    if (lib)
        FreeLibrary(lib);
#else
    void *lib = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    void *found = lib ? dlsym(lib, "bridge_reload") : nullptr;

    if (found)
        memcpy(&reload, &found, sizeof(reload));
    if (reload)
        got = reload(nullptr);
    if (lib)
        dlclose(lib);
#endif
    return (got);
}

static const char *get_life(const bridge_api_t *api, uint64_t id)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const bridge_entity_t *index = (const bridge_entity_t *)(api->shared_base
        + h->entity_index);
    uint32_t at = (uint32_t)id;

    if (at >= h->entities || index[at].generation != (uint32_t)(id >> 32))
        return ("gone");
    return (index[at].table ? "alive" : "a husk");
}

void probe_look(const bridge_api_t *api, const char *phase)
{
    const probe_t *p = probe_get();
    char text[64];

    for (uint32_t i = 0; i < p->scene->item_count; i++) {
        if (!p->scene->items[i].look || !p->ids[i])
            continue;
        snprintf(text, sizeof(text), "%s item %u is %s", phase, i,
            get_life(api, p->ids[i]));
        probe_say(api, text, 0, 0);
    }
}

static void show_point(const bridge_api_t *api, const gpu_image_t *image,
    uint32_t x, uint32_t y)
{
    const probe_t *p = probe_get();
    const uint8_t *px = image->color + 4 * (y * SIZE + x);
    uint32_t depth = p->gpu.frame.spec.depth;
    char text[64];

    snprintf(text, sizeof(text), "pixel %u %u is %u %u", x, y, px[0], px[1]);
    probe_say(api, text, px[2], px[3]);
    snprintf(text, sizeof(text), "depth %u %u is %.3f", x, y,
        (double)image->depth[y * SIZE + x]);
    if (p->gpu.frame.read_depth && depth == WGPUTextureFormat_Depth32Float)
        probe_say(api, text, 0, 0);
    snprintf(text, sizeof(text), "stencil %u %u is %u", x, y,
        image->stencil[y * SIZE + x]);
    if (p->gpu.frame.read_depth && p->scene->prefill
        && depth == WGPUTextureFormat_Depth24PlusStencil8)
        probe_say(api, text, 0, 0);
}

void probe_show(const bridge_api_t *api)
{
    static gpu_image_t image;
    probe_t *p = probe_get();
    gpu_counts_t c;
    char text[128];

    if (!p->opened)
        return;
    gpu_read(&p->gpu, &image);
    for (uint32_t i = 0; i < p->scene->point_count; i++)
        show_point(api, &image, p->scene->points[i][0],
            p->scene->points[i][1]);
    if (!p->scene->counts)
        return;
    gpu_count(&p->gpu, &c);
    snprintf(text, sizeof(text), "holds %zu buffers %zu textures %zu views "
        "%zu groups %zu samplers", c.buffers, c.textures, c.views, c.groups,
        c.samplers);
    probe_say(api, text, 0, 0);
    snprintf(text, sizeof(text), "makes %zu pipelines %zu shaders %zu layouts "
        "%zu pipeline layouts", c.pipelines, c.shaders, c.layouts,
        c.pipeline_layouts);
    probe_say(api, text, 0, 0);
}
