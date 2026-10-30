/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "mesh_inside.h"

static const char SHADER[] =
    "struct Block { matrix: mat4x4f, clear: vec4f, lx: f32, ly: f32,\n"
    "    lz: f32, cr: f32, cg: f32, cb: f32, ar: f32, ag: f32, ab: f32,\n"
    "    size: vec2f }\n"
    "struct Part { @location(0) position: vec3f,\n"
    "    @location(1) normal: vec3f, @location(2) uv: vec2f,\n"
    "    @location(3) color: vec4f }\n"
    "struct Record { @location(4) m0: vec4f, @location(5) m1: vec4f,\n"
    "    @location(6) m2: vec4f, @location(7) m3: vec4f,\n"
    "    @location(8) color: vec4f, @location(9) bias_size: vec2f,\n"
    "    @location(10) shading: u32 }\n"
    "struct Second { @location(11) position: vec3f,\n"
    "    @location(12) normal: vec3f, @location(13) uv: vec2f,\n"
    "    @location(14) color: vec4f }\n"
    "struct Out { @builtin(position) position: vec4f,\n"
    "    @location(0) color: vec4f, @location(1) uv: vec2f,\n"
    "    @location(2) @interpolate(flat) shading: u32 }\n"
    "@group(0) @binding(0) var<uniform> block: Block;\n"
    "@group(1) @binding(0) var image: texture_2d<f32>;\n"
    "@group(1) @binding(1) var smp: sampler;\n"
    "const CORNERS = array(vec2f(-1, -1), vec2f(1, -1), vec2f(1, 1),\n"
    "    vec2f(-1, -1), vec2f(1, 1), vec2f(-1, 1));\n"
    "fn place(r: Record, p: Part) -> Out {\n"
    "    let model = mat4x4f(r.m0, r.m1, r.m2, r.m3);\n"
    "    var o: Out;\n"
    "    o.position = block.matrix * model * vec4f(p.position, 1.0);\n"
    "    o.position.z -= r.bias_size.x * o.position.w;\n"
    "    o.color = r.color * p.color;\n"
    "    if (r.shading == 1u) {\n"
    "        let n = (model * vec4f(p.normal, 0.0)).xyz;\n"
    "        let light = vec3f(block.lx, block.ly, block.lz);\n"
    "        let lit = vec3f(block.ar, block.ag, block.ab)\n"
    "            + vec3f(block.cr, block.cg, block.cb)\n"
    "            * max(0.0, dot(n, light));\n"
    "        o.color = vec4f(o.color.rgb * lit, o.color.a);\n"
    "    }\n"
    "    o.uv = p.uv;\n"
    "    o.shading = r.shading;\n"
    "    return o;\n"
    "}\n"
    "fn widen(o: Out, side: vec2f, size: f32) -> Out {\n"
    "    var w = o;\n"
    "    w.position += vec4f(side * size / block.size * o.position.w,\n"
    "        0.0, 0.0);\n"
    "    return w;\n"
    "}\n"
    "fn trim(a: Out, b: Out) -> Out {\n"
    "    var t = a;\n"
    "    if (a.position.z < 0.0 && b.position.z >= 0.0) {\n"
    "        t.position = mix(a.position, b.position,\n"
    "            a.position.z / (a.position.z - b.position.z));\n"
    "    }\n"
    "    return t;\n"
    "}\n"
    "@vertex fn vs_thin(p: Part, r: Record) -> Out {\n"
    "    return place(r, p);\n"
    "}\n"
    "@vertex fn vs_point(@builtin(vertex_index) i: u32, p: Part,\n"
    "    r: Record) -> Out {\n"
    "    var corners = CORNERS;\n"
    "    return widen(place(r, p), corners[i], r.bias_size.y);\n"
    "}\n"
    "@vertex fn vs_line(@builtin(vertex_index) i: u32, p: Part,\n"
    "    r: Record, q: Second) -> Out {\n"
    "    var corners = CORNERS;\n"
    "    let first = place(r, p);\n"
    "    let second = place(r, Part(q.position, q.normal, q.uv, q.color));\n"
    "    let a = trim(first, second);\n"
    "    let b = trim(second, a);\n"
    "    var d = (b.position.xy / b.position.w\n"
    "        - a.position.xy / a.position.w) * block.size;\n"
    "    if (dot(d, d) == 0.0) {\n"
    "        d = vec2f(1.0, 0.0);\n"
    "    }\n"
    "    var o = a;\n"
    "    if (corners[i].x > 0.0) {\n"
    "        o = b;\n"
    "    }\n"
    "    return widen(o, normalize(vec2f(-d.y, d.x)) * corners[i].y,\n"
    "        r.bias_size.y);\n"
    "}\n"
    "@fragment fn fs_main(i: Out) -> @location(0) vec4f {\n"
    "    let t = textureSample(image, smp, i.uv);\n"
    "    if (i.shading == 2u) {\n"
    "        return i.color * t;\n"
    "    }\n"
    "    return i.color;\n"
    "}\n";

static const WGPUPrimitiveTopology TOPOLOGIES[5] = {
    WGPUPrimitiveTopology_TriangleList, WGPUPrimitiveTopology_LineList,
    WGPUPrimitiveTopology_PointList, WGPUPrimitiveTopology_TriangleList,
    WGPUPrimitiveTopology_TriangleList
};

static const WGPUBlendState BLEND = {
    { WGPUBlendOperation_Add, WGPUBlendFactor_SrcAlpha,
        WGPUBlendFactor_OneMinusSrcAlpha },
    { WGPUBlendOperation_Add, WGPUBlendFactor_One, WGPUBlendFactor_Zero }
};

static const WGPUBindGroupLayoutEntry BINDINGS[3] = {
    { .binding = 0, .visibility = WGPUShaderStage_Vertex,
        .buffer = { .type = WGPUBufferBindingType_Uniform,
        .hasDynamicOffset = true, .minBindingSize = sizeof(mesh_block_t) } },
    { .binding = 0, .visibility = WGPUShaderStage_Fragment,
        .texture = { .sampleType = WGPUTextureSampleType_Float,
        .viewDimension = WGPUTextureViewDimension_2D } },
    { .binding = 1, .visibility = WGPUShaderStage_Fragment,
        .sampler = { .type = WGPUSamplerBindingType_Filtering } }
};

static const WGPUBindGroupLayoutDescriptor GROUPS[2] = {
    { .label = { "webgpu:mesh:block", WGPU_STRLEN }, .entryCount = 1,
        .entries = BINDINGS },
    { .label = { "webgpu:mesh:image", WGPU_STRLEN }, .entryCount = 2,
        .entries = BINDINGS + 1 }
};

static WGPUPrimitiveState get_primitive(uint32_t shape)
{
    const webgpu_mesh_settings_t *set = &mesh_get_state()->settings;
    WGPUFrontFace front = set->clockwise ? WGPUFrontFace_CW
        : WGPUFrontFace_CCW;
    WGPUCullMode cull = shape < SHAPE_WIDE_LINES ? (WGPUCullMode)set->cull
        : WGPUCullMode_None;
    WGPUPrimitiveState primitive = { .topology = TOPOLOGIES[shape],
        .frontFace = front, .cullMode = cull };

    return (primitive);
}

static WGPUDepthStencilState get_depth_state(uint32_t depth)
{
    const mesh_state_t *s = mesh_get_state();
    WGPUOptionalBool write = depth & 2 ? WGPUOptionalBool_True
        : WGPUOptionalBool_False;
    WGPUCompareFunction compare = depth & 1
        ? (WGPUCompareFunction)s->settings.depth_compare
        : WGPUCompareFunction_Always;
    WGPUDepthStencilState tested = { .format = s->frame.depth_format,
        .depthWriteEnabled = write, .depthCompare = compare };

    return (tested);
}

static WGPURenderPipeline make_pipeline(uint32_t layout, uint32_t shape,
    uint32_t depth)
{
    const mesh_state_t *s = mesh_get_state();
    WGPUVertexAttribute parts[15];
    WGPUVertexBufferLayout buffers[4];
    WGPUColorTargetState target = { .format = s->frame.format,
        .blend = s->settings.blend ? &BLEND : nullptr,
        .writeMask = WGPUColorWriteMask_All };
    WGPUFragmentState fragment = { .module = s->shader, .entryPoint = {
        "fs_main", WGPU_STRLEN }, .targetCount = 1, .targets = &target };
    WGPUDepthStencilState tested = get_depth_state(depth);
    WGPURenderPipelineDescriptor desc = { .label = { "webgpu:mesh:pipeline",
        WGPU_STRLEN }, .layout = s->layout,
        .vertex = mesh_set_buffers(buffers, parts, layout, shape),
        .primitive = get_primitive(shape),
        .depthStencil = s->frame.depth_format ? &tested : nullptr,
        .multisample = { .count = s->frame.samples, .mask = UINT32_MAX },
        .fragment = &fragment };

    return (wgpuDeviceCreateRenderPipeline((WGPUDevice)(uintptr_t)
        s->frame.device, &desc));
}

WGPURenderPipeline mesh_get_pipeline(uint32_t layout, uint32_t shape,
    uint32_t depth)
{
    mesh_state_t *s = mesh_get_state();
    uint32_t i = layout + 8 * (shape + 5 * depth);

    if (!s->pipelines[i])
        s->pipelines[i] = make_pipeline(layout, shape, depth);
    return (s->pipelines[i]);
}

void mesh_make_layouts(void)
{
    mesh_state_t *s = mesh_get_state();
    WGPUDevice device = (WGPUDevice)(uintptr_t)s->frame.device;
    WGPUShaderSourceWGSL wgsl = { .chain = { .sType =
        WGPUSType_ShaderSourceWGSL }, .code = { SHADER, WGPU_STRLEN } };
    WGPUShaderModuleDescriptor shader = { .nextInChain = &wgsl.chain,
        .label = { "webgpu:mesh:shader", WGPU_STRLEN } };
    WGPUPipelineLayoutDescriptor joined = { .label = { "webgpu:mesh:layout",
        WGPU_STRLEN }, .bindGroupLayoutCount = 2,
        .bindGroupLayouts = s->groups };

    s->shader = wgpuDeviceCreateShaderModule(device, &shader);
    for (uint32_t i = 0; i < 2; i++)
        s->groups[i] = wgpuDeviceCreateBindGroupLayout(device, &GROUPS[i]);
    s->layout = wgpuDeviceCreatePipelineLayout(device, &joined);
}
