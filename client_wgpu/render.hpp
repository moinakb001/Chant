#pragma once
#include "types.hpp"
#include "webgpu/lib_webgpu.h"

struct __attribute__ ((aligned (8))) scaleInfo  {
    vec2f offset;
    vec2f screenInches;
    f32 scale;
};

struct CanvasMeta
{
    vec2d screenDims;
    vec2d screenInches;
    vec2d cssDims;
    b isTouch;
    vec2d offsInches;
    double dpi;
    double scale;
    u32 scrollDirty;
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
};

extern u32 renderState;
extern u32 renderWorker;
extern CanvasMeta renderMeta;
extern u32 curFrameIdx;