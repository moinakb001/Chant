#pragma once
#include "types.hpp"
#include <emscripten/wasm_worker.h>

typedef void (*waitFnType)(int *, unsigned int, int, void *);
typedef void (*cbFn)(void*);


template <auto Fn, typename ...T>
static inline u<1> waitAsyncRaw(u32 *addr, u32 val, T ...args)
{
    u32 result;
    u32 arr[sizeof...(args)];
    EM_ASM({
        let prom = Atomics["waitAsync"](HEAP32, arguments[0] >> 2, arguments[1]);
        let realArgs = Array.prototype.slice.call(arguments, 4, arguments.length);
        let resultAddr = arguments[2] >> 2;
        let realFn = arguments[3];
        let fnSig = realArgs.length == 0 ? "" : "i".repeat(realArgs.length);
        if(prom.async)
            prom.value.then(()=>{
                console.log("promise finished");
                Module["dynCall"]("v" + fnSig, realFn, realArgs);
            });
        HEAP32[resultAddr] = !!prom.async;
    }, (void*)addr, (void*)val, &result, (void*)Fn, (void*)args...);
    return (u<1>)result;
}

template<auto Fn, typename ...T>
static inline void waitUntilValAsync(u32 *pData, u32 expected, T... args)
{
    u32 fin;
    while ((fin = __atomic_load_n((unsigned int*)pData, __ATOMIC_RELAXED)) != expected)
    {
        if(waitAsyncRaw<waitUntilValAsync<Fn, typeof(args)...>>(pData, fin, pData, expected, args...)){
            return;
        }
    }
    Fn(args...);
}

template<auto Fn, typename ...T>
static inline void waitLockAcquireAsync(u32 *pLock, T... args)
{
    u32 fin;
    while ((fin = __atomic_exchange_n((unsigned int*)pLock, 1, __ATOMIC_RELAXED)) != 0)
    {
        if(waitAsyncRaw<waitLockAcquireAsync<Fn>>(pLock, fin, pLock, args...)){
            return;
        }
    }
    Fn(args...);
}

static inline void atomicSetValueNotify(u32 *pData, u32 value)
{
    __atomic_store_n((unsigned int*)pData, value, __ATOMIC_RELAXED);
    __builtin_wasm_memory_atomic_notify((int*)pData, (unsigned int)-1);
}