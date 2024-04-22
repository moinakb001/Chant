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
R"(
    struct scaleInfo {
        offset : vec2<f32>,
        screenInches : vec2<f32>,
        scale : f32,
    };
    struct vertOut {
        @builtin(position) pos: vec4<f32>,
        @location(0) color: vec4<f32>,
    };
    @group(0) @binding(0) var<uniform> offs : scaleInfo;
    @vertex
    fn vmain(@builtin(vertex_index) vertexIndex : u32) -> vertOut {
        var out: vertOut;
        var pos = array<vec2<f32>, 6>(
            vec2<f32>(-0.5, 0.5),
            vec2<f32>(-0.5, -0.5),
            vec2<f32>(0.5, -0.5),
            vec2<f32>(0.5, -0.5),
            vec2<f32>(0.5, 0.5),
            vec2<f32>(-0.5, 0.5)
        );
        var cPosScaled = pos[vertexIndex] + 0.5;
        out.pos = vec4<f32>((pos[vertexIndex] * 2.0 + 2.0 * offs.offset) * offs.scale / offs.screenInches, 0.0, 1.0);
        out.color = vec4<f32>(cPosScaled, cPosScaled.x * cPosScaled.y ,1.0);
        return out;
    }
    @fragment
    fn fmain(inp: vertOut) -> @location(0) vec4<f32> {
        return inp.color;
    }
)";

scaleInfo cur = {vec2f{}, vec2f{}, 1.f};

u32 highestSeen = 1;
u32 lastScroll = 1;
vec2f lastVelocity{};
vec2f lastPosition{};
vec2f addedOffset{};
typedef EM_BOOL (*rafCbFn)(double time, void *userData);

void renderCreateSwapChain(renderCtx *pCtx)
{
    WGpuCanvasConfiguration config = WGPU_CANVAS_CONFIGURATION_DEFAULT_INITIALIZER;
    config.device = pCtx->device;
    config.format = pCtx->texFmt;
    wgpu_canvas_context_configure(pCtx->canvasCtx, &config);
}

void wscb(WGpuQueue queue, void *userData);
f64 last = 0.0;
constexpr f64 touchDecel = 2.0;
constexpr f64 wheelDecel = 1000.0;

b renderFrame(f64 now, renderCtx *pCtx)
{
    b isDirty = false;
    auto delta = now - last;
    last = now;
    auto fps = 1000.0/delta;
    emscripten_request_animation_frame((rafCbFn)renderFrame, pCtx);
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
            cur.screenInches = vec2f{(f32)local.screenInches[0], (f32)local.screenInches[1]};
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
        cur.offset = vec2f{(f32)local.offsInches[0], (f32)local.offsInches[1]};
        cur.scale = local.scale;
        if(prev != lastScroll)
        {
            lastScroll = prev;
            isDirty = true;
            lastVelocity = (cur.offset - lastPosition) / delta;
        }
        else
        {
            constexpr f64 eps =  0.0000001;
            auto scaleSquared = local.scale * local.scale;
            if(smag(lastVelocity) < (eps * scaleSquared)) lastVelocity = vec2f{};
            else
            {
                f64 factor = local.isTouch ? touchDecel : wheelDecel;
                isDirty = true;
                addedOffset += lastVelocity * delta;
                lastVelocity *=  exp(- factor * (delta / 1000.f));
            }
            
        }
        lastPosition = cur.offset;
        cur.offset += addedOffset;
        
    }
    if(!isDirty) {
        return true;
    }
    
    if(fps < 50.0) printf("fps %f\n", fps);
    
    if(pCtx->canvasView != 0) wgpu_object_destroy(pCtx->canvasView);
    pCtx->canvasView = wgpu_canvas_context_get_current_texture_view(pCtx->canvasCtx);
    WGpuCommandEncoder encoder = wgpu_device_create_command_encoder(pCtx->device, 0);
    WGpuRenderPassColorAttachment colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_DEFAULT_INITIALIZER;

  colorAttachment.view = pCtx->canvasView;

  WGpuRenderPassDescriptor passDesc = {};
  passDesc.numColorAttachments = 1;
  passDesc.colorAttachments = &colorAttachment;

  wgpu_queue_write_buffer(pCtx->queue, pCtx->offsBuffer, 0, &cur, sizeof(scaleInfo));
    
  WGpuRenderPassEncoder pass = wgpu_command_encoder_begin_render_pass(encoder, &passDesc);
  wgpu_render_pass_encoder_set_pipeline(pass, pCtx->renderPipeline);
  wgpu_render_pass_encoder_set_bind_group(pass, 0, pCtx->bindGroup, 0, 0);
  wgpu_render_pass_encoder_draw(pass, 6, 1, 0, 0);
  wgpu_render_pass_encoder_end(pass);

  WGpuCommandBuffer commandBuffer = wgpu_command_encoder_finish(encoder);
  wgpu_queue_submit_one_and_destroy(pCtx->queue, commandBuffer);

  //wgpu_queue_set_on_submitted_work_done_callback(pCtx->queue, wscb, pCtx);
  //emscripten_request_animation_frame((rafCbFn)renderFrame, pCtx);

  return false; // Render just one frame, static content
}

void wscb(WGpuQueue queue, void *userData)
{
    auto pCtx = (renderCtx*)userData;
    //wgpu_object_destroy(pCtx->canvasView);
    //emscripten_request_animation_frame((rafCbFn)renderFrame, pCtx);
    renderFrame(emscripten_get_now(), pCtx);
    return;
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
}

void renderPipelineInit(WGpuDevice result, renderCtx *pCtx)
{
    pCtx->device = result;
    pCtx->queue = wgpu_device_get_queue(pCtx->device);
    {
        WGpuBufferDescriptor desc;
        desc.size = sizeof(scaleInfo);
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