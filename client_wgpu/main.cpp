#include <stdio.h>
#include <emscripten/wasm_worker.h>
#include "types.hpp"
#include "render.hpp"
#include "locking.hpp"

#include "webgpu/lib_webgpu.cpp"
#include "webgpu/lib_webgpu_cpp20.cpp"
#include "render.cpp"
#include "locking.cpp"
#include "fonts.cpp"
#include "math.h"
#include "inputs.hpp"

InputQueue<10> globalInput{};

EM_JS(void, postCanvas, (int worker, const char *canvas), {
  var proxyCanvasNew = document.querySelector(UTF8ToString(canvas)).transferControlToOffscreen();
  _wasmWorkers[worker].postMessage({
    canvas: proxyCanvasNew
  }, [proxyCanvasNew]);
});

EM_BOOL wheelEvt(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData)
{
  InputEvent evt{};
  evt.type = wheelEvent->mouse.ctrlKey ? InputEventType::Zoom : InputEventType::Scroll;
  evt.pos = vec2d{(f64)wheelEvent->deltaX,(f64)wheelEvent->deltaY };
  evt.pos /= 96.0;
  inputAddEvent(&globalInput, evt);
  return true;
}

void updateCanvasMeta()
{
    double ratio = emscripten_get_device_pixel_ratio();
    vec2d cssDims{};
    emscripten_get_element_css_size("canvas", &cssDims[0],  &cssDims[1]);
    f64 dpi = 96.0 * ratio;
    vec2d screenDims = cssDims * ratio;
    printf("screenDims %f %f\n", screenDims[0], screenDims[1]);
    __atomic_store_n((unsigned int*)&renderMeta.renderDirty, (unsigned int)(renderMeta.renderDirty + 1u), __ATOMIC_RELAXED);
    renderMeta.dpi = dpi;
    renderMeta.screenDims = screenDims;
    __atomic_store_n((unsigned int*)&renderMeta.renderDirty, (unsigned int)(renderMeta.renderDirty + 1u), __ATOMIC_RELAXED);
}

EM_BOOL consumeTouch(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
  InputEventType evtT;

  switch(eventType)
  {
    case EMSCRIPTEN_EVENT_TOUCHSTART:
    {
      evtT = InputEventType::TouchBegin;
      break;
    }
    case EMSCRIPTEN_EVENT_TOUCHMOVE:
    {
      evtT = InputEventType::TouchMove;
      break;
    }
    case EMSCRIPTEN_EVENT_TOUCHCANCEL:
    case EMSCRIPTEN_EVENT_TOUCHEND:
    {
      evtT = InputEventType::TouchEnd;
      break;
    }
    default:
    {
      return true;
    }
  }
  for(usize i = 0; i < touchEvent->numTouches; i++)
  {
    InputEvent evt{};
    if(!touchEvent->touches[i].isChanged)
      continue;
    evt.type = evtT;
    evt.pos = vec2d{(f64)touchEvent->touches[i].clientX, (f64)touchEvent->touches[i].clientY};
    evt.pos /= 96.0;
    evt.identifier = touchEvent->touches[i].identifier;
    inputAddEvent(&globalInput, evt);
  }
  return true;
}

extern "C"
int main()
{
    renderState = 0;
    renderMeta = {};
    if(!navigator_gpu_available())
    {
      EM_ASM({
        document.querySelector("body").innerHTML="Please use a WebGPU-compatible browser.";
      });
      return -1;
    }
    updateCanvasMeta();
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, true, (em_ui_callback_func)updateCanvasMeta);
    emscripten_set_wheel_callback("canvas", 0, true, wheelEvt);
    emscripten_set_touchstart_callback("canvas", 0, true, consumeTouch);
    emscripten_set_touchend_callback("canvas", 0, true, consumeTouch);
    emscripten_set_touchmove_callback("canvas", 0, true, consumeTouch);
    emscripten_set_touchcancel_callback("canvas", 0, true, consumeTouch);
    auto worker = emscripten_malloc_wasm_worker(4096);
    renderWorker = worker;
    fontTest();
    emscripten_wasm_worker_post_function_v(worker, renderMain);
    waitUntilValAsync<postCanvas>(&renderState, 1, renderWorker, "canvas");
    return 0;
}
