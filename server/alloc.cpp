#include "alloc.hpp"

static inline void arenaAddChunk(Arena *pArena, u64 size)
{
    buf_t<u8> pData = {(u8*) mmap(NULL, sizeof(ArenaChunk) + size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0), sizeof(ArenaChunk) + size};
    ArenaChunk *pChunk = (ArenaChunk*) &pData.arr[size];
    pChunk->chunkSize = size;
    pChunk->pNext = pArena->pChunkHead;
    pArena->pChunkHead = pChunk;

}

buf_t<u8> arenaAlloc(Arena *pArena, u64 size, u64 align )
{
    u64 base = 0;
    u64 alignMask = align - 1;
    
    b needAlloc = (align >= backingPageSize) || (size >= backingPageSize) || (pArena->curOffs == 0);
    if (!needAlloc)
    {
        needAlloc = (((pArena->curOffs + backingPageSize + alignMask) & (~alignMask)) + size) <= pArena->pChunkHead->chunkSize;
    }
    if (needAlloc && pArena->curOffs != 0)
    {
        u64 toAdd = pArena->pChunkHead->chunkSize - pArena->curOffs;
        pArena->curOffs = 0 ;
        pArena->curStack += toAdd;
    }
    u64 allocAlign = ((backingPageSize + alignMask + pArena->curOffs) & (~alignMask)) - backingPageSize - pArena->curOffs;
    u64 allocSize = allocAlign + size;
    if (needAlloc)
    {
        u64 newAllocSize = allocSize < pArena->minChunkSize ? pArena->minChunkSize : allocSize;
        newAllocSize = arenaIdealChunkAlloc(newAllocSize);
        arenaAddChunk(pArena, newAllocSize);
    }
    auto buf = bufFromChunk(pArena->pChunkHead);
    pArena->curStack += allocSize;
    if (pArena->curStack > pArena->watershed) pArena->watershed = pArena->curStack;
    u64 finalOffs = (allocSize + pArena->curOffs) ;
    buf = {&buf.arr[pArena->curOffs + allocAlign], size};
    pArena->curOffs = finalOffs < pArena->pChunkHead->chunkSize ? finalOffs : 0;
    return buf;
}
void arenaFreeUntil(Arena *pArena, void *ptr)
{
    u64 offs = pArena->curOffs;
    auto curChunk = pArena->pChunkHead;

    while (curChunk != NULL)
    {
        if (offs != 0)
        {
            offs  = curChunk->chunkSize - offs;
        }
        if (arenaAddrInChunk(curChunk, ptr, 0, offs))
        {
            goto found;
        }
        offs = 0;
        curChunk = curChunk->pNext;
    }
    return;
found:
    offs = pArena->curOffs;
    while (pArena->pChunkHead != NULL)
    {
        u64 offs = pArena->curOffs;
        if (offs != 0)
        {
            offs  = curChunk->chunkSize - offs;
        }
        b inChunk = arenaAddrInChunk(pArena->pChunkHead, ptr);
        u64 offset = inChunk ? 0 : arenaGetChunkOffset(pArena->pChunkHead, ptr);
        while (arenaAddrInChunk(pArena->pChunkHead, pArena->pStackHead, offset, offs))
        {
            pArena->pStackHead = pArena->pStackHead->pNext;
            pArena->curFreeStack++;
        }
        while (arenaAddrInChunk(pArena->pChunkHead, pArena->pArenaHead, offset, offs))
        {
            pArena->pArenaHead = pArena->pArenaHead->pNext;
            arenaReset(&pArena->pArenaHead->cur);
            pArena->curFreeArena++;
        }

        pArena->curStack += offset - pArena->curOffs;
        pArena->curOffs = offset;
        if (offset == 0)
        {
            arenaRemoveChunk(pArena);
        }
        if (inChunk)
        {
            return;
        }
    }
    return;
}