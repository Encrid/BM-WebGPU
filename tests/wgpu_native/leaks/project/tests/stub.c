/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

static char object;
static WGPUTextureFormat format = WGPUTextureFormat_BGRA8Unorm;
static WGPUFeatureName feature = WGPUFeatureName_Depth32FloatStencil8;

#define ONE(type) ((type)(void *)&object)

WGPUDevice stub_device(void)
{
    return (ONE(WGPUDevice));
}

WGPUAdapter stub_adapter(void)
{
    return (ONE(WGPUAdapter));
}

WGPUInstance wgpuCreateInstance(
    [[maybe_unused]] WGPUInstanceDescriptor const *descriptor)
{
    return (ONE(WGPUInstance));
}

WGPUSurface wgpuInstanceCreateSurface([[maybe_unused]] WGPUInstance instance,
    [[maybe_unused]] WGPUSurfaceDescriptor const *descriptor)
{
    return (ONE(WGPUSurface));
}

void wgpuSurfaceGetCurrentTexture([[maybe_unused]] WGPUSurface surface,
    WGPUSurfaceTexture *surfaceTexture)
{
    surfaceTexture->texture = ONE(WGPUTexture);
    surfaceTexture->status = WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal;
}

WGPUStatus wgpuSurfaceGetCapabilities([[maybe_unused]] WGPUSurface surface,
    [[maybe_unused]] WGPUAdapter adapter,
    WGPUSurfaceCapabilities *capabilities)
{
    capabilities->formatCount = 1;
    capabilities->formats = &format;
    return (WGPUStatus_Success);
}

WGPUQueue wgpuDeviceGetQueue([[maybe_unused]] WGPUDevice device)
{
    return (ONE(WGPUQueue));
}

WGPUBuffer wgpuDeviceCreateBuffer([[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUBufferDescriptor const *descriptor)
{
    return (ONE(WGPUBuffer));
}

WGPUTexture wgpuDeviceCreateTexture([[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUTextureDescriptor const *descriptor)
{
    return (ONE(WGPUTexture));
}

WGPUTextureView wgpuTextureCreateView([[maybe_unused]] WGPUTexture texture,
    [[maybe_unused]] WGPUTextureViewDescriptor const *descriptor)
{
    return (ONE(WGPUTextureView));
}

WGPUSampler wgpuDeviceCreateSampler([[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUSamplerDescriptor const *descriptor)
{
    return (ONE(WGPUSampler));
}

WGPUQuerySet wgpuDeviceCreateQuerySet([[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUQuerySetDescriptor const *descriptor)
{
    return (ONE(WGPUQuerySet));
}

WGPUShaderModule wgpuDeviceCreateShaderModule(
    [[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUShaderModuleDescriptor const *descriptor)
{
    return (ONE(WGPUShaderModule));
}

WGPUShaderModule wgpuDeviceCreateShaderModuleSpirV(
    [[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUShaderModuleDescriptorSpirV const *descriptor)
{
    return (ONE(WGPUShaderModule));
}

WGPUShaderModule wgpuDeviceCreateShaderModuleTrusted(
    [[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUShaderModuleDescriptor const *descriptor,
    [[maybe_unused]] WGPUShaderRuntimeChecks runtimeChecks)
{
    return (ONE(WGPUShaderModule));
}

WGPUBindGroupLayout wgpuDeviceCreateBindGroupLayout(
    [[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUBindGroupLayoutDescriptor const *descriptor)
{
    return (ONE(WGPUBindGroupLayout));
}

WGPUPipelineLayout wgpuDeviceCreatePipelineLayout(
    [[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUPipelineLayoutDescriptor const *descriptor)
{
    return (ONE(WGPUPipelineLayout));
}

WGPURenderPipeline wgpuDeviceCreateRenderPipeline(
    [[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPURenderPipelineDescriptor const *descriptor)
{
    return (ONE(WGPURenderPipeline));
}

WGPUComputePipeline wgpuDeviceCreateComputePipeline(
    [[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUComputePipelineDescriptor const *descriptor)
{
    return (ONE(WGPUComputePipeline));
}

WGPUBindGroupLayout wgpuRenderPipelineGetBindGroupLayout(
    [[maybe_unused]] WGPURenderPipeline renderPipeline,
    [[maybe_unused]] uint32_t groupIndex)
{
    return (ONE(WGPUBindGroupLayout));
}

WGPUBindGroupLayout wgpuComputePipelineGetBindGroupLayout(
    [[maybe_unused]] WGPUComputePipeline computePipeline,
    [[maybe_unused]] uint32_t groupIndex)
{
    return (ONE(WGPUBindGroupLayout));
}

WGPUBindGroup wgpuDeviceCreateBindGroup([[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUBindGroupDescriptor const *descriptor)
{
    return (ONE(WGPUBindGroup));
}

WGPUCommandEncoder wgpuDeviceCreateCommandEncoder(
    [[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPUCommandEncoderDescriptor const *descriptor)
{
    return (ONE(WGPUCommandEncoder));
}

WGPURenderPassEncoder wgpuCommandEncoderBeginRenderPass(
    [[maybe_unused]] WGPUCommandEncoder commandEncoder,
    [[maybe_unused]] WGPURenderPassDescriptor const *descriptor)
{
    return (ONE(WGPURenderPassEncoder));
}

WGPUComputePassEncoder wgpuCommandEncoderBeginComputePass(
    [[maybe_unused]] WGPUCommandEncoder commandEncoder,
    [[maybe_unused]] WGPUComputePassDescriptor const *descriptor)
{
    return (ONE(WGPUComputePassEncoder));
}

WGPUCommandBuffer wgpuCommandEncoderFinish(
    [[maybe_unused]] WGPUCommandEncoder commandEncoder,
    [[maybe_unused]] WGPUCommandBufferDescriptor const *descriptor)
{
    return (ONE(WGPUCommandBuffer));
}

WGPURenderBundleEncoder wgpuDeviceCreateRenderBundleEncoder(
    [[maybe_unused]] WGPUDevice device,
    [[maybe_unused]] WGPURenderBundleEncoderDescriptor const *descriptor)
{
    return (ONE(WGPURenderBundleEncoder));
}

WGPURenderBundle wgpuRenderBundleEncoderFinish(
    [[maybe_unused]] WGPURenderBundleEncoder renderBundleEncoder,
    [[maybe_unused]] WGPURenderBundleDescriptor const *descriptor)
{
    return (ONE(WGPURenderBundle));
}

void wgpuAdapterGetFeatures([[maybe_unused]] WGPUAdapter adapter,
    WGPUSupportedFeatures *features)
{
    features->featureCount = 1;
    features->features = &feature;
}

void wgpuDeviceGetFeatures([[maybe_unused]] WGPUDevice device,
    WGPUSupportedFeatures *features)
{
    features->featureCount = 1;
    features->features = &feature;
}

WGPUStatus wgpuAdapterGetInfo([[maybe_unused]] WGPUAdapter adapter,
    WGPUAdapterInfo *info)
{
    info->vendorID = 1;
    return (WGPUStatus_Success);
}

void wgpuInstanceRelease([[maybe_unused]] WGPUInstance instance)
{
}

void wgpuSurfaceRelease([[maybe_unused]] WGPUSurface surface)
{
}

void wgpuQueueRelease([[maybe_unused]] WGPUQueue queue)
{
}

void wgpuBufferRelease([[maybe_unused]] WGPUBuffer buffer)
{
}

void wgpuTextureRelease([[maybe_unused]] WGPUTexture texture)
{
}

void wgpuTextureViewRelease([[maybe_unused]] WGPUTextureView textureView)
{
}

void wgpuSamplerRelease([[maybe_unused]] WGPUSampler sampler)
{
}

void wgpuQuerySetRelease([[maybe_unused]] WGPUQuerySet querySet)
{
}

void wgpuShaderModuleRelease([[maybe_unused]] WGPUShaderModule shaderModule)
{
}

void wgpuBindGroupLayoutRelease(
    [[maybe_unused]] WGPUBindGroupLayout bindGroupLayout)
{
}

void wgpuPipelineLayoutRelease(
    [[maybe_unused]] WGPUPipelineLayout pipelineLayout)
{
}

void wgpuRenderPipelineRelease(
    [[maybe_unused]] WGPURenderPipeline renderPipeline)
{
}

void wgpuComputePipelineRelease(
    [[maybe_unused]] WGPUComputePipeline computePipeline)
{
}

void wgpuBindGroupRelease([[maybe_unused]] WGPUBindGroup bindGroup)
{
}

void wgpuCommandEncoderRelease(
    [[maybe_unused]] WGPUCommandEncoder commandEncoder)
{
}

void wgpuRenderPassEncoderRelease(
    [[maybe_unused]] WGPURenderPassEncoder renderPassEncoder)
{
}

void wgpuComputePassEncoderRelease(
    [[maybe_unused]] WGPUComputePassEncoder computePassEncoder)
{
}

void wgpuCommandBufferRelease(
    [[maybe_unused]] WGPUCommandBuffer commandBuffer)
{
}

void wgpuRenderBundleEncoderRelease(
    [[maybe_unused]] WGPURenderBundleEncoder renderBundleEncoder)
{
}

void wgpuRenderBundleRelease([[maybe_unused]] WGPURenderBundle renderBundle)
{
}

void wgpuSurfaceCapabilitiesFreeMembers(
    [[maybe_unused]] WGPUSurfaceCapabilities surfaceCapabilities)
{
}

void wgpuSupportedFeaturesFreeMembers(
    [[maybe_unused]] WGPUSupportedFeatures supportedFeatures)
{
}

void wgpuAdapterInfoFreeMembers([[maybe_unused]] WGPUAdapterInfo adapterInfo)
{
}
