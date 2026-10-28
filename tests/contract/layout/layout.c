/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <webgpu_contract.h>

#define SIZE(type) printf("size %zu\nalignment %zu\n", sizeof(type), \
    alignof(type))
#define AT(type, field) printf("%s %zu\n", #field, offsetof(type, field))
#define VALUE(constant) printf("%s %u\n", #constant, (unsigned)(constant))
#define TEXT_OF(...) BRIDGE_SCHEMA_TEXT(__VA_ARGS__)

typedef struct {
    const char *name;
    void (*show)(void);
} show_mode_t;

static void show_window(void)
{
    SIZE(webgpu_window_t);
    AT(webgpu_window_t, surface);
    AT(webgpu_window_t, width);
    AT(webgpu_window_t, height);
    AT(webgpu_window_t, hinstance);
    AT(webgpu_window_t, hwnd);
    AT(webgpu_window_t, layer);
    AT(webgpu_window_t, x11_display);
    AT(webgpu_window_t, x11_window);
    AT(webgpu_window_t, wl_display);
    AT(webgpu_window_t, wl_surface);
}

static void show_frame(void)
{
    SIZE(webgpu_frame_t);
    AT(webgpu_frame_t, state);
    AT(webgpu_frame_t, format);
    AT(webgpu_frame_t, depth_format);
    AT(webgpu_frame_t, samples);
    AT(webgpu_frame_t, width);
    AT(webgpu_frame_t, height);
    AT(webgpu_frame_t, device);
    AT(webgpu_frame_t, queue);
    AT(webgpu_frame_t, frame);
    AT(webgpu_frame_t, target);
    AT(webgpu_frame_t, depth);
}

static void show_surface(void)
{
    SIZE(webgpu_surface_t);
    VALUE(WEBGPU_SURFACE_NONE);
    VALUE(WEBGPU_SURFACE_WINDOWS);
    VALUE(WEBGPU_SURFACE_METAL);
    VALUE(WEBGPU_SURFACE_X11);
    VALUE(WEBGPU_SURFACE_WAYLAND);
}

static void show_frame_state(void)
{
    SIZE(webgpu_frame_state_t);
    VALUE(WEBGPU_FRAME_READY);
    VALUE(WEBGPU_FRAME_SKIPPED);
    VALUE(WEBGPU_FRAME_NO_DEVICE);
}

static void show_descriptors(void)
{
    printf("%s\n%s\n%s\n%s\n", webgpu_surface_t_desc,
        webgpu_frame_state_t_desc, webgpu_window_t_desc,
        webgpu_frame_t_desc);
}

static void show_texts(void)
{
    printf("%s\n%s\n%s\n%s\n", TEXT_OF(WEBGPU_SURFACE_VALUES),
        TEXT_OF(WEBGPU_FRAME_STATE_VALUES), TEXT_OF(WEBGPU_WINDOW_FIELDS),
        TEXT_OF(WEBGPU_FRAME_FIELDS));
}

int main(int argc, char **argv)
{
    static const show_mode_t MODES[] = {
        { "window", show_window }, { "frame", show_frame },
        { "surface", show_surface }, { "frame_state", show_frame_state },
        { "descriptors", show_descriptors }, { "texts", show_texts }
    };

    for (size_t i = 0; argc > 1 && i < sizeof(MODES) / sizeof(MODES[0]); i++)
        if (!strcmp(argv[1], MODES[i].name)) {
            MODES[i].show();
            return (0);
        }
    printf("usage: layout <mode>\n");
    return (2);
}
