/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <webgpu_device.h>

int main(void)
{
    webgpu_device_settings_t settings = { .samples = WEBGPU_SAMPLES_4 };
    webgpu_frame_t frame = { .state = WEBGPU_FRAME_NO_DEVICE };

    if (settings.samples != WEBGPU_SAMPLES_4)
        return (1);
    if (frame.state != WEBGPU_FRAME_NO_DEVICE)
        return (1);
    return (sizeof(webgpu_device_settings_t_desc) > 1 ? 0 : 1);
}
