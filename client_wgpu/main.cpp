#include <stdio.h>
#include <emscripten/wasm_worker.h>
#include "types.hpp"
#include "render.hpp"
#include "locking.hpp"

#include "webgpu/lib_webgpu.cpp"
#include "webgpu/lib_webgpu_cpp20.cpp"
#include "render.cpp"
#include "locking.cpp"
#include "math.h"


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
    emscripten_get_element_css_size("canvas", &renderMeta.cssDims[0],  &renderMeta.cssDims[1]);
    renderMeta.screenDims = renderMeta.cssDims * ratio;
}
EM_BOOL wheelEvt(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData)
{
    __atomic_fetch_add((unsigned int*)&renderMeta.scrollDirty, 1, __ATOMIC_RELAXED);
    vec2d toAdd = vec2d{(f64)-wheelEvent->deltaX,(f64)wheelEvent->deltaY };
    renderMeta.offsInches += toAdd / 96.0 / renderMeta.scale;
    __atomic_fetch_add((unsigned int*)&renderMeta.scrollDirty, 1, __ATOMIC_RELAXED);
  return true;
}

void updateCanvasMeta()
{
  __atomic_fetch_add((unsigned int*)&renderMeta.renderDirty, 1, __ATOMIC_RELAXED);
  updateCanvasMetaInternal();
  __atomic_fetch_add((unsigned int*)&renderMeta.renderDirty, 1, __ATOMIC_RELAXED);
}



struct TouchCtxPt
{
    vec2d coords;
    u64 id;
};

vec2d getBarycenter(vec2d *pts, usize numPts)
{
  vec2d pt{};

  for(u64 i = 0; i <numPts; i++ )
  {
    pt += pts[i];
  }
  pt /= (f64) numPts;
  return pt;
}
f64 magScale(vec2d *pts, usize num)
{
  if(num != 2) return 1.0;
  return smag(pts[1] - pts[0]);
}

struct TouchCtx
{
    u64 numTouches;
    TouchCtxPt start[2];
};

TouchCtx tCtx{};

constexpr f64 epsilon = 0.000001;

EM_BOOL consumeTouch(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
  switch(eventType)
  {
    case EMSCRIPTEN_EVENT_TOUCHSTART:
    {
      
      u64 iterNum = 0;
      iterNum--;
      while (tCtx.numTouches != 2 && (++iterNum) != touchEvent->numTouches)
      { 
          if (tCtx.start[0].id == touchEvent->touches[iterNum].identifier && tCtx.numTouches == 1)
          {
            continue;
          }
          tCtx.start[tCtx.numTouches].coords = vec2d{(double)touchEvent->touches[iterNum].clientX, (double)touchEvent->touches[iterNum].clientY};
          tCtx.start[tCtx.numTouches].id = touchEvent->touches[iterNum].identifier;
          tCtx.numTouches++;
      }
      break;
    }
    case EMSCRIPTEN_EVENT_TOUCHMOVE:
    {
      vec2d pts[2];
      pts[0] = tCtx.start[0].coords;
      pts[1] = tCtx.start[1].coords;
      vec2d opts[2];
      opts[0] = tCtx.start[0].coords;
      opts[1] = tCtx.start[1].coords;
      for (u64 i = 0; i < touchEvent->numTouches; i++)
      {
        for(u64 j = 0; j < tCtx.numTouches; j++)
        {
          if(tCtx.start[j].id == touchEvent->touches[i].identifier)
          {
            pts[j] =  vec2d{(double)touchEvent->touches[i].clientX, (double)touchEvent->touches[i].clientY};
            break;
          }
        }
      }
      auto fCenter = getBarycenter(pts, tCtx.numTouches);
      auto oCenter = getBarycenter(opts, tCtx.numTouches);
      auto diff = (oCenter - fCenter) / 96.0 / renderMeta.scale;
      


      auto scaleF = magScale(pts, tCtx.numTouches);
      auto scaleO = magScale(opts, tCtx.numTouches);
      auto scale = renderMeta.scale;
      if (tCtx.numTouches == 2){
        scale *= sqrt((scaleF+epsilon) / (scaleO+epsilon));
        diff = ((oCenter / renderMeta.scale) - (fCenter / scale)) / 96.0;
      } 

      diff[0] = -diff[0];
      tCtx.start[0].coords = pts[0];
      tCtx.start[1].coords = pts[1];
      __atomic_fetch_add((unsigned int*)&renderMeta.scrollDirty, 1, __ATOMIC_RELAXED);
    renderMeta.offsInches += diff;
    renderMeta.scale = scale;
    __atomic_fetch_add((unsigned int*)&renderMeta.scrollDirty, 1, __ATOMIC_RELAXED);
    break;
    }
    case EMSCRIPTEN_EVENT_TOUCHCANCEL:
    case EMSCRIPTEN_EVENT_TOUCHEND:
    {
      b usedOne = true;
      b usedTwo = true;
      for(u64 iterNum = 0; iterNum != touchEvent->numTouches; iterNum++)
      {
        for(u64 i = 0; i < tCtx.numTouches; i++)
        {
          if (tCtx.start[i].id == touchEvent->touches[iterNum].identifier && touchEvent->touches[iterNum].isChanged)
          {
            usedOne = usedOne && !(i == 0) ;
            usedTwo = usedTwo && !(i == 1);
            break;
          }
        }
      }
      if(tCtx.numTouches == 2 && !usedOne)
      {
        tCtx.start[0] = tCtx.start[1];
      }
      tCtx.numTouches -= !usedOne;
      tCtx.numTouches -= !usedTwo;
      
      break;
    }
  }
  return true;
}


EM_BOOL nextAnimCb(double time, void *pData)
{
  atomicSetValueNotify(&renderMeta.frameIdx, renderMeta.frameIdx + 1);
  
  emscripten_request_animation_frame(nextAnimCb, 0);
  return false;
}

EM_JS(void, exitIncompatible, (), {
  document.querySelector("body").innerHTML="Please use a WebGPU-compatible browser.";
});

extern "C"
int main()
{
    renderState = 0;
    renderMeta = {};
    renderMeta.scale = 1.0;
    if(!navigator_gpu_available())
    {
      exitIncompatible();
      return -1;
    }
    updateCanvasMeta();
    emscripten_request_animation_frame(nextAnimCb, 0);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, true, (em_ui_callback_func)updateCanvasMeta);
    emscripten_set_wheel_callback("canvas", 0, true, wheelEvt);
    emscripten_set_touchstart_callback("canvas", 0, true, consumeTouch);
    emscripten_set_touchend_callback("canvas", 0, true, consumeTouch);
    emscripten_set_touchmove_callback("canvas", 0, true, consumeTouch);
    emscripten_set_touchcancel_callback("canvas", 0, true, consumeTouch);
  auto worker = emscripten_malloc_wasm_worker(4096);
  renderWorker = worker;
  emscripten_wasm_worker_post_function_v(worker, renderMain);
  //nextStep();
  waitUntilValAsync<callPostCanvas>(&renderState, 1);
}
