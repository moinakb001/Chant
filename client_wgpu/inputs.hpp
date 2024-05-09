#pragma once
#include "types.hpp"
#include "locking.hpp"

enum class InputEventType: u8
{
    Scroll,
    Zoom,
    Click,
    TouchBegin,
    TouchMove,
    TouchEnd,
};


struct InputEvent
{
    InputEventType type;
    vec2d pos;
    u64 identifier;
};

struct InputEventGC
{
    u32 generation;
    InputEvent evt;
};

template<usize shift>
struct InputQueue
{
    vec_t<InputEventGC, ((usize)1) << shift> queue;
    u64 validArray[1u << (shift - 6)];
    u32 head;
    u32 tail;
};

extern InputQueue<10> globalInput;

template<usize shift>
struct InputQueueIterator
{
    InputQueue<shift> *pQueue;
    u32 max;
    u32 cur;
    u32 origCur;
};

template<usize shift>
static inline 
InputQueueIterator<shift> inputValidateEvt(InputQueueIterator<shift> iter)
{
    if (iter.cur == iter.max) return iter;
    u32 curGen = iter.cur >> shift;
    if (iter.pQueue->queue[iter.cur & ((1u << shift) - 1u)].generation != (1u + curGen))
    {
        iter.max = iter.cur;
    }
    return iter;
}


template<usize shift>
static inline 
InputQueueIterator<shift> inputIter(InputQueue<shift> *pQueue)
{
    InputQueueIterator<shift> iter;
    iter.pQueue = pQueue;
    iter.max = __atomic_load_n((unsigned int *)&pQueue->head, __ATOMIC_RELAXED);
    iter.cur = __atomic_load_n((unsigned int *)&pQueue->tail, __ATOMIC_RELAXED);
    iter.origCur = iter.cur;
    return inputValidateEvt(iter);
}

template<usize shift>
static inline 
InputQueueIterator<shift> inputIterAdvance(InputQueueIterator<shift> iter)
{
    iter.cur++;
    return inputValidateEvt(iter);
}
template<usize shift>
InputEvent inputIterGet(InputQueueIterator<shift> iter)
{
    return iter.pQueue->queue[iter.cur & ((1u << shift) - 1u)].evt;
}
template<usize shift>
static inline 
b inputIterFinished(InputQueueIterator<shift> iter)
{
    return iter.cur == iter.max;
}

template<usize shift>
static inline 
void inputReleaseEvents
(
    InputQueue<shift> *pQueue,
    u32 num
)
{
    u32 tail = __atomic_fetch_add((unsigned int *)&pQueue->tail, num, __ATOMIC_RELAXED);
    u32 head = __atomic_load_n((unsigned int *)&pQueue->head, __ATOMIC_RELAXED);
    if ((head - tail) >> shift)
    {
       // __builtin_wasm_memory_atomic_notify((int*)&pQueue->tail, (unsigned int)-1);
    }
}

template<usize shift>
static inline 
b inputIterFlush(InputQueueIterator<shift> iter)
{
    if(!inputIterFinished(iter)) return false;
    inputReleaseEvents<shift>(iter.pQueue, iter.cur - iter.origCur);
    return true;
}

template<usize shift>
static inline 
void inputAddEventCb(InputQueue<shift> *pQueue, InputEvent event, u32 idx)
{
    u32 tail = __atomic_load_n((unsigned int *)&pQueue->tail, __ATOMIC_RELAXED);
    u32 head  = idx;
loop:
    while((head - tail) >> shift)
    {
        //if (waitAsyncRaw<inputAddEventCb<shift>>(&pQueue->tail, tail, pQueue, event, head))
        {
            return;
        }
        tail = __atomic_load_n((unsigned int *)&pQueue->tail, __ATOMIC_RELAXED);
    }
    pQueue->queue[head & ((1u << shift) - 1u)].evt = event;
    __atomic_store_n((unsigned int *)&(pQueue->queue[head & ((1u << shift) - 1u)].generation), (head >> shift) + 1, __ATOMIC_RELAXED);
}

template<usize shift>
static inline 
void inputAddEvent(InputQueue<shift> *pQueue, InputEvent event)
{
    u32 head = __atomic_fetch_add((unsigned int *)&pQueue->head, 1, __ATOMIC_RELAXED);
    inputAddEventCb(pQueue, event, head);
}