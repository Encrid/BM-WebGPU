/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "device_inside.h"

static const char *const FALLBACKS[][2] = {
    [FALLBACK_PRESENT] = { "present", webgpu_present_t_desc },
    [FALLBACK_FORMAT] = { "format", webgpu_format_t_desc },
    [FALLBACK_ALPHA] = { "alpha", webgpu_alpha_t_desc },
    [FALLBACK_SAMPLES] = { "samples", webgpu_samples_t_desc },
    [FALLBACK_DEPTH] = { "depth", webgpu_depth_t_desc }
};

static const char *find_name(const char *desc, uint32_t value, int *len)
{
    const char *at = strstr(desc, " = ");
    const char *name = nullptr;

    while (strtoul(at + 3, nullptr, 10) != value)
        at = strstr(at + 3, " = ");
    name = at;
    while (name[-1] != ' ')
        name--;
    *len = (int)(at - name);
    return (name);
}

void device_say_no_device(const device_t *d, const char *reason,
    const WGPUStringView *text)
{
    bridge_native_say(d->api, BRIDGE_LOG_ERR, "no device: %s: %.*s", reason,
        (int)text->length, text->data);
}

void device_say_no_surface(const device_t *d, const char *reason)
{
    bridge_native_say(d->api, BRIDGE_LOG_ERR, "no surface: %s", reason);
}

void device_say_fallback(const device_t *d, fallback_t item, uint32_t asked,
    uint32_t used)
{
    int asked_len = 0;
    int used_len = 0;
    const char *asked_name = find_name(FALLBACKS[item][1], asked, &asked_len);
    const char *used_name = find_name(FALLBACKS[item][1], used, &used_len);

    bridge_native_say(d->api, BRIDGE_LOG_WARN,
        "no %s %.*s: the frame uses %.*s", FALLBACKS[item][0], asked_len,
        asked_name, used_len, used_name);
}
