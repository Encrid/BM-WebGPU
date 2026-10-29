/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

#ifdef _WIN32
    #include <wchar.h>
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

#ifdef RTLD_DEEPBIND
constexpr int OPEN_FLAGS = RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND;
#elif !defined(_WIN32)
constexpr int OPEN_FLAGS = RTLD_NOW | RTLD_LOCAL;
#endif

constexpr size_t MOST_ITEMS = 16;
constexpr size_t TEXT_ROOM = 256;
constexpr uint32_t LATE_RUNS = 2;
constexpr uint32_t LOST = WGPUSurfaceGetCurrentTextureStatus_Lost;
constexpr uint32_t SUBOPTIMAL =
    WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal;

typedef void (*call_t)(void);

enum stage : uint32_t {
    IDLE = 0,
    LOADING,
    LOADED
};

enum answer_kind : uint32_t {
    NO_ANSWER = 0,
    ADAPTER_ANSWER,
    DEVICE_ANSWER
};

typedef struct {
    size_t count;
    uint32_t items[MOST_ITEMS];
} list_t;

typedef struct {
    enum answer_kind kind;
    uint32_t status;
    void *made;
    char text[TEXT_ROOM];
    size_t length;
    uint32_t runs;
    WGPURequestAdapterCallbackInfo adapter;
    WGPURequestDeviceCallbackInfo device;
    WGPUUncapturedErrorCallbackInfo errors;
} answer_t;

static const char STAND_IN[] = "stand-in";

static atomic_uint stage;
static void *real;
static answer_t answer;
static uint32_t texture_calls;
static uint32_t held;

#ifdef _WIN32

static void *load_real(void)
{
    HMODULE self = nullptr;
    wchar_t path[2 * MAX_PATH];
    wchar_t name[MAX_PATH];
    wchar_t *at = nullptr;

    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&real, &self);
    GetModuleFileNameW(self, path, MAX_PATH);
    at = wcsrchr(path, L'\\') + 1;
    wcscpy(name, at);
    wcscpy(at, L"real\\");
    wcscat(at, name);
    return (LoadLibraryExW(path, nullptr, 0));
}

#else

static void *load_real(void)
{
    Dl_info self = {};
    char path[4096];
    const char *name = nullptr;

    dladdr(&real, &self);
    name = strrchr(self.dli_fname, '/') + 1;
    snprintf(path, sizeof(path), "%.*sreal/%s", (int)(name - self.dli_fname),
        self.dli_fname, name);
    return (dlopen(path, OPEN_FLAGS));
}

#endif

static void *get_real(void)
{
    unsigned int idle = IDLE;

    if (atomic_compare_exchange_strong(&stage, &idle, LOADING)) {
        real = load_real();
        atomic_store(&stage, LOADED);
    }
    while (atomic_load(&stage) != LOADED)
        continue;
    return (real);
}

#ifdef _WIN32

static call_t find_real(const char *name)
{
    HMODULE lib = get_real();

    return (lib != nullptr ? (call_t)GetProcAddress(lib, name) : nullptr);
}

#else

static call_t find_real(const char *name)
{
    void *lib = get_real();
    void *symbol = lib != nullptr ? dlsym(lib, name) : nullptr;
    call_t call = nullptr;

    memcpy(&call, &symbol, sizeof(call));
    return (call);
}

#endif

static list_t read_list(const char *name)
{
    const char *at = getenv(name);
    char *end = nullptr;
    list_t got = {};

    for (; at != nullptr && got.count < MOST_ITEMS; at = end) {
        got.items[got.count] = (uint32_t)strtoul(at, &end, 0);
        if (end == at)
            break;
        got.count++;
    }
    return (got);
}

static bool has_item(const list_t *list, uint32_t value)
{
    for (size_t i = 0; i < list->count; i++)
        if (list->items[i] == value)
            return (true);
    return (false);
}

static void *copy_items(const void *items, size_t count, size_t size)
{
    void *copy = count > 0 ? malloc(count * size) : nullptr;

    if (copy != nullptr)
        memcpy(copy, items, count * size);
    return (copy);
}

static size_t put_feature(WGPUFeatureName *items, size_t count,
    WGPUFeatureName feature, const list_t *lacks)
{
    if (has_item(lacks, (uint32_t)feature))
        return (count);
    items[count] = feature;
    return (count + 1);
}

static WGPUSupportedFeatures change_features(const WGPUSupportedFeatures *got)
{
    list_t lacks = read_list("STAND_IN_LACKS");
    list_t more = read_list("STAND_IN_FEATURES");
    list_t copies = read_list("STAND_IN_COPIES");
    size_t extra = copies.count > 0 && got->featureCount > 0
        ? copies.items[0] : 0;
    WGPUFeatureName *items = malloc((got->featureCount + more.count + extra
        + 1) * sizeof(*items));
    size_t count = 0;

    for (size_t i = 0; i < got->featureCount; i++)
        count = put_feature(items, count, got->features[i], &lacks);
    for (size_t i = 0; i < more.count; i++)
        count = put_feature(items, count, (WGPUFeatureName)more.items[i],
            &lacks);
    for (size_t i = 0; i < extra; i++)
        count = put_feature(items, count, got->features[0], &lacks);
    return ((WGPUSupportedFeatures){ count, items });
}

static WGPUPresentMode *copy_presents(const WGPUSurfaceCapabilities *got,
    size_t *count)
{
    list_t kept = read_list("STAND_IN_PRESENTS");
    WGPUPresentMode *modes = malloc((got->presentModeCount + 1)
        * sizeof(*modes));

    *count = 0;
    for (size_t i = 0; modes != nullptr && i < got->presentModeCount; i++) {
        if (kept.count > 0
            && !has_item(&kept, (uint32_t)got->presentModes[i]))
            continue;
        modes[*count] = got->presentModes[i];
        (*count)++;
    }
    return (modes);
}

static void keep_answer(enum answer_kind kind, uint32_t status, void *made,
    WGPUStringView message)
{
    size_t length = message.length;

    if (length == WGPU_STRLEN)
        length = message.data != nullptr ? strlen(message.data) : 0;
    answer.kind = kind;
    answer.status = status;
    answer.made = made;
    answer.length = length < TEXT_ROOM ? length : TEXT_ROOM;
    answer.runs = 0;
    if (answer.length > 0)
        memcpy(answer.text, message.data, answer.length);
}

static void keep_adapter(WGPURequestAdapterStatus status, WGPUAdapter adapter,
    WGPUStringView message, [[maybe_unused]] void *userdata1,
    [[maybe_unused]] void *userdata2)
{
    keep_answer(ADAPTER_ANSWER, status, adapter, message);
}

static void keep_device(WGPURequestDeviceStatus status, WGPUDevice device,
    WGPUStringView message, [[maybe_unused]] void *userdata1,
    [[maybe_unused]] void *userdata2)
{
    keep_answer(DEVICE_ANSWER, status, device, message);
}

static void tell_error(const answer_t *given)
{
    list_t code = read_list("STAND_IN_ERROR");
    WGPUDevice device = given->made;
    WGPUStringView text = { STAND_IN, sizeof(STAND_IN) - 1 };

    if (code.count == 0 || given->errors.callback == nullptr
        || given->status != WGPURequestDeviceStatus_Success)
        return;
    given->errors.callback(&device, (WGPUErrorType)code.items[0], text,
        given->errors.userdata1, given->errors.userdata2);
}

static void give_answer(void)
{
    answer_t now = answer;
    WGPUStringView text = { now.text, now.length };

    answer.kind = NO_ANSWER;
    if (now.kind == ADAPTER_ANSWER)
        now.adapter.callback((WGPURequestAdapterStatus)now.status, now.made,
            text, now.adapter.userdata1, now.adapter.userdata2);
    if (now.kind != DEVICE_ANSWER)
        return;
    now.device.callback((WGPURequestDeviceStatus)now.status, now.made, text,
        now.device.userdata1, now.device.userdata2);
    tell_error(&now);
}

static bool is_mended_by_configure(uint32_t status)
{
    return (status == WGPUSurfaceGetCurrentTextureStatus_Timeout
        || status == WGPUSurfaceGetCurrentTextureStatus_Outdated
        || status == SUBOPTIMAL);
}

static uint32_t take_status(void)
{
    list_t asked = read_list("STAND_IN_TEXTURE");
    uint32_t given = 0;

    texture_calls++;
    if (asked.count == 2 && asked.items[0] == texture_calls)
        held = asked.items[1];
    given = held;
    if (!is_mended_by_configure(held) && held != LOST)
        held = 0;
    return (given);
}

WGPUInstance wgpuCreateInstance(const WGPUInstanceDescriptor *descriptor)
{
    if (getenv("STAND_IN_NO_INSTANCE") != nullptr || get_real() == nullptr)
        return (nullptr);
    return (((WGPUProcCreateInstance)find_real("wgpuCreateInstance"))(
        descriptor));
}

void wgpuInstanceProcessEvents(WGPUInstance instance)
{
    ((WGPUProcInstanceProcessEvents)find_real("wgpuInstanceProcessEvents"))(
        instance);
    if (answer.kind == NO_ANSWER)
        return;
    answer.runs++;
    if (answer.runs >= LATE_RUNS)
        give_answer();
}

WGPUFuture wgpuInstanceRequestAdapter(WGPUInstance instance,
    const WGPURequestAdapterOptions *options,
    WGPURequestAdapterCallbackInfo callbackInfo)
{
    WGPURequestAdapterCallbackInfo mine = {
        .mode = WGPUCallbackMode_AllowSpontaneous, .callback = keep_adapter };
    WGPUFuture future = {};

    answer.adapter = callbackInfo;
    future = ((WGPUProcInstanceRequestAdapter)find_real(
        "wgpuInstanceRequestAdapter"))(instance, options, mine);
    if (getenv("STAND_IN_LATE") == nullptr)
        give_answer();
    return (future);
}

WGPUFuture wgpuAdapterRequestDevice(WGPUAdapter adapter,
    const WGPUDeviceDescriptor *descriptor,
    WGPURequestDeviceCallbackInfo callbackInfo)
{
    WGPURequestDeviceCallbackInfo mine = {
        .mode = WGPUCallbackMode_AllowSpontaneous, .callback = keep_device };
    WGPUStringView text = { STAND_IN, sizeof(STAND_IN) - 1 };
    WGPUFuture future = {};

    answer.device = callbackInfo;
    answer.errors = (WGPUUncapturedErrorCallbackInfo){};
    if (descriptor != nullptr)
        answer.errors = descriptor->uncapturedErrorCallbackInfo;
    if (getenv("STAND_IN_NO_DEVICE") != nullptr)
        keep_answer(DEVICE_ANSWER, WGPURequestDeviceStatus_Error, nullptr,
            text);
    else
        future = ((WGPUProcAdapterRequestDevice)find_real(
            "wgpuAdapterRequestDevice"))(adapter, descriptor, mine);
    if (getenv("STAND_IN_LATE") == nullptr)
        give_answer();
    return (future);
}

void wgpuAdapterGetFeatures(WGPUAdapter adapter,
    WGPUSupportedFeatures *features)
{
    WGPUSupportedFeatures got = {};

    ((WGPUProcAdapterGetFeatures)find_real("wgpuAdapterGetFeatures"))(adapter,
        &got);
    *features = change_features(&got);
    ((WGPUProcSupportedFeaturesFreeMembers)find_real(
        "wgpuSupportedFeaturesFreeMembers"))(got);
}

void wgpuDeviceGetFeatures(WGPUDevice device, WGPUSupportedFeatures *features)
{
    WGPUSupportedFeatures got = {};

    ((WGPUProcDeviceGetFeatures)find_real("wgpuDeviceGetFeatures"))(device,
        &got);
    features->featureCount = got.featureCount;
    features->features = copy_items(got.features, got.featureCount,
        sizeof(*got.features));
    ((WGPUProcSupportedFeaturesFreeMembers)find_real(
        "wgpuSupportedFeaturesFreeMembers"))(got);
}

void wgpuSupportedFeaturesFreeMembers(WGPUSupportedFeatures supportedFeatures)
{
    free((void *)supportedFeatures.features);
}

WGPUBool wgpuAdapterHasFeature(WGPUAdapter adapter, WGPUFeatureName feature)
{
    list_t lacks = read_list("STAND_IN_LACKS");

    if (has_item(&lacks, (uint32_t)feature))
        return (false);
    return (((WGPUProcAdapterHasFeature)find_real("wgpuAdapterHasFeature"))(
        adapter, feature));
}

WGPUStatus wgpuSurfaceGetCapabilities(WGPUSurface surface, WGPUAdapter adapter,
    WGPUSurfaceCapabilities *capabilities)
{
    list_t most = read_list("STAND_IN_FORMATS");
    WGPUSurfaceCapabilities got = {};
    WGPUStatus status = ((WGPUProcSurfaceGetCapabilities)find_real(
        "wgpuSurfaceGetCapabilities"))(surface, adapter, &got);

    *capabilities = got;
    if (most.count > 0 && most.items[0] < got.formatCount)
        capabilities->formatCount = most.items[0];
    capabilities->formats = copy_items(got.formats, capabilities->formatCount,
        sizeof(*got.formats));
    capabilities->presentModes = copy_presents(&got,
        &capabilities->presentModeCount);
    capabilities->alphaModes = copy_items(got.alphaModes, got.alphaModeCount,
        sizeof(*got.alphaModes));
    ((WGPUProcSurfaceCapabilitiesFreeMembers)find_real(
        "wgpuSurfaceCapabilitiesFreeMembers"))(got);
    return (status);
}

void wgpuSurfaceCapabilitiesFreeMembers(
    WGPUSurfaceCapabilities surfaceCapabilities)
{
    free((void *)surfaceCapabilities.formats);
    free((void *)surfaceCapabilities.presentModes);
    free((void *)surfaceCapabilities.alphaModes);
}

WGPUTexture wgpuDeviceCreateTexture(WGPUDevice device,
    const WGPUTextureDescriptor *descriptor)
{
    list_t most = read_list("STAND_IN_SAMPLES");
    WGPUTextureDescriptor shape = *descriptor;

    if (most.count > 0 && shape.sampleCount > most.items[0])
        shape.sampleCount = most.items[0];
    return (((WGPUProcDeviceCreateTexture)find_real(
        "wgpuDeviceCreateTexture"))(device, &shape));
}

WGPUSurface wgpuInstanceCreateSurface(WGPUInstance instance,
    const WGPUSurfaceDescriptor *descriptor)
{
    if (held == LOST)
        held = 0;
    return (((WGPUProcInstanceCreateSurface)find_real(
        "wgpuInstanceCreateSurface"))(instance, descriptor));
}

void wgpuSurfaceConfigure(WGPUSurface surface,
    const WGPUSurfaceConfiguration *config)
{
    if (is_mended_by_configure(held))
        held = 0;
    ((WGPUProcSurfaceConfigure)find_real("wgpuSurfaceConfigure"))(surface,
        config);
}

void wgpuSurfaceGetCurrentTexture(WGPUSurface surface,
    WGPUSurfaceTexture *surfaceTexture)
{
    uint32_t given = take_status();

    if (given != 0 && given != SUBOPTIMAL) {
        surfaceTexture->texture = nullptr;
        surfaceTexture->status = (WGPUSurfaceGetCurrentTextureStatus)given;
        return;
    }
    ((WGPUProcSurfaceGetCurrentTexture)find_real(
        "wgpuSurfaceGetCurrentTexture"))(surface, surfaceTexture);
    if (given == SUBOPTIMAL && surfaceTexture->status
        == WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
        surfaceTexture->status = (WGPUSurfaceGetCurrentTextureStatus)given;
}
