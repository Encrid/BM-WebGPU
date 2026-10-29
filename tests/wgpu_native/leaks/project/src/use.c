/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

WGPUDevice stub_device(void);
WGPUAdapter stub_adapter(void);

static void use_surface(WGPUAdapter adapter)
{
    WGPUInstance instance = wgpuCreateInstance(nullptr);
    WGPUSurface surface = wgpuInstanceCreateSurface(instance,
        &(WGPUSurfaceDescriptor){});
    WGPUSurfaceTexture frame = {};
    WGPUSurfaceCapabilities caps = {};

    wgpuSurfaceGetCurrentTexture(surface, &frame);
    wgpuSurfaceGetCapabilities(surface, adapter, &caps);
    if (!GIVE)
        return;
    wgpuTextureRelease(frame.texture);
    wgpuSurfaceCapabilitiesFreeMembers(caps);
    wgpuSurfaceRelease(surface);
    wgpuInstanceRelease(instance);
}

static void use_memory(WGPUDevice device)
{
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device,
        &(WGPUBufferDescriptor){});
    WGPUTexture texture = wgpuDeviceCreateTexture(device,
        &(WGPUTextureDescriptor){});
    WGPUTextureView view = wgpuTextureCreateView(texture, nullptr);
    WGPUSampler sampler = wgpuDeviceCreateSampler(device, nullptr);
    WGPUQuerySet queries = wgpuDeviceCreateQuerySet(device,
        &(WGPUQuerySetDescriptor){});

    if (!GIVE)
        return;
    wgpuQuerySetRelease(queries);
    wgpuSamplerRelease(sampler);
    wgpuTextureViewRelease(view);
    wgpuTextureRelease(texture);
    wgpuBufferRelease(buffer);
    wgpuQueueRelease(queue);
}

static void use_shaders(WGPUDevice device)
{
    WGPUShaderModule wgsl = wgpuDeviceCreateShaderModule(device,
        &(WGPUShaderModuleDescriptor){});
    WGPUShaderModule spirv = wgpuDeviceCreateShaderModuleSpirV(device,
        &(WGPUShaderModuleDescriptorSpirV){});
    WGPUShaderModule trusted = wgpuDeviceCreateShaderModuleTrusted(device,
        &(WGPUShaderModuleDescriptor){}, 0);

    if (!GIVE)
        return;
    wgpuShaderModuleRelease(trusted);
    wgpuShaderModuleRelease(spirv);
    wgpuShaderModuleRelease(wgsl);
}

static void use_pipelines(WGPUDevice device)
{
    WGPUBindGroupLayout made = wgpuDeviceCreateBindGroupLayout(device,
        &(WGPUBindGroupLayoutDescriptor){});
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(device,
        &(WGPUPipelineLayoutDescriptor){});
    WGPURenderPipeline render = wgpuDeviceCreateRenderPipeline(device,
        &(WGPURenderPipelineDescriptor){});
    WGPUComputePipeline compute = wgpuDeviceCreateComputePipeline(device,
        &(WGPUComputePipelineDescriptor){});
    WGPUBindGroupLayout of_render = wgpuRenderPipelineGetBindGroupLayout(
        render, 0);
    WGPUBindGroupLayout of_compute = wgpuComputePipelineGetBindGroupLayout(
        compute, 0);
    WGPUBindGroup group = wgpuDeviceCreateBindGroup(device,
        &(WGPUBindGroupDescriptor){});

    if (!GIVE)
        return;
    wgpuBindGroupRelease(group);
    wgpuBindGroupLayoutRelease(of_compute);
    wgpuBindGroupLayoutRelease(of_render);
    wgpuComputePipelineRelease(compute);
    wgpuRenderPipelineRelease(render);
    wgpuPipelineLayoutRelease(layout);
    wgpuBindGroupLayoutRelease(made);
}

static void use_commands(WGPUDevice device)
{
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device,
        nullptr);
    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder,
        &(WGPURenderPassDescriptor){});
    WGPUComputePassEncoder work = wgpuCommandEncoderBeginComputePass(encoder,
        nullptr);
    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
    WGPURenderBundleEncoder bundler = wgpuDeviceCreateRenderBundleEncoder(
        device, &(WGPURenderBundleEncoderDescriptor){});
    WGPURenderBundle bundle = wgpuRenderBundleEncoderFinish(bundler, nullptr);

    if (!GIVE)
        return;
    wgpuRenderBundleRelease(bundle);
    wgpuRenderBundleEncoderRelease(bundler);
    wgpuCommandBufferRelease(commands);
    wgpuComputePassEncoderRelease(work);
    wgpuRenderPassEncoderRelease(pass);
    wgpuCommandEncoderRelease(encoder);
}

static void use_lists(WGPUAdapter adapter, WGPUDevice device)
{
    WGPUSupportedFeatures of_adapter = {};
    WGPUSupportedFeatures of_device = {};
    WGPUAdapterInfo info = {};

    wgpuAdapterGetFeatures(adapter, &of_adapter);
    wgpuDeviceGetFeatures(device, &of_device);
    wgpuAdapterGetInfo(adapter, &info);
    if (!GIVE)
        return;
    wgpuAdapterInfoFreeMembers(info);
    wgpuSupportedFeaturesFreeMembers(of_device);
    wgpuSupportedFeaturesFreeMembers(of_adapter);
}

int main(int argc, [[maybe_unused]] char **argv)
{
    WGPUDevice device = stub_device();
    WGPUAdapter adapter = stub_adapter();

    use_surface(adapter);
    use_memory(device);
    use_shaders(device);
    use_pipelines(device);
    use_commands(device);
    use_lists(adapter, device);
    return (argc > 1);
}
