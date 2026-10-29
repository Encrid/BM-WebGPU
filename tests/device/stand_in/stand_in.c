/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <webgpu/webgpu.h>

#ifdef _WIN32
    #include <wchar.h>
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

typedef void (*call_t)(void);
typedef WGPUInstance (*create_t)(const WGPUInstanceDescriptor *descriptor);
typedef void (*release_t)(WGPUInstance instance);

static void *real;

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

static call_t find(const char *name)
{
    return ((call_t)GetProcAddress(real, name));
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
    return (dlopen(path, RTLD_NOW | RTLD_LOCAL));
}

static call_t find(const char *name)
{
    void *symbol = dlsym(real, name);
    call_t call = nullptr;

    memcpy(&call, &symbol, sizeof(call));
    return (call);
}

#endif

WGPUInstance wgpuCreateInstance(const WGPUInstanceDescriptor *descriptor)
{
    if (getenv("STAND_IN_NO_INSTANCE"))
        return (nullptr);
    if (!real)
        real = load_real();
    if (!real)
        return (nullptr);
    return (((create_t)find("wgpuCreateInstance"))(descriptor));
}

void wgpuInstanceRelease(WGPUInstance instance)
{
    ((release_t)find("wgpuInstanceRelease"))(instance);
}
