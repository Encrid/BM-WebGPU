/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static const WGPUStringView ENTRIES[5] = {
    { "vs_thin", WGPU_STRLEN }, { "vs_thin", WGPU_STRLEN },
    { "vs_thin", WGPU_STRLEN }, { "vs_line", WGPU_STRLEN },
    { "vs_point", WGPU_STRLEN }
};

static const WGPUVertexStepMode STEPS[5] = { WGPUVertexStepMode_Vertex,
    WGPUVertexStepMode_Vertex, WGPUVertexStepMode_Vertex,
    WGPUVertexStepMode_Instance, WGPUVertexStepMode_Instance };

static const WGPUVertexFormat FORMATS[3] = { WGPUVertexFormat_Float32x3,
    WGPUVertexFormat_Float32x2, WGPUVertexFormat_Unorm8x4 };

static const uint64_t SIZES[3] = { 12, 8, 4 };

static const uint64_t AT_DEFAULTS[3] = { 0, 12, 20 };

static const WGPUVertexAttribute RECORD[7] = {
    { .format = WGPUVertexFormat_Float32x4, .offset = 0, .shaderLocation = 4 },
    { .format = WGPUVertexFormat_Float32x4, .offset = 16, .shaderLocation = 5 },
    { .format = WGPUVertexFormat_Float32x4, .offset = 32, .shaderLocation = 6 },
    { .format = WGPUVertexFormat_Float32x4, .offset = 48, .shaderLocation = 7 },
    { .format = WGPUVertexFormat_Float32x4, .offset = 64, .shaderLocation = 8 },
    { .format = WGPUVertexFormat_Float32x2, .offset = 80, .shaderLocation = 9 },
    { .format = WGPUVertexFormat_Uint32, .offset = 88, .shaderLocation = 10 }
};

static const WGPUVertexBufferLayout RECORDS[2] = {
    { .stepMode = WGPUVertexStepMode_Instance, .arrayStride = 0,
        .attributeCount = 7, .attributes = RECORD },
    { .stepMode = WGPUVertexStepMode_Instance,
        .arrayStride = sizeof(mesh_record_t), .attributeCount = 7,
        .attributes = RECORD }
};

static uint32_t set_parts(WGPUVertexAttribute *at, uint32_t layout,
    uint32_t base, bool from)
{
    uint32_t n = from;
    uint64_t offset = 12;

    at[0] = (WGPUVertexAttribute){ .format = WGPUVertexFormat_Float32x3,
        .shaderLocation = base };
    for (uint32_t i = 0; i < 3; i++) {
        if (((layout >> i) & 1) == from) {
            at[n] = (WGPUVertexAttribute){ .format = FORMATS[i],
                .offset = from ? offset : AT_DEFAULTS[i],
                .shaderLocation = base + 1 + i };
            n++;
        }
        offset += ((layout >> i) & 1) * SIZES[i];
    }
    return (n);
}

WGPUVertexState mesh_set_buffers(WGPUVertexBufferLayout *buffers,
    WGPUVertexAttribute *parts, uint32_t layout, uint32_t shape)
{
    bool line = shape == SHAPE_WIDE_LINES;
    uint32_t first = set_parts(parts, layout, 0, true);
    uint32_t second = line ? set_parts(parts + first, layout, 11, true) : 0;
    uint32_t left = set_parts(parts + first + second, layout, 0, false);

    left += line ? set_parts(parts + first + second + left, layout, 11,
        false) : 0;
    buffers[0] = (WGPUVertexBufferLayout){ .stepMode = STEPS[shape],
        .arrayStride = (1 + line) * STRIDES[layout], .attributeCount = first,
        .attributes = parts };
    buffers[1] = RECORDS[shape < SHAPE_WIDE_LINES];
    buffers[2] = (WGPUVertexBufferLayout){ .stepMode =
        WGPUVertexStepMode_Instance, .arrayStride = 2 * STRIDES[layout],
        .attributeCount = second, .attributes = parts + first };
    buffers[2 + line] = (WGPUVertexBufferLayout){ .stepMode =
        WGPUVertexStepMode_Vertex, .attributeCount = left,
        .attributes = parts + first + second };
    return ((WGPUVertexState){ .module = mesh_get_state()->shader,
        .entryPoint = ENTRIES[shape], .bufferCount = 2 + line + (left > 0),
        .buffers = buffers });
}
