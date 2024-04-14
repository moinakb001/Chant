#pragma once
#include "types.hpp"

struct ArenaChunk;
struct ArenaChunk
{
    u64 chunkSize;
    ArenaChunk *pNext;
};
constexpr u64 backingPageSize = 4096;
static inline buf_t<u8> bufFromChunk(ArenaChunk *pChunk)
{
    return {((u8*)pChunk) - pChunk->chunkSize, pChunk->chunkSize};
}
struct ArenaRef;
struct StackRef;
struct StackRef
{
    StackRef *pNext;
};


struct Arena {
    ArenaChunk *pChunkHead;
    ArenaRef *pArenaHead;
    StackRef *pStackHead;
    u64 minChunkSize;
    u64 curOffs;
    u64 curStack;
    u64 watershed;

    u64 curFreeArena;
    u64 curFreeStack;
};

struct ArenaRef
{
    Arena     cur;
    ArenaRef *pNext;
};

static inline u64 arenaGetChunkOffset(ArenaChunk *pChunk, void *pAddr)
{
    u8* addr = (u8*)pAddr;
    auto buf = bufFromChunk(pChunk);
    return (addr - &buf.arr[0]);
}

static inline b arenaAddrInChunk(ArenaChunk *pChunk, void *pAddr, u64 bOffs = 0, u64 eOffs = 0)
{
    u64 offs = arenaGetChunkOffset(pChunk, pAddr);
    return (offs < (pChunk->chunkSize - eOffs)) && (offs >= bOffs);
}

static inline void arenaRemoveChunk(Arena *pArena)
{
    ArenaChunk *pNext = pArena->pChunkHead->pNext;
    auto buf = bufFromChunk(pArena->pChunkHead);
    munmap(&buf.arr[0], sizeof(ArenaChunk) + buf.num);
    pArena->pChunkHead = pNext;
}
static inline u64 arenaIdealChunkAlloc(u64 amt)
{
    u64 ret =  ((amt + sizeof(ArenaChunk) + backingPageSize - 1llu) & (~(backingPageSize - 1llu))) - sizeof(ArenaChunk);
    return ret;
}

static inline void arenaInit(Arena *pArena, u64 chunkSize)
{
    pArena->pChunkHead = NULL;
    pArena->pArenaHead = NULL;
    pArena->pStackHead = NULL;
    pArena->minChunkSize = arenaIdealChunkAlloc(chunkSize);
    pArena->curOffs = 0;
    pArena->curStack = 0;
    pArena->watershed = 0;
    pArena->curFreeStack = 0;
    pArena->curFreeArena = 0;
}

buf_t<u8> arenaAlloc(Arena *pArena, u64 size, u64 align = 16);
void      arenaFreeUntil(Arena *pArena, void *ptr);

template<typename T>
static inline buf_t<T> arenaAllocObjArr(Arena *pArena, u64 num, u64 align = 16)
{
    auto buf = arenaAlloc(pArena, sizeof(T) * num, align);
    return {(T *)buf.arr, buf.num};
}

template<typename T>
static inline T * arenaAllocObj(Arena *pArena, u64 align = 16)
{
    return &arenaAllocObjArr<T>(pArena, 1, align).arr[0];
}

static inline void arenaSaveStack(Arena *pArena)
{
    auto pPtr = arenaAllocObj<StackRef>(pArena);
    pPtr->pNext = pArena->pStackHead;
    pArena->pStackHead = pPtr;
}
static inline void arenaRestoreStack(Arena *pArena)
{
    if(pArena->pStackHead == NULL)
    {
        return;
    }
    if (pArena->curFreeStack == 0)
    {
        arenaFreeUntil(pArena, pArena->pStackHead);
    }
    pArena->curFreeStack--;
}

static inline Arena *arenaPushArena(Arena *pArena)
{
    auto pPtr = arenaAllocObj<ArenaRef>(pArena);
    pPtr->pNext = pArena->pArenaHead;
    arenaInit(&pPtr->cur, pArena->minChunkSize);
    return &pPtr->cur;
}

static inline void arenaPopArena(Arena *pArena)
{
    if (pArena->pArenaHead == NULL)
    {
        return;
    }
    if (pArena->curFreeArena == 0)
    {
        arenaFreeUntil(pArena, pArena->pArenaHead);
    }
    pArena->curFreeArena--;
}

static inline void arenaReset(Arena *pArena)
{
    while (pArena->pArenaHead != NULL)
    {
        arenaReset(&pArena->pArenaHead->cur);
        pArena->pArenaHead = pArena->pArenaHead->pNext;
    }
    pArena->pStackHead = NULL;
    pArena->curStack = 0;
    pArena->curOffs = 0;
    while (pArena->pChunkHead != NULL)
    {
        arenaRemoveChunk(pArena);
    }
    pArena->curFreeStack = 0;
    pArena->curFreeArena = 0;
}

