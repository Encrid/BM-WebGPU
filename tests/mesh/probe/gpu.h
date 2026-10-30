/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef GPU_H
    #define GPU_H

    #include <stddef.h>
    #include <stdint.h>

    #include <webgpu/webgpu.h>
    #include <webgpu/wgpu.h>

constexpr uint32_t SIZE = 64;
constexpr uint32_t ROW = 256;
constexpr uint32_t BAND = 16;

typedef struct {
    uint32_t format;
    uint32_t samples;
    uint32_t depth;
} gpu_spec_t;

typedef struct {
    WGPUTexture color;
    WGPUTextureView color_view;
    WGPUTexture multi;
    WGPUTextureView multi_view;
    WGPUTexture depth;
    WGPUTextureView depth_view;
    WGPUBuffer read;
    WGPUBuffer read_depth;
    gpu_spec_t spec;
} gpu_frame_t;

typedef struct {
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    gpu_frame_t frame;
    uint32_t own_buffers;
    uint32_t own_textures;
    uint32_t own_views;
    bool small;
} gpu_t;

typedef struct {
    float color[4];
    float depth;
    uint32_t stencil;
} gpu_clear_t;

typedef struct {
    uint8_t color[SIZE * SIZE * 4];
    float depth[SIZE * SIZE];
    uint8_t stencil[SIZE * SIZE];
} gpu_image_t;

typedef struct {
    size_t buffers;
    size_t textures;
    size_t views;
    size_t groups;
    size_t samplers;
    size_t pipelines;
    size_t shaders;
    size_t layouts;
    size_t pipeline_layouts;
} gpu_counts_t;

bool gpu_open(gpu_t *gpu, bool small);
bool gpu_reads_depth(const gpu_spec_t *spec);
bool gpu_renew(gpu_t *gpu, bool small);
void gpu_close(gpu_t *gpu);
void gpu_make_frame(gpu_t *gpu, const gpu_spec_t *spec);
void gpu_drop_frame(gpu_t *gpu);
void gpu_clear(gpu_t *gpu, const gpu_clear_t *clear);
void gpu_read(gpu_t *gpu, gpu_image_t *image);
void gpu_count(const gpu_t *gpu, gpu_counts_t *counts);
WGPUStringView gpu_text(const char *text);

#endif
