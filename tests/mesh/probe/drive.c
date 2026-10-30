/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "probe.h"

static webgpu_frame_t get_frame(void)
{
    const probe_t *p = probe_get();
    const gpu_frame_t *g = &p->gpu.frame;
    webgpu_frame_t f = { .state = p->state };

    if (p->state == WEBGPU_FRAME_NO_DEVICE)
        return (f);
    f.format = p->lie ? p->lie : g->spec.format;
    f.depth_format = g->spec.depth;
    f.samples = g->spec.samples;
    f.width = SIZE;
    f.height = SIZE;
    f.device = (uint64_t)(uintptr_t)p->gpu.device;
    f.queue = (uint64_t)(uintptr_t)p->gpu.queue;
    if (p->state != WEBGPU_FRAME_READY)
        return (f);
    f.frame = (uint64_t)(uintptr_t)g->color_view;
    f.target = (uint64_t)(uintptr_t)(g->multi_view ? g->multi_view
        : g->color_view);
    f.depth = (uint64_t)(uintptr_t)g->depth_view;
    return (f);
}

static void remake_frame(probe_t *p, enum act act, uint32_t value)
{
    gpu_spec_t spec = p->gpu.frame.spec;

    if (act == DEVICE)
        gpu_renew(&p->gpu, p->scene->small);
    if (act == FORMAT)
        spec.format = value;
    if (act == SAMPLES)
        spec.samples = value;
    if (act == DEPTH)
        spec.depth = value;
    gpu_make_frame(&p->gpu, &spec);
}

static void write_setting(const bridge_api_t *api, enum act act,
    int64_t value)
{
    webgpu_mesh_settings_t *s = bridge_native_get_single(api,
        probe_get()->slots[SETTINGS]);

    if (!s)
        return;
    if (act == LOAD_TO)
        s->load = (webgpu_load_t)value;
    if (act == BLEND_TO)
        s->blend = value != 0;
    if (act == CLEAR_DEPTH_TO)
        s->clear_depth = (float)value / 1000.0f;
    if (act == COMPARE_TO)
        s->depth_compare = (webgpu_compare_t)value;
    if (act == CULL_TO)
        s->cull = (webgpu_cull_t)value;
    if (act == CLOCKWISE_TO)
        s->clockwise = value != 0;
    if (act == REPEAT_TO)
        s->repeat = value != 0;
    if (act == GPU_MEMORY_TO)
        s->gpu_memory = (uint64_t)value;
}

static void apply_step(const bridge_api_t *api, const step_t *st)
{
    probe_t *p = probe_get();
    webgpu_frame_t frame = get_frame();
    char text[32];

    if (st->act == STATE)
        p->state = (webgpu_frame_state_t)st->value;
    if (st->act == LIE)
        p->lie = (uint32_t)st->value;
    if (st->act == GONE)
        api->detach(api, 0, p->slots[FRAME]);
    p->gone = st->act == GONE ? true : p->gone;
    if (st->act == BACK)
        BRIDGE_NATIVE_ATTACH(api, 0, p->slots[FRAME], frame);
    p->gone = st->act == BACK ? false : p->gone;
    if (st->act >= DEVICE && st->act <= DEPTH)
        remake_frame(p, st->act, (uint32_t)st->value);
    if (st->act >= LOAD_TO && st->act <= GPU_MEMORY_TO)
        write_setting(api, st->act, st->value);
    if (st->act != RELOAD)
        return;
    snprintf(text, sizeof(text), "reload %d", (int)probe_reload(
        p->on->module));
    probe_say(api, text, 0, 0);
}

static webgpu_scene_t get_scene(bool dark)
{
    webgpu_scene_t scene = { .clear = { 51.0f / 255, 102.0f / 255,
        153.0f / 255, 1 }, .light = { 0, 0, 1 },
        .light_color = { 0.6f, 0.4f, 0.2f },
        .ambient = { 0.2f, 0.2f, 0.2f } };

    if (dark)
        memcpy(scene.clear, (float[4]){ 0, 0, 0, 1 }, sizeof(scene.clear));
    return (scene);
}

static void open_frame(const bridge_api_t *api, probe_t *p)
{
    webgpu_scene_t scene = get_scene(p->scene->dark);

    if (!gpu_open(&p->gpu, p->scene->small)) {
        probe_say(api, "no adapter", 0, 0);
        return;
    }
    gpu_make_frame(&p->gpu, &p->scene->spec);
    p->opened = true;
    p->state = WEBGPU_FRAME_READY;
    if (!p->scene->none)
        BRIDGE_NATIVE_ATTACH(api, 0, p->slots[LIGHTS], scene);
}

void probe_drive_frame(const bridge_api_t *api)
{
    probe_t *p = probe_get();
    const step_t *steps;
    uint32_t count;
    webgpu_frame_t *f;
    webgpu_scene_t *light;

    if (p->round == 1)
        open_frame(api, p);
    if (!p->opened)
        return;
    steps = probe_get_steps(&count);
    for (uint32_t i = 0; i < count; i++)
        if (steps[i].round == p->round && !strcmp(steps[i].scene,
            p->scene->name))
            apply_step(api, &steps[i]);
    gpu_clear(&p->gpu, &(gpu_clear_t){ { 7.0f / 255, 7.0f / 255, 7.0f / 255,
        1 }, p->scene->prefill ? p->scene->prefill_depth : 1.0f,
        p->scene->prefill ? p->scene->prefill_stencil : 0 });
    f = bridge_native_get_single(api, p->slots[FRAME]);
    if (f && !p->gone)
        *f = get_frame();
    light = bridge_native_get_single(api, p->slots[LIGHTS]);
    if (light)
        *light = get_scene(p->scene->dark);
}
