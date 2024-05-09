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
        //out.color = vec4<f32>(1.0);
        return out;
    }
    @fragment
    fn fmain(inp: vertOut) -> @location(0) vec4<f32> {
        return inp.color;
    }
)";

scaleInfo cur = {vec2f{}, vec2f{}, 1.f};

u32 highestSeen = 1;
typedef EM_BOOL (*rafCbFn)(double time, void *userData);

void lostcb(WGpuDevice device, WGPU_DEVICE_LOST_REASON deviceLostReason, const char *message NOTNULL, void *userData)
{
    printf("%s\n", message);
}

void renderCreateSwapChain(renderCtx *pCtx)
{
    WGpuCanvasConfiguration config = WGPU_CANVAS_CONFIGURATION_DEFAULT_INITIALIZER;
    config.device = pCtx->device;
    config.format = pCtx->texFmt;
    pCtx->canvasCtx = wgpu_canvas_get_webgpu_context("");
    wgpu_canvas_context_configure(pCtx->canvasCtx, &config);
}
EM_BOOL renderFrame(f64 now, renderCtx *pCtx);

f64 sub;

void wscb(WGpuQueue queue, void *userData)
{
    //printf("delta %f\n", now - sub);
    //renderFrame(0.0, (renderCtx *) userData);
    return;
}
f64 last = 0.0;
constexpr f64 touchDecel = 2.0;
constexpr f64 wheelDecel = 1000.0;
b lastDirty = false;
u32 numDropped = 0;
f64 tot = 0.0;
f64 num = 0.0;

vec2d touchPos[32];
u32 touchId[32];
vec2d touchCenter{};
usize touchNum = 0;
vec2d lastDelta{};
f64 lastDebounce = 0.0;
b useDelta = false;
#define epsilon 0.001

EM_BOOL renderFrame(f64 now, renderCtx *pCtx)
{
    now = emscripten_performance_now();
    b isDirty = false;
    auto delta = now - last;
    last = now;
    auto fps = 1000.0/delta;
    tot += delta;
    num += 1.0;
    vec2d allDelta{};
    b touchMoved = false;
    b inGotten = false;

    numDropped += (fps < 50.0);
    if(numDropped % 60 == 0)
    {
        numDropped++;
        printf("fps %f %u %f\n", fps, numDropped, 1000 * num / tot);
    }
    auto iter = inputIter(&globalInput);
    while(!inputIterFlush(iter))
    {
        auto evt = inputIterGet(iter);
        if(evt.type == InputEventType::Scroll)
        {
            inGotten = true;
            allDelta += vec2d{(f32)-evt.pos[0], (f32)evt.pos[1]} / cur.scale;
        }
        else if (evt.type == InputEventType::Zoom)
        {
            // An inch gives 2x magnification
            cur.scale *= pow(2.0, -evt.pos[1]);
        }
        else if(evt.type == InputEventType::TouchBegin)
        {
            if(touchNum == 32) goto end;
            touchCenter = (touchCenter * ((f64)touchNum))  + evt.pos;
            touchPos[touchNum] =  evt.pos;
            touchId[touchNum] = evt.identifier;
            touchNum++;
            touchCenter /= ((f64)touchNum);
        }
        else if(evt.type == InputEventType::TouchEnd)
        {
            u32 idx;
            for(idx = 0; idx < touchNum; idx++)
            {
                if(touchId[idx] == evt.identifier)
                {
                    goto found;
                }
            }
            goto end;
found:
            touchCenter = (touchCenter * ((f64)touchNum))  - evt.pos;
            touchNum--;
            touchId[idx] = touchId[touchNum];
            touchPos[idx] = touchPos[touchNum];
            if(touchNum != 0) touchCenter /= ((f64)touchNum);
        }
        else if(evt.type == InputEventType::TouchMove)
        {
            u32 idx;
            vec2d newPos, delta;
            vec2d covtd = vec2d{cur.offset[0], -cur.offset[1]};
            f64 magDiff;
            for(idx = 0; idx < touchNum; idx++)
            {
                if(touchId[idx] == evt.identifier)
                {
                    goto foundx;
                }
            }
            goto end;
foundx:
            newPos = (touchCenter * ((f64)touchNum))  - touchPos[idx] + evt.pos;
            newPos /= ((f64)touchNum);
            magDiff = (smag(newPos-evt.pos) + epsilon) / (smag(touchCenter - touchPos[idx]) + epsilon);
            magDiff = sqrt(magDiff);
            delta = (newPos) - (touchCenter);
            touchCenter = newPos;
            cur.scale *= magDiff;
            delta /= cur.scale;
            touchPos[idx] = evt.pos;
            inGotten = true;
            touchMoved = true;
            allDelta += vec2d{(f32)delta[0], (f32)-delta[1]};
        }
        isDirty = true;
end:
        iter = inputIterAdvance(iter);
    }
    useDelta = useDelta && (!inGotten) && ((touchNum == 0) ||((now - lastDebounce) < 100.0));
    if(useDelta && touchNum == 0)
    {
        isDirty = true;
        allDelta = lastDelta * delta;
        lastDelta *= pow(2, -delta / 300.0);
        if(smag(lastDelta) < epsilon) useDelta = false;
    }
    if(touchMoved)
    {
        useDelta = true;
        lastDebounce = now;
        lastDelta = allDelta / delta;
    }
    
    
    {
        CanvasMeta local;
        u32 next =__atomic_load_n((unsigned int*)&renderMeta.renderDirty, __ATOMIC_RELAXED);
        if(next == highestSeen) goto loopDone;
        u32 prev = 0;
        do{
            prev = next;
            local = renderMeta;
            next = __atomic_load_n((unsigned int*)&renderMeta.renderDirty, __ATOMIC_RELAXED);
        } while(prev != next && (prev & 1));
        highestSeen = prev;
        setOffscreenSize(local.screenDims[0], local.screenDims[1]);
        auto inc = local.screenDims / local.dpi;
        cur.screenInches = vec2f{(f32)inc[0], (f32)inc[1]};
        isDirty = true;
    }
loopDone:
    

    if(!isDirty) {
        lastDirty = isDirty;
        return true;
    }
    cur.offset += vec2f{(f32)allDelta[0], (f32)allDelta[1]};

    lastDirty = isDirty;
    WGpuCommandEncoder encoder = wgpu_device_create_command_encoder(pCtx->device, 0);
    WGpuRenderPassColorAttachment colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_DEFAULT_INITIALIZER;
    colorAttachment.loadOp = WGPU_LOAD_OP_CLEAR;
    colorAttachment.view = wgpu_canvas_context_get_current_texture_view(pCtx->canvasCtx);

    WGpuRenderPassDescriptor passDesc = WGPU_RENDER_PASS_DESCRIPTOR_DEFAULT_INITIALIZER;
    passDesc.numColorAttachments = 1;
    passDesc.colorAttachments = &colorAttachment;

    wgpu_queue_write_buffer(pCtx->queue, pCtx->offsBuffer, 0, &cur, sizeof(scaleInfo));
    WGpuRenderPassEncoder pass = wgpu_command_encoder_begin_render_pass(encoder, &passDesc);
    wgpu_render_pass_encoder_execute_bundles(pass, &pCtx->bundle, 1);
    wgpu_render_pass_encoder_end(pass);
    WGpuCommandBuffer commandBuffer = wgpu_command_encoder_finish(encoder);
    wgpu_queue_submit_one_and_destroy(pCtx->queue, commandBuffer);
    return true;

    
}
void renderInitDone(WGpuDevice device, WGpuPipelineBase pipeline, renderCtx *pCtx)
{
    pCtx->renderPipeline = pipeline;
    pCtx->renderLayout = wgpu_render_pipeline_get_bind_group_layout(pCtx->renderPipeline, 0);
    {
        WGpuBindGroupEntry entry{};
        entry.binding = 0;
        entry.resource = pCtx->offsBuffer;
        pCtx->bindGroup =    wgpu_device_create_bind_group(pCtx->device, pCtx->renderLayout, &entry, 1);
    }
    {
        WGpuRenderBundleEncoderDescriptor desc = {};
        desc.numColorFormats = 1;
        desc.colorFormats = &pCtx->texFmt;
        desc.sampleCount = 1;
        auto enc = wgpu_device_create_render_bundle_encoder(pCtx->device, &desc);
        wgpu_render_bundle_encoder_set_pipeline(enc, pCtx->renderPipeline);
        wgpu_render_bundle_encoder_set_bind_group(enc, 0, pCtx->bindGroup, 0, 0);
        wgpu_render_bundle_encoder_draw(enc, 6, 1, 0, 0);
        pCtx->bundle = wgpu_render_bundle_encoder_finish(enc);
    }
    //renderCreateSwapChain(pCtx);
    last = emscripten_get_now() - (1000.0 / 60.0);
    emscripten_request_animation_frame_loop((rafCbFn)renderFrame, pCtx);
    return;
}

void renderPipelineInit(WGpuDevice result, renderCtx *pCtx)
{
    pCtx->device = result;
    wgpu_device_set_lost_callback(result, lostcb, NULL);
    pCtx->queue = wgpu_device_get_queue(pCtx->device);
    renderCreateSwapChain(pCtx);
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
    options.powerPreference = WGPU_POWER_PREFERENCE_LOW_POWER;

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