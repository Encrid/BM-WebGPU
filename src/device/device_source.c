/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "device_inside.h"

#ifdef _WIN32

static bool set_windows_source(device_source_t *s, const webgpu_window_t *w)
{
    s->surface.windows = (WGPUSurfaceSourceWindowsHWND){
        { nullptr, WGPUSType_SurfaceSourceWindowsHWND },
        (void *)(uintptr_t)w->hinstance, (void *)(uintptr_t)w->hwnd };
    return (w->hwnd != 0);
}

#elifdef __APPLE__

static bool set_metal_source(device_source_t *s, const webgpu_window_t *w)
{
    s->surface.metal = (WGPUSurfaceSourceMetalLayer){
        { nullptr, WGPUSType_SurfaceSourceMetalLayer },
        (void *)(uintptr_t)w->layer };
    return (w->layer != 0);
}

#else

static bool set_x11_source(device_source_t *s, const webgpu_window_t *w)
{
    s->surface.x11 = (WGPUSurfaceSourceXlibWindow){
        { nullptr, WGPUSType_SurfaceSourceXlibWindow },
        (void *)(uintptr_t)w->x11_display, w->x11_window };
    return (w->x11_display != 0 && w->x11_window != 0);
}

static bool set_wayland_source(device_source_t *s, const webgpu_window_t *w)
{
    s->surface.wayland = (WGPUSurfaceSourceWaylandSurface){
        { nullptr, WGPUSType_SurfaceSourceWaylandSurface },
        (void *)(uintptr_t)w->wl_display, (void *)(uintptr_t)w->wl_surface };
    s->extras.displayHandle.type = WGPUNativeDisplayHandleType_Wayland;
    s->extras.displayHandle.data.wayland.display = s->surface.wayland.display;
    return (w->wl_display != 0 && w->wl_surface != 0);
}

#endif

bool device_set_source(device_source_t *s, const webgpu_window_t *w)
{
    switch (w->surface) {
#ifdef _WIN32
        case WEBGPU_SURFACE_WINDOWS:
            return (set_windows_source(s, w));
#elifdef __APPLE__
        case WEBGPU_SURFACE_METAL:
            return (set_metal_source(s, w));
#else
        case WEBGPU_SURFACE_X11:
            return (set_x11_source(s, w));
        case WEBGPU_SURFACE_WAYLAND:
            return (set_wayland_source(s, w));
#endif
        default:
            return (false);
    }
}
