#include <stdio.h>
#include <emscripten/wasm_worker.h>
#include "types.hpp"
#include "render.hpp"
#include "locking.hpp"

#include "webgpu/lib_webgpu.cpp"
#include "webgpu/lib_webgpu_cpp20.cpp"
#include "render.cpp"
#include "locking.cpp"


EM_JS(void, postCanvas, (int worker, char *canvas), {
  var proxyCanvasNew = document.querySelector(UTF8ToString(canvas)).transferControlToOffscreen();
  _wasmWorkers[worker].postMessage({
    canvas: proxyCanvasNew
  }, [proxyCanvasNew]);
});

void callPostCanvas()
{
  postCanvas(renderWorker, "canvas");
}

void updateCanvasMetaInternal()
{
    double ratio = emscripten_get_device_pixel_ratio();
    renderMeta.dpi = 96.0 * ratio;
    emscripten_get_element_css_size("canvas", &renderMeta.scrX, &renderMeta.scrY);
    renderMeta.scrX *= ratio;
    renderMeta.scrY *= ratio;
}

void updateCanvasMeta()
{
  __atomic_fetch_add((unsigned int*)&renderMeta.renderDirty, 1, __ATOMIC_RELAXED);
  updateCanvasMetaInternal();
  __atomic_fetch_add((unsigned int*)&renderMeta.renderDirty, 1, __ATOMIC_RELAXED);
}
EM_BOOL nextAnimCb(double time, void *pData);
void animCbNotify(u32 *ptr, u32 val)
{
  atomicSetValueNotify(ptr, val);
  
  emscripten_request_animation_frame(nextAnimCb, 0);
}

EM_BOOL nextAnimCb(double time, void *pData)
{
  //printf("waiting for ack\n", );
  waitUntilValAsync<animCbNotify>(&curFrameIdx, renderMeta.frameIdx, &renderMeta.frameIdx, renderMeta.frameIdx + 1);
  return false;
}

extern "C"
int main()
{
    renderState = 0;
    renderMeta = {};
    updateCanvasMeta();
    emscripten_request_animation_frame(nextAnimCb, 0);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, true, (em_ui_callback_func)updateCanvasMeta);
  auto worker = emscripten_malloc_wasm_worker(4096);
  renderWorker = worker;
  emscripten_wasm_worker_post_function_v(worker, renderMain);
  //nextStep();
  waitUntilValAsync<callPostCanvas>(&renderState, 1);
}
