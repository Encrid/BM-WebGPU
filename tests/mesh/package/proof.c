/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <webgpu_mesh.h>

int main(void)
{
    webgpu_mesh_settings_t settings = { .load = WEBGPU_LOAD_CLEAR };

    if (settings.load != WEBGPU_LOAD_CLEAR)
        return (1);
    return (sizeof(webgpu_mesh_settings_t_desc) > 1 ? 0 : 1);
}
