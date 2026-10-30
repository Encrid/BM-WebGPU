/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <webgpu_contract.h>

int main(void)
{
    webgpu_frame_t frame = { .state = WEBGPU_FRAME_NO_DEVICE };

    if (frame.state != WEBGPU_FRAME_NO_DEVICE)
        return (1);
    return (sizeof(webgpu_frame_t_desc) > 1 ? 0 : 1);
}
