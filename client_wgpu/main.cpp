#include "webgpu/lib_webgpu.h"
#include "webgpu/lib_webgpu.cpp"
#include "webgpu/lib_webgpu_cpp20.cpp"
#include <stdio.h>


WGpuAdapter adapter;
WGpuCanvasContext canvasContext;
WGpuDevice device;
WGpuQueue queue;
WGpuRenderPipeline renderPipeline;

EM_BOOL raf(double time, void *userData)
{
  WGpuCommandEncoder encoder = wgpu_device_create_command_encoder(device, 0);

  WGpuRenderPassColorAttachment colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_DEFAULT_INITIALIZER;
  colorAttachment.view = wgpu_texture_create_view(wgpu_canvas_context_get_current_texture(canvasContext), 0);

  WGpuRenderPassDescriptor passDesc = {};
  passDesc.numColorAttachments = 1;
  passDesc.colorAttachments = &colorAttachment;

  WGpuRenderPassEncoder pass = wgpu_command_encoder_begin_render_pass(encoder, &passDesc);
  wgpu_render_pass_encoder_set_pipeline(pass, renderPipeline);
  wgpu_render_pass_encoder_draw(pass, 3, 1, 0, 0);
  wgpu_render_pass_encoder_end(pass);

  WGpuCommandBuffer commandBuffer = wgpu_command_encoder_finish(encoder);

  wgpu_queue_submit_one_and_destroy(queue, commandBuffer);

  return EM_FALSE; // Render just one frame, static content
}

EM_BOOL resizeStuff()
{
  WGpuCanvasConfiguration config = WGPU_CANVAS_CONFIGURATION_DEFAULT_INITIALIZER;
  double w, h, ratio;
  emscripten_get_element_css_size("canvas", &w, &h);
  ratio = emscripten_get_device_pixel_ratio();
  emscripten_set_canvas_element_size("canvas", w * ratio, h * ratio);
  config.device = device;
  config.format = navigator_gpu_get_preferred_canvas_format();
  canvasContext = wgpu_canvas_get_webgpu_context("canvas");
  wgpu_canvas_context_configure(canvasContext, &config);

  emscripten_request_animation_frame_loop(raf, 0);

  return 1;
}

void ObtainedWebGpuDevice(WGpuDevice result, void *userData)
{
  device = result;
  queue = wgpu_device_get_queue(device);

  const char *shader =
    "@vertex\n"
    "fn vmain(@builtin(vertex_index) vertexIndex : u32) -> @builtin(position) vec4<f32> {\n"
      "var pos = array<vec2<f32>, 3>(\n"
        "vec2<f32>(0.0, 0.5),\n"
        "vec2<f32>(-0.5, -0.5),\n"
        "vec2<f32>(0.5, -0.5)\n"
      ");\n"

      "return vec4<f32>(pos[vertexIndex], 0.0, 1.0);\n"
    "}\n"
    "@fragment\n"
    "fn fmain() -> @location(0) vec4<f32> {\n"
      "return vec4<f32>(1.0, 0.5, 0.3, 1.0);\n"
    "}\n";


  WGpuShaderModuleDescriptor shaderModuleDesc = {};
  shaderModuleDesc.code = shader;
  WGpuShaderModule shade = wgpu_device_create_shader_module(device, &shaderModuleDesc);

  WGpuRenderPipelineDescriptor renderPipelineDesc = WGPU_RENDER_PIPELINE_DESCRIPTOR_DEFAULT_INITIALIZER;
  renderPipelineDesc.vertex.module = shade;
  renderPipelineDesc.vertex.entryPoint = "vmain";
  renderPipelineDesc.fragment.module = shade;
  renderPipelineDesc.fragment.entryPoint = "fmain";

  WGpuColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_DEFAULT_INITIALIZER;
  colorTarget.format = navigator_gpu_get_preferred_canvas_format();
  renderPipelineDesc.fragment.numTargets = 1;
  renderPipelineDesc.fragment.targets = &colorTarget;

  renderPipeline = wgpu_device_create_render_pipeline(device, &renderPipelineDesc);

  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, (em_ui_callback_func)resizeStuff);

  resizeStuff();
}

void ObtainedWebGpuAdapter(WGpuAdapter result, void *userData)
{
  adapter = result;
  WGpuDeviceDescriptor deviceDesc = {};
  wgpu_adapter_request_device_async(adapter, &deviceDesc, ObtainedWebGpuDevice, 0);
}

extern "C"
int main()
{
  WGpuRequestAdapterOptions options = {};
  options.powerPreference = WGPU_POWER_PREFERENCE_LOW_POWER;

  navigator_gpu_request_adapter_async(&options, ObtainedWebGpuAdapter, 0);
}
