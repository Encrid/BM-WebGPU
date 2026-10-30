/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "device_inside.h"

constexpr uint32_t TEXT_HEAD = sizeof(text_head_t);
constexpr uint32_t NAME_COUNT = 6;

static const char *const ERROR_NAMES[NAME_COUNT] = {
    [0] = "unknown",
    [WGPUErrorType_NoError] = "unknown",
    [WGPUErrorType_Validation] = "validation",
    [WGPUErrorType_OutOfMemory] = "out of memory",
    [WGPUErrorType_Internal] = "internal",
    [WGPUErrorType_Unknown] = "unknown"
};

static const char *const LOST_NAMES[NAME_COUNT] = {
    [0] = "unknown",
    [WGPUDeviceLostReason_Unknown] = "unknown",
    [WGPUDeviceLostReason_Destroyed] = "destroyed",
    [WGPUDeviceLostReason_CallbackCancelled] = "callback cancelled",
    [WGPUDeviceLostReason_FailedCreation] = "failed creation",
    [5] = "unknown"
};

static bool keep_text(device_t *d, const text_head_t *head,
    const WGPUStringView *text)
{
    room_t *r = &d->room;
    size_t need = (size_t)TEXT_HEAD + head->len;
    uint32_t at = atomic_load(&r->used);

    do {
        if (need > r->size - at) {
            atomic_fetch_add(&r->missed, 1);
            return (false);
        }
    } while (!atomic_compare_exchange_strong(&r->used, &at,
        (uint32_t)(at + need)));
    memcpy(r->bytes + at, head, TEXT_HEAD);
    memcpy(r->bytes + at + TEXT_HEAD, text->data, head->len);
    return (true);
}

void device_keep_error([[maybe_unused]] const WGPUDevice *device,
    WGPUErrorType type, WGPUStringView message, void *userdata1,
    [[maybe_unused]] void *userdata2)
{
    text_head_t head = { TEXT_ERROR, type, (uint32_t)message.length };

    keep_text(userdata1, &head, &message);
}

void device_keep_lost([[maybe_unused]] const WGPUDevice *device,
    WGPUDeviceLostReason reason, WGPUStringView message, void *userdata1,
    [[maybe_unused]] void *userdata2)
{
    device_t *d = userdata1;
    text_head_t head = { TEXT_LOST, reason, (uint32_t)message.length };

    keep_text(d, &head, &message);
    atomic_store(&d->room.lost, true);
}

bool device_drain_room(device_t *d)
{
    room_t *r = &d->room;
    text_head_t head = {};
    uint32_t missed = atomic_exchange(&r->missed, 0);

    for (uint32_t at = 0; at < r->used; at += head.len) {
        memcpy(&head, r->bytes + at, TEXT_HEAD);
        at += TEXT_HEAD;
        head.code = head.code < NAME_COUNT ? head.code : 0;
        if (head.kind == TEXT_LOST)
            bridge_native_say(d->api, BRIDGE_LOG_ERR, "device lost: %s: %.*s",
                LOST_NAMES[head.code], (int)head.len, r->bytes + at);
        else
            bridge_native_say(d->api, BRIDGE_LOG_ERR, "webgpu error: %s: %.*s",
                ERROR_NAMES[head.code], (int)head.len, r->bytes + at);
    }
    if (missed > 0)
        bridge_native_say(d->api, BRIDGE_LOG_ERR,
            "webgpu error: %u texts did not fit in error_room", missed);
    atomic_store(&r->used, 0);
    return (atomic_exchange(&r->lost, false));
}

void device_size_room(device_t *d, uint32_t size)
{
    free(d->room.bytes);
    d->room.bytes = nullptr;
    d->room.size = 0;
    if (size > 0)
        d->room.bytes = malloc(size);
    if (d->room.bytes != nullptr)
        d->room.size = size;
}
