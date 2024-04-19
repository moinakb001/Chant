#pragma once
#include "types.hpp"
#include "webgpu/lib_webgpu.h"

struct CanvasMeta
{
    double scrX;
    double scrY;
    double dpi;
    u32 renderDirty;
    u32 frameIdx;
};
struct renderCtx
{
    WGpuAdapter adapter;
    WGpuDevice device;
    WGPU_TEXTURE_FORMAT texFmt;
    WGpuQueue queue;
    WGpuRenderPipeline renderPipeline;
    WGpuShaderModule shade;
    WGpuCanvasContext canvasCtx;
    WGpuTextureView canvasView;
    WGpuBindGroupLayout renderLayout;
    WGpuBuffer offsBuffer;
    WGpuBindGroup bindGroup;
    double_int53_t offsMapping;
};

extern u32 renderState;
extern u32 renderWorker;
extern CanvasMeta renderMeta;
extern u32 curFrameIdx;