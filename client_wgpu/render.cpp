#include "render.hpp"
#include "math.h"
CanvasMeta renderMeta;
void renderProxyRequestAnimFrame(renderCtx *pCtx);
typedef void (*tempFnPtr)(int);

EM_JS(void, setOffscreenSize, (int width, int height), {
    self.globalOffscreen.width = width;
    self.globalOffscreen.height = height;
});

const char *pShader =
    "@group(0) @binding(0) var<uniform> offs : vec3f;\n"
    "@vertex\n"
    "fn vmain(@builtin(vertex_index) vertexIndex : u32) -> @builtin(position) vec4<f32> {\n"
      "var pos = array<vec2<f32>, 3>(\n"
        "vec2<f32>(0.0, 0.5),\n"
        "vec2<f32>(-0.5, -0.5),\n"
        "vec2<f32>(0.5, -0.5)\n"
      ");\n"

      "return vec4<f32>((pos[vertexIndex] + offs.xy * 2.0) * offs.z, 0.0, 1.0);\n"
    "}\n"
    "@fragment\n"
    "fn fmain() -> @location(0) vec4<f32> {\n"
      "return vec4<f32>(1.0, 0.5, 0.3, 1.0);\n"
    "}\n";
u32 highestSeen = 1;
u32 lastScroll = 1;
typedef EM_BOOL (*rafCbFn)(double time, void *userData);

void renderCreateSwapChain(renderCtx *pCtx)
{
    WGpuCanvasConfiguration config = WGPU_CANVAS_CONFIGURATION_DEFAULT_INITIALIZER;
    config.device = pCtx->device;
    config.format = pCtx->texFmt;
    wgpu_canvas_context_configure(pCtx->canvasCtx, &config);
}
float cur[3] = {0.f, 0.f, 1.f};
void wscb(WGpuQueue queue, void *userData);
f64 last = 0.0;

b renderFrame(f64 now, renderCtx *pCtx)
{
    b isDirty = false;
    {
        CanvasMeta local;
        u32 next =__atomic_load_n((unsigned int*)&renderMeta.renderDirty, __ATOMIC_RELAXED);
        u32 prev = 0;
        do{
            prev = next;
            local = renderMeta;
            next = __atomic_load_n((unsigned int*)&renderMeta.renderDirty, __ATOMIC_RELAXED);
        } while(prev != next && (prev & 1));
        if(prev != highestSeen)
        {
            highestSeen = prev;
            setOffscreenSize(local.screenDims[0], local.screenDims[1]);
            renderCreateSwapChain(pCtx);
            isDirty = true;
        }
    }
    {
        CanvasMeta local;
        u32 next =__atomic_load_n((unsigned int*)&renderMeta.scrollDirty, __ATOMIC_RELAXED);
        u32 prev = 0;
        do{
            prev = next;
            local = renderMeta;
            next = __atomic_load_n((unsigned int*)&renderMeta.scrollDirty, __ATOMIC_RELAXED);
        } while(prev != next && (prev & 1));
        if(prev != lastScroll)
        {
            lastScroll = prev;
            isDirty = true;
        }
        auto inScreen = local.offsInches * local.dpi / local.screenDims;
        cur[0] = inScreen[0];
        cur[1] = inScreen[1];
        cur[2] = local.scale;
    }
    if(!isDirty) {
        emscripten_request_animation_frame((rafCbFn)renderFrame, pCtx);
        return true;
    }
    auto delta = now - last;
    last = now;
    //printf("fps %f\n", 1000.0/delta);
    pCtx->canvasView = wgpu_canvas_context_get_current_texture_view(pCtx->canvasCtx);
    WGpuCommandEncoder encoder = wgpu_device_create_command_encoder(pCtx->device, 0);
    WGpuRenderPassColorAttachment colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_DEFAULT_INITIALIZER;

  colorAttachment.view = pCtx->canvasView;

  WGpuRenderPassDescriptor passDesc = {};
  passDesc.numColorAttachments = 1;
  passDesc.colorAttachments = &colorAttachment;

  wgpu_queue_write_buffer(pCtx->queue, pCtx->offsBuffer, 0, cur, 12);
    
  WGpuRenderPassEncoder pass = wgpu_command_encoder_begin_render_pass(encoder, &passDesc);
  wgpu_render_pass_encoder_set_pipeline(pass, pCtx->renderPipeline);
  wgpu_render_pass_encoder_set_bind_group(pass, 0, pCtx->bindGroup, 0, 0);
  wgpu_render_pass_encoder_draw(pass, 3, 1, 0, 0);
  wgpu_render_pass_encoder_end(pass);

  WGpuCommandBuffer commandBuffer = wgpu_command_encoder_finish(encoder);
  wgpu_queue_submit_one_and_destroy(pCtx->queue, commandBuffer);

  wgpu_queue_set_on_submitted_work_done_callback(pCtx->queue, wscb, pCtx);

  return false; // Render just one frame, static content
}
u32 curFrameIdx = 0;

void renderAnimLoop(renderCtx *pCtx)
{
    while(1)
    {
        u32 oldFrame = curFrameIdx;
        u32 newFrame = __atomic_load_n((unsigned int *)&renderMeta.frameIdx, __ATOMIC_RELAXED);
        if(oldFrame != newFrame)
        {
            b val = renderFrame(emscripten_get_now(), pCtx);
            curFrameIdx = newFrame;
            if(!val) return;
        }
        __builtin_wasm_memory_atomic_wait32((int *)&renderMeta.frameIdx, curFrameIdx, (u32) -1);

    }
}
void wscb(WGpuQueue queue, void *userData)
{
    auto pCtx = (renderCtx*)userData;
    wgpu_object_destroy(pCtx->canvasView);
    emscripten_request_animation_frame((rafCbFn)renderFrame, pCtx);
    return;
    renderAnimLoop((renderCtx*)pCtx);
}


void renderInitDone(WGpuDevice device, WGpuPipelineBase pipeline, renderCtx *pCtx)
{
    pCtx->renderPipeline = pipeline;
    pCtx->renderLayout = wgpu_render_pipeline_get_bind_group_layout(pCtx->renderPipeline, 0);
    {
        WGpuBindGroupEntry entry{};
        entry.binding = 0;
        entry.resource = pCtx->offsBuffer;
        pCtx->bindGroup = wgpu_device_create_bind_group(pCtx->device, pCtx->renderLayout, &entry, 1);
    }
    renderCreateSwapChain(pCtx);
    emscripten_request_animation_frame((rafCbFn)renderFrame, pCtx);
    return;
    renderAnimLoop(pCtx);
}

void renderPipelineInit(WGpuDevice result, renderCtx *pCtx)
{
    pCtx->device = result;
    pCtx->queue = wgpu_device_get_queue(pCtx->device);
    {
        WGpuBufferDescriptor desc;
        desc.size = 12;
        desc.usage = WGPU_BUFFER_USAGE_UNIFORM | WGPU_BUFFER_USAGE_COPY_DST;
        desc.mappedAtCreation = false;
        pCtx->offsBuffer = wgpu_device_create_buffer(pCtx->device, &desc);
    }
    {
        WGpuShaderModuleDescriptor shaderModuleDesc = {};
        shaderModuleDesc.code = pShader;
        pCtx->shade = wgpu_device_create_shader_module(pCtx->device, &shaderModuleDesc);
    }

    {
        WGpuRenderPipelineDescriptor renderPipelineDesc = WGPU_RENDER_PIPELINE_DESCRIPTOR_DEFAULT_INITIALIZER;
        WGpuColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_DEFAULT_INITIALIZER;

        renderPipelineDesc.vertex.module = pCtx->shade;
        renderPipelineDesc.vertex.entryPoint = "vmain";
        renderPipelineDesc.fragment.module = pCtx->shade;
        renderPipelineDesc.fragment.entryPoint = "fmain";

        colorTarget.format = pCtx->texFmt;
        renderPipelineDesc.fragment.numTargets = 1;
        renderPipelineDesc.fragment.targets = &colorTarget;
        wgpu_device_create_render_pipeline_async(pCtx->device, &renderPipelineDesc, (WGpuCreatePipelineCallback)renderInitDone, pCtx);
    }
}

void renderDeviceInit(WGpuAdapter res, renderCtx *pCtx)
{
    WGpuDeviceDescriptor deviceDesc = {};
    pCtx->adapter = res;
    wgpu_adapter_request_device_async(pCtx->adapter, &deviceDesc, (WGpuRequestDeviceCallback)renderPipelineInit, pCtx);

}

extern "C" EMSCRIPTEN_KEEPALIVE
int renderAdapterInit(renderCtx *pCtx)
{
    WGpuRequestAdapterOptions options = {};
    options.powerPreference = WGPU_POWER_PREFERENCE_HIGH_PERFORMANCE;

    pCtx->canvasCtx = wgpu_canvas_get_webgpu_context("");
    pCtx->texFmt = navigator_gpu_get_preferred_canvas_format();
    navigator_gpu_request_adapter_async(&options, (WGpuRequestAdapterCallback) renderDeviceInit, pCtx);
    return 0;
}

EM_JS(void, renderHijackMessage, (void *ctx), {
    addEventListener('message', (data) => {
      var md = data["data"];
      if ("canvas" in md)
      {
        self.globalOffscreen = md["canvas"];
        console.log(self.globalOffscreen);
        Module["_renderAdapterInit"](ctx);
      }
    });
});


renderCtx ctx;
u32 renderState;
u32 renderWorker;

void renderMain()
{
    renderHijackMessage(&ctx);
    atomicSetValueNotify(&renderState, 1);
}