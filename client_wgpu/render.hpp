#pragma once
#include "types.hpp"
#include "webgpu/lib_webgpu.h"
#include "inputs.hpp"

struct __attribute__ ((aligned (8))) scaleInfo  {
    vec2f offset;
    vec2f screenInches;
    f32 scale;
};

struct CanvasMeta
{
    vec2d screenDims;
    double dpi;
    u32 renderDirty;
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
    WGpuRenderBundle bundle;
};

extern u32 renderState;
extern u32 renderWorker;
extern CanvasMeta renderMeta;
extern u32 curFrameIdx;