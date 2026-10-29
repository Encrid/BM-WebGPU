/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_metal.h>
#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

#include "bridge_native.h"
#include "webgpu_device.h"

#define SCENE_FIELDS { char scene[32]; char module[512]; }
#define COUNT(items) (sizeof(items) / sizeof((items)[0]))
#define LABEL(text) ((WGPUStringView){ text, sizeof(text) - 1 })
#define ENUM(api, name, T) BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, name, T, \
    BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE)
#define SINGLE(api, name, T) BRIDGE_NATIVE_COMPONENT(api, name, T, \
    BRIDGE_STRIP, BRIDGE_SINGLETON, BRIDGE_ON_DESTROY_REMOVE)

BRIDGE_SCHEMA_STRUCT_TYPE(test_scene_t, SCENE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(test_scene_t, SCENE_FIELDS);

enum act : uint32_t {
    RESIZE = 1,
    HIDE,
    SHOW,
    SIZE,
    KIND,
    FOREIGN,
    ZERO,
    NO_WINDOW,
    SAMPLES,
    DESTROY,
    REPORT,
    BIND,
    SCOPED,
    FEATURE,
    SETTINGS,
    RELOAD,
    TELL,
    NO_X11_DISPLAY,
    NO_WL_DISPLAY,
    NO_WL_SURFACE,
    END
};

typedef struct {
    const char *scene;
    uint32_t from;
    uint32_t until;
    enum act act;
    uint32_t a;
    uint32_t b;
} step_t;

constexpr uint32_t SIDE = 64;
constexpr uint32_t WIDE = 8193;
constexpr uint32_t LAST = 99;
constexpr uint32_t ROUNDS = 6;
#ifdef OTHER_STORAGE
constexpr enum bridge_storage SETTINGS_STORAGE = BRIDGE_TABLE;
#else
constexpr enum bridge_storage SETTINGS_STORAGE = BRIDGE_SINGLETON;
#endif
#ifdef __APPLE__
constexpr uint32_t FOREIGN_KIND = WEBGPU_SURFACE_WINDOWS;
constexpr SDL_WindowFlags FLAGS = SDL_WINDOW_METAL;
#else
constexpr uint32_t FOREIGN_KIND = WEBGPU_SURFACE_METAL;
constexpr SDL_WindowFlags FLAGS = 0;
#endif

[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]]
static const char WHO[] = MANIFEST;

static const step_t STEPS[] = {
    { "none", 1, LAST, NO_WINDOW, 0, 0 },
    { "none", 4, 4, END, 0, 0 },
    { "close", 5, 5, KIND, WEBGPU_SURFACE_NONE, 0 },
    { "close", 10, 10, END, 0, 0 },
    { "foreign", 1, LAST, FOREIGN, 0, 0 },
    { "zero", 1, LAST, ZERO, 0, 0 },
    { "nodisplay", 1, LAST, NO_X11_DISPLAY, 0, 0 },
    { "nowldisplay", 1, LAST, NO_WL_DISPLAY, 0, 0 },
    { "nowlsurface", 1, LAST, NO_WL_SURFACE, 0, 0 },
    { "nokind", 1, LAST, KIND, 0, 0 },
    { "resize", 6, LAST, RESIZE, 80, SIDE },
    { "resize", 7, 7, END, 0, 0 },
    { "heighten", 6, LAST, RESIZE, SIDE, 80 },
    { "heighten", 7, 7, END, 0, 0 },
    { "stale", 6, LAST, RESIZE, 80, SIDE },
    { "stale", 6, LAST, SIZE, SIDE, SIDE },
    { "stale", 9, 9, END, 0, 0 },
    { "flat", 6, 6, SIZE, 0, SIDE },
    { "flat", 7, 7, END, 0, 0 },
    { "tall", 6, 6, SIZE, SIDE, 0 },
    { "tall", 7, 7, END, 0, 0 },
    { "wide", 6, 6, SIZE, WIDE, SIDE },
    { "wide", 7, 7, END, 0, 0 },
    { "high", 6, 6, SIZE, SIDE, WIDE },
    { "high", 7, 7, END, 0, 0 },
    { "hide", 5, 5, HIDE, 0, 0 },
    { "hide", 7, 7, SHOW, 0, 0 },
    { "hide", 7, LAST, RESIZE, SIDE, SIDE },
    { "hide", 10, 10, END, 0, 0 },
    { "pause", 5, 5, SIZE, 0, SIDE },
    { "pause", 11, 11, END, 0, 0 },
    { "destroy", 5, 5, SIZE, 0, SIDE },
    { "destroy", 5, 5, DESTROY, 0, 0 },
    { "destroy", 5, 5, REPORT, 0, 0 },
    { "destroy", 11, 11, END, 0, 0 },
    { "silent", 5, 5, SIZE, 0, SIDE },
    { "silent", 5, 5, DESTROY, 0, 0 },
    { "silent", 11, 11, END, 0, 0 },
    { "retune", 5, 5, SAMPLES, WEBGPU_SAMPLES_4, 0 },
    { "retune", 6, 6, SIZE, 0, SIDE },
    { "retune", 6, 6, DESTROY, 0, 0 },
    { "retune", 6, 6, REPORT, 0, 0 },
    { "retune", 12, 12, END, 0, 0 },
    { "invalid", 5, 5, BIND, 0, 0 },
    { "invalid", 7, 7, END, 0, 0 },
    { "scoped", 5, 5, SCOPED, 0, 0 },
    { "scoped", 7, 7, END, 0, 0 },
    { "feature", 4, 4, FEATURE, 0, 0 },
    { "tune", 1, 1, SETTINGS, 0, 0 },
    { "tune", 2, 2, END, 0, 0 },
    { "tell", 1, 1, TELL, 0, 0 },
    { "reload", 1, 1, RELOAD, 0, 0 },
    { "reload", 2, 2, END, 0, 0 }
};

static uint32_t scene_slot;
static uint32_t settings_slot;
static uint32_t window_slot;
static uint32_t frame_slot;
static uint32_t round_now;
static uint32_t devices;
static bool had_device;
static SDL_Window *window;
static SDL_MetalView view;

static void say(const bridge_api_t *api, const char *label, uint32_t a0,
    uint32_t a1)
{
    char text[160];
    bridge_line_t line = { text, 0, a0, a1 };

    snprintf(text, sizeof(text), "r%u %s", round_now, label);
    line.len = (uint32_t)strlen(text);
    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
}

static bool is_now(const step_t *step, const test_scene_t *on)
{
    return (step->from <= round_now && round_now <= step->until
        && !strcmp(step->scene, on->scene));
}

static bool has_act(const test_scene_t *on, enum act act)
{
    for (size_t i = 0; i < COUNT(STEPS); i++)
        if (STEPS[i].act == act && is_now(&STEPS[i], on))
            return (true);
    return (false);
}

static uint32_t end_round(const test_scene_t *on)
{
    for (size_t i = 0; i < COUNT(STEPS); i++)
        if (STEPS[i].act == END && !strcmp(STEPS[i].scene, on->scene))
            return (STEPS[i].from);
    return (ROUNDS);
}

static void count_device(const webgpu_frame_t *f)
{
    bool has = f && f->device != 0;

    if (has && !had_device)
        devices++;
    had_device = has;
}

static uint32_t mark_target(const webgpu_frame_t *f)
{
    if (f->target == 0)
        return (0);
    return (f->target == f->frame ? 1 : 2);
}

static void say_frame(const bridge_api_t *api, const char *label,
    const webgpu_frame_t *f)
{
    char text[128];

    if (!f) {
        say(api, "no frame", 0, 0);
        return;
    }
    snprintf(text, sizeof(text), "%s %u %u %u %u %u %u %u %u %u %u %u", label,
        (unsigned)f->state, (unsigned)f->format, (unsigned)f->depth_format,
        (unsigned)f->samples, (unsigned)f->width, (unsigned)f->height,
        (unsigned)(f->device != 0 ? devices : 0), (unsigned)(f->queue != 0),
        (unsigned)(f->frame != 0), (unsigned)mark_target(f),
        (unsigned)(f->depth != 0));
    say(api, text, 0, 0);
}

static webgpu_surface_t get_kind(const webgpu_window_t *w)
{
    if (w->layer != 0)
        return (WEBGPU_SURFACE_METAL);
    if (w->hwnd != 0)
        return (WEBGPU_SURFACE_WINDOWS);
    if (w->x11_window != 0)
        return (WEBGPU_SURFACE_X11);
    return (w->wl_surface != 0 ? WEBGPU_SURFACE_WAYLAND : WEBGPU_SURFACE_NONE);
}

static void read_window(webgpu_window_t *w)
{
    SDL_PropertiesID p = SDL_GetWindowProperties(window);
    int width = 0;
    int height = 0;

    SDL_GetWindowSizeInPixels(window, &width, &height);
    w->width = (uint32_t)width;
    w->height = (uint32_t)height;
    w->hinstance = (uintptr_t)SDL_GetPointerProperty(p,
        SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);
    w->hwnd = (uintptr_t)SDL_GetPointerProperty(p,
        SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    w->layer = view ? (uintptr_t)SDL_Metal_GetLayer(view) : 0;
    w->x11_display = (uintptr_t)SDL_GetPointerProperty(p,
        SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
    w->x11_window = (uint64_t)SDL_GetNumberProperty(p,
        SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
    w->wl_display = (uintptr_t)SDL_GetPointerProperty(p,
        SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
    w->wl_surface = (uintptr_t)SDL_GetPointerProperty(p,
        SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
    w->surface = get_kind(w);
}

static void move_window(const test_scene_t *on)
{
    SDL_PumpEvents();
    if (round_now == 1)
        SDL_SetWindowSize(window, (int)SIDE, (int)SIDE);
    for (size_t i = 0; i < COUNT(STEPS); i++) {
        if (!is_now(&STEPS[i], on))
            continue;
        if (STEPS[i].act == RESIZE)
            SDL_SetWindowSize(window, (int)STEPS[i].a, (int)STEPS[i].b);
        if (STEPS[i].act == HIDE)
            SDL_HideWindow(window);
        if (STEPS[i].act == SHOW)
            SDL_ShowWindow(window);
    }
    SDL_SyncWindow(window);
    SDL_PumpEvents();
}

static void shape_by(const step_t *step, webgpu_window_t *w)
{
    if (step->act == SIZE) {
        w->width = step->a;
        w->height = step->b;
    }
    if (step->act == KIND)
        w->surface = (webgpu_surface_t)step->a;
    if (step->act == FOREIGN)
        w->surface = (webgpu_surface_t)FOREIGN_KIND;
    if (step->act == ZERO) {
        w->hwnd = 0;
        w->layer = 0;
        w->x11_window = 0;
        w->wl_surface = 0;
    }
    if (step->act == NO_X11_DISPLAY)
        w->x11_display = 0;
    if (step->act == NO_WL_DISPLAY)
        w->wl_display = 0;
    if (step->act == NO_WL_SURFACE)
        w->wl_surface = 0;
}

static void shape_window(const test_scene_t *on, webgpu_window_t *w)
{
    for (size_t i = 0; i < COUNT(STEPS); i++)
        if (is_now(&STEPS[i], on))
            shape_by(&STEPS[i], w);
    if (round_now >= end_round(on))
        w->surface = WEBGPU_SURFACE_NONE;
}

static void tune_settings(const bridge_api_t *api, const test_scene_t *on)
{
    webgpu_device_settings_t *s = bridge_native_get_single(api,
        settings_slot);

    for (size_t i = 0; s && i < COUNT(STEPS); i++)
        if (STEPS[i].act == SAMPLES && is_now(&STEPS[i], on))
            s->samples = (webgpu_samples_t)STEPS[i].a;
}

static void ignore_scope([[maybe_unused]] WGPUPopErrorScopeStatus status,
    [[maybe_unused]] WGPUErrorType type,
    [[maybe_unused]] WGPUStringView message,
    [[maybe_unused]] void *userdata1, [[maybe_unused]] void *userdata2)
{
}

static void report_loss(WGPUDevice device)
{
    WGPUCommandEncoderDescriptor shape = {
        .label = LABEL("test:probe report") };

    wgpuCommandEncoderRelease(wgpuDeviceCreateCommandEncoder(device, &shape));
}

static void bind_frame(WGPUDevice device, const webgpu_frame_t *f,
    bool scoped)
{
    WGPUBindGroupLayoutEntry slot = { .binding = 0,
        .visibility = WGPUShaderStage_Fragment,
        .texture = { .sampleType = WGPUTextureSampleType_Float,
            .viewDimension = WGPUTextureViewDimension_2D } };
    WGPUBindGroupLayoutDescriptor shape = {
        .label = LABEL("test:probe layout"), .entryCount = 1,
        .entries = &slot };
    WGPUBindGroupEntry bound = { .binding = 0,
        .textureView = (WGPUTextureView)(uintptr_t)f->frame };
    WGPUBindGroupDescriptor group = { .label = LABEL("test:probe group"),
        .entryCount = 1, .entries = &bound };
    WGPUPopErrorScopeCallbackInfo pop = {
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = ignore_scope };

    if (scoped)
        wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
    group.layout = wgpuDeviceCreateBindGroupLayout(device, &shape);
    wgpuBindGroupRelease(wgpuDeviceCreateBindGroup(device, &group));
    wgpuBindGroupLayoutRelease(group.layout);
    if (scoped)
        wgpuDevicePopErrorScope(device, pop);
}

static int call_reload(const char *path)
{
    SDL_SharedObject *lib = SDL_LoadObject(path);
    SDL_FunctionPointer reload = nullptr;
    int got = -1;

    if (lib)
        reload = SDL_LoadFunction(lib, "bridge_reload");
    if (reload)
        got = ((int (*)(const bridge_api_t *))reload)(nullptr);
    if (lib)
        SDL_UnloadObject(lib);
    return (got);
}

static void say_settings(const bridge_api_t *api)
{
    const webgpu_device_settings_t *s = bridge_native_get_single(api,
        settings_slot);

    if (s)
        say(api, "settings", (uint32_t)s->present, (uint32_t)s->samples);
}

static void act_on_device(const bridge_api_t *api, const step_t *step,
    const webgpu_frame_t *f)
{
    WGPUDevice device = (WGPUDevice)(uintptr_t)f->device;
    bool bind = step->act == BIND || step->act == SCOPED;

    if (device == nullptr)
        return;
    if (step->act == DESTROY)
        wgpuDeviceDestroy(device);
    if (step->act == REPORT)
        report_loss(device);
    if (bind && f->frame != 0)
        bind_frame(device, f, step->act == SCOPED);
    if (step->act == FEATURE)
        say(api, "feature", (uint32_t)wgpuDeviceHasFeature(device,
            WGPUFeatureName_DepthClipControl), 0);
}

static void act_on(const bridge_api_t *api, const step_t *step,
    const webgpu_frame_t *f, const test_scene_t *on)
{
    if (step->act == SETTINGS)
        say_settings(api);
    if (step->act == RELOAD)
        say(api, "reload", (uint32_t)call_reload(on->module), 0);
    act_on_device(api, step, f);
}

BRIDGE_NATIVE(drive)
{
    const test_scene_t *on = bridge_native_get_single(api, scene_slot);
    webgpu_window_t *w = bridge_native_get_single(api, window_slot);
    webgpu_window_t want = {};

    round_now++;
    if (round_now == 1)
        say_frame(api, "before", bridge_native_get_single(api, frame_slot));
    if (!on || has_act(on, NO_WINDOW))
        return (0);
    move_window(on);
    read_window(&want);
    shape_window(on, &want);
    if (has_act(on, TELL))
        say(api, "window", (uint32_t)want.surface, 0);
    if (w)
        *w = want;
    else
        BRIDGE_NATIVE_ATTACH(api, 0, window_slot, want);
    tune_settings(api, on);
    return (0);
}

BRIDGE_NATIVE(render)
{
    const test_scene_t *on = bridge_native_get_single(api, scene_slot);
    const webgpu_frame_t *f = bridge_native_get_single(api, frame_slot);

    count_device(f);
    say_frame(api, "frame", f);
    for (size_t i = 0; on && f && i < COUNT(STEPS); i++)
        if (is_now(&STEPS[i], on))
            act_on(api, &STEPS[i], f, on);
    return (0);
}

BRIDGE_NATIVE(check)
{
    const test_scene_t *on = bridge_native_get_single(api, scene_slot);

    if (on && round_now < end_round(on))
        return (0);
    return (BRIDGE_END);
}

static bool open_window(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return (false);
    window = SDL_CreateWindow("probe", (int)SIDE, (int)SIDE, FLAGS);
    if (window && (FLAGS & SDL_WINDOW_METAL) != 0)
        view = SDL_Metal_CreateView(window);
    return (window != nullptr);
}

static void declare_types(const bridge_api_t *api)
{
    scene_slot = SINGLE(api, "test:Scene", test_scene_t);
    ENUM(api, "webgpu:webgpu_present_t", webgpu_present_t);
    ENUM(api, "webgpu:webgpu_format_t", webgpu_format_t);
    ENUM(api, "webgpu:webgpu_alpha_t", webgpu_alpha_t);
    ENUM(api, "webgpu:webgpu_power_t", webgpu_power_t);
    ENUM(api, "webgpu:webgpu_backend_t", webgpu_backend_t);
    ENUM(api, "webgpu:webgpu_feature_level_t", webgpu_feature_level_t);
    ENUM(api, "webgpu:webgpu_samples_t", webgpu_samples_t);
    ENUM(api, "webgpu:webgpu_depth_t", webgpu_depth_t);
    ENUM(api, "webgpu:webgpu_limits_t", webgpu_limits_t);
    ENUM(api, "webgpu:webgpu_features_t", webgpu_features_t);
    ENUM(api, "webgpu:webgpu_surface_t", webgpu_surface_t);
    ENUM(api, "webgpu:webgpu_frame_state_t", webgpu_frame_state_t);
    settings_slot = BRIDGE_NATIVE_COMPONENT(api, "webgpu:DeviceSettings",
        webgpu_device_settings_t, BRIDGE_STRIP, SETTINGS_STORAGE,
        BRIDGE_ON_DESTROY_REMOVE);
    window_slot = SINGLE(api, "webgpu:Window", webgpu_window_t);
    frame_slot = SINGLE(api, "webgpu:Frame", webgpu_frame_t);
}

static void add_systems(const bridge_api_t *api)
{
    bridge_term_t drive[4] = { { scene_slot, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
        { window_slot, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 },
        { settings_slot, BRIDGE_WRITE, BRIDGE_OPTIONAL, 0 },
        { frame_slot, BRIDGE_READ, BRIDGE_OPTIONAL, 0 } };
    bridge_term_t render[3] = {
        { scene_slot, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
        { frame_slot, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
        { settings_slot, BRIDGE_READ, BRIDGE_OPTIONAL, 0 } };
    bridge_term_t check[1] = { { scene_slot, BRIDGE_READ, BRIDGE_OPTIONAL,
        0 } };
    uint32_t changes[1] = { window_slot };

    BRIDGE_NATIVE_ADD_SYSTEM(api, "drive", "drive", drive, 4, changes, 1,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "render", "render", render, 3, nullptr, 0,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "check", "check", check, 1, nullptr, 0,
        nullptr, 0);
}

BRIDGE_NATIVE(init)
{
    if (!open_window())
        return (1);
    declare_types(api);
    add_systems(api);
    return (0);
}

BRIDGE_NATIVE(shutdown)
{
    if (view)
        SDL_Metal_DestroyView(view);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
    return (0);
}

BRIDGE_NATIVE(reload)
{
    return (1);
}
