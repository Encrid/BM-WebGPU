/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef DEVICE_INSIDE_H
    #define DEVICE_INSIDE_H

    #include <stdatomic.h>

    #include <webgpu/webgpu.h>
    #include <webgpu/wgpu.h>

    #include "bridge_native.h"
    #include "webgpu_device.h"

constexpr uint32_t SETTINGS = 18;
constexpr uint32_t WINDOW = 19;
constexpr uint32_t FRAME = 20;
constexpr size_t MOST_FEATURES = 128;

typedef struct {
    const char *name;
    const char *desc;
    enum bridge_kind kind;
} declared_type_t;

typedef enum device_step : uint32_t {
    DEVICE_READY = 0,
    DEVICE_MAKING,
    DEVICE_FAILED,
    DEVICE_WAITING,
    DEVICE_NO_FRAME,
    DEVICE_GONE
} device_step_t;

enum next_step : uint32_t {
    NEXT_ADAPTER = 0,
    NEXT_DEVICE,
    NEXT_KEEP
};

typedef enum fallback : uint32_t {
    FALLBACK_PRESENT = 0,
    FALLBACK_FORMAT,
    FALLBACK_ALPHA,
    FALLBACK_SAMPLES,
    FALLBACK_DEPTH
} fallback_t;

enum text_kind : uint32_t {
    TEXT_ERROR = 1,
    TEXT_LOST
};

typedef struct {
    enum text_kind kind;
    uint32_t code;
    uint32_t len;
} text_head_t;

typedef struct {
    uint8_t *bytes;
    uint32_t size;
    _Atomic uint32_t used;
    _Atomic uint32_t missed;
    _Atomic bool lost;
} room_t;

typedef struct {
    WGPUTexture texture;
    WGPUTextureView view;
} target_t;

typedef struct {
    WGPUInstance instance;
    WGPUSurface surface;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    target_t frame;
    target_t color;
    target_t depth;
    WGPUTextureFormat format;
    WGPUPresentMode present;
    WGPUCompositeAlphaMode alpha;
    uint32_t samples;
    bool checked;
    WGPUTextureFormat depth_format;
    uint32_t width;
    uint32_t height;
    uint32_t max_side;
    enum next_step next;
    bool waiting;
    bool failed;
    bool configure;
} gpu_t;

typedef struct {
    const bridge_api_t *api;
    gpu_t gpu;
    room_t room;
} device_t;

typedef struct {
    WGPUInstanceDescriptor instance;
    WGPUInstanceExtras extras;
    union {
        WGPUChainedStruct chain;
        WGPUSurfaceSourceWindowsHWND windows;
        WGPUSurfaceSourceMetalLayer metal;
        WGPUSurfaceSourceXlibWindow x11;
        WGPUSurfaceSourceWaylandSurface wayland;
    } surface;
} device_source_t;

typedef struct {
    WGPUDeviceDescriptor shape;
    WGPUSupportedFeatures adapter;
    WGPUFeatureName features[MOST_FEATURES];
    size_t count;
    WGPULimits limits;
} device_request_t;

device_t *device_get_state(void);
device_step_t device_make_next(device_t *d, const webgpu_device_settings_t *s,
    const webgpu_window_t *w);
bool device_make_surface(device_t *d, const webgpu_device_settings_t *s,
    const webgpu_window_t *w);
bool device_set_source(device_source_t *s, const webgpu_window_t *w);
void device_set_instance(device_source_t *src,
    const webgpu_device_settings_t *s);
void device_release(device_t *d);
device_step_t device_ask_adapter(device_t *d,
    const webgpu_device_settings_t *s, const webgpu_window_t *w);
device_step_t device_ask_device(device_t *d,
    const webgpu_device_settings_t *s);
bool device_choose_modes(device_t *d, const webgpu_device_settings_t *s);
size_t device_choose_targets(device_t *d, const webgpu_device_settings_t *s,
    WGPUFeatureName *features);
device_step_t device_make_targets(device_t *d);
void device_release_targets(device_t *d);
device_step_t device_take_frame(device_t *d);
void device_release_frame(device_t *d);
bool device_is_lost(const device_t *d);
void device_keep_error(const WGPUDevice *device, WGPUErrorType type,
    WGPUStringView message, void *userdata1, void *userdata2);
void device_keep_lost(const WGPUDevice *device, WGPUDeviceLostReason reason,
    WGPUStringView message, void *userdata1, void *userdata2);
bool device_drain_room(device_t *d);
void device_size_room(device_t *d, uint32_t size);
void device_say_no_device(const device_t *d, const char *reason,
    const WGPUStringView *text);
void device_say_no_surface(const device_t *d, const char *reason);
void device_say_fallback(const device_t *d, fallback_t item, uint32_t asked,
    uint32_t used);

#endif
