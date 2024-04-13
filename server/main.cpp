#include <stdio.h>
#include <stdlib.h>
#include <liburing.h>
#include <unistd.h>
#include <sys/mman.h>
#include "types.hpp"
#include <sys/socket.h>
#include <netinet/in.h>

constexpr u64 ringDepth = 64;
constexpr u64 lgNumBufs =  10;
constexpr u64 numBufs = 1llu << lgNumBufs;
constexpr u64 lgNumBufSize =  16;
constexpr u64 bufSize = 1llu << lgNumBufSize;

constexpr u64 bufBgid = 1;

struct Buffer
{
    u8 data[bufSize];
};

struct BufferGroup
{
    Buffer buffers[numBufs];
};

#define AOE(c) if(c < 0) _exit(-1);

enum class OpType: u64 {
    Accept = 0,
    Read = 1,
    Write = 2,
};

struct IoCtx {
    io_uring ring;
    io_uring_buf_ring *pBufRing;
    BufferGroup *pBufferGroup;
};

struct CbResult
{
    s32 result;
    buf_t<u8> data;
    u64 ctx;
};

typedef void (*CbFn)(CbResult pResult);

struct CbInfo
{
    CbFn fn;
    u64 ctx;
};

struct CbPtr {
    union{
        struct {
            u32 ctx;
            u32 requeue: 2;
            u32 fd: 30;
        };
        u64 ptr;
    };
    
};

constexpr u64 numCbSlots = ringDepth * 5;

u64 freeMask[(numCbSlots + 63) >> 6];
CbInfo cbSlots[numCbSlots];

void initIoCtx(IoCtx *pCtx)
{
    s32 retVal;
    io_uring_params params{};
    params.flags = IORING_SETUP_COOP_TASKRUN;
    AOE(io_uring_queue_init_params(ringDepth, &pCtx->ring, &params));
    AOE(io_uring_register_files_sparse(&pCtx->ring, 1 << 5));
    pCtx->pBufRing = io_uring_setup_buf_ring(&pCtx->ring, numBufs, bufBgid, 0, (int*) &retVal);
    AOE(retVal);
    pCtx->pBufferGroup = (BufferGroup *) mmap(NULL, sizeof(BufferGroup), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    for(u64 i = 0; i < numBufs; i++)
    {
        io_uring_buf_ring_add(pCtx->pBufRing, &(pCtx->pBufferGroup->buffers[i]), bufSize, i + 1, io_uring_buf_ring_mask(numBufs), i);
    }
    io_uring_buf_ring_advance(pCtx->pBufRing, numBufs);
    return;
}
constexpr u64 INVALID_SLOT = 0-1llu;
u64 acquireCbSlot()
{
    for(u64 i = 0; i < (sizeof(freeMask) >> 3); i++)
    {
        u64 idx = __builtin_ctzll(~freeMask[i]);
        u64 adjIdx = idx + (i << 6);
        if(idx == 64) continue;
        if(adjIdx >= numCbSlots) return INVALID_SLOT;
        freeMask[i] |= (1llu << idx);
        return adjIdx;
    }
    return INVALID_SLOT;
}
void freeCbSlot(u64 slot)
{
    freeMask[slot >> 6] &= ~(1llu << (slot & 63llu));
}

void queueIoOperation(IoCtx *pCtx, CbInfo cbInf, OpType type, u64 fd, u<1> repeat, buf_t<u8> data)
{
    u64 slot = acquireCbSlot();
    CbPtr ptr{};
    ptr.ctx = (u32) slot;
    AOE(slot == INVALID_SLOT ? -1 : 0);
    cbSlots[slot] = cbInf;
    auto sqe = io_uring_get_sqe(&pCtx->ring);
    AOE(sqe == NULL ? -1 : 0);
    ptr.requeue = (type == OpType::Accept) ? 1 :  (type == OpType::Read && repeat) ? 2 : 0;
    ptr.fd = (u32)fd;
    switch(type)
    {
        case OpType::Accept:
        {
            io_uring_prep_multishot_accept_direct(sqe, (int) fd, NULL, NULL, 0);
            break;
        }
        case OpType::Read:
        {
            io_uring_prep_read(sqe, (int)fd, 0, 0, -1);
            sqe->buf_group = bufBgid;
            sqe->flags |= IOSQE_BUFFER_SELECT;
            break;
        }
        case OpType::Write:
        {
            io_uring_prep_write(sqe, (int)fd, data.arr, data.num, -1);
            break;
        }
        default: 
        {
            AOE(-1);
        }
    }
    io_uring_sqe_set_data64(sqe, ptr.ptr);
    io_uring_submit(&pCtx->ring);
}
void dequeueIoAndCb(IoCtx *pCtx)
{
    io_uring_cqe *pCqe;
    AOE(io_uring_wait_cqe(&pCtx->ring, &pCqe));
    CbPtr ptr;
    io_uring_sqe *pSqe = NULL;
    ptr.ptr = io_uring_cqe_get_data64(pCqe);
    CbInfo info = cbSlots[ptr.ctx];
    u<4> requeue = (pCqe->flags & IORING_CQE_F_MORE) ? 0 : ptr.requeue;
    if (requeue)
    {
        pSqe = io_uring_get_sqe(&pCtx->ring);
    }
    else if(ptr.requeue == 0)
    {
        freeCbSlot(ptr.ctx);
    }
    switch(requeue)
    {
        case 1:
        {
            io_uring_prep_multishot_accept_direct(pSqe, (int) ptr.fd, NULL, NULL, 0);
            io_uring_submit(&pCtx->ring);
            break;
        }
        case 2:
        {
            io_uring_prep_read(pSqe, (int)ptr.fd, 0, 0, -1);
            pSqe->buf_group = bufBgid;
            pSqe->flags |= IOSQE_BUFFER_SELECT;
            io_uring_submit(&pCtx->ring);
            break;
        }
    }
    CbResult result{};
    result.result  = pCqe->res;
    result.ctx = info.ctx;
    if(pCqe->flags & IORING_CQE_F_BUFFER)
    {
        u64 id = pCqe->flags >> 16;
        result.data.arr = (u8*) &(pCtx->pBufferGroup->buffers[id - 1]);
        result.data.num = pCqe->res;
    }
    io_uring_cqe_seen(&pCtx->ring, pCqe);
    info.fn(result);
    return;
}

void acceptCb(CbResult result)
{
    printf("got here at least %llx %d\n", result.ctx, result.result);
    return;
}

int main()
{
    IoCtx ctx;
    initIoCtx(&ctx);
    
    u32 sock = (u32) socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4269);
    addr.sin_addr.s_addr = 0;
    AOE(bind(sock, (struct sockaddr*)&addr, sizeof(addr)));
    AOE(listen(sock, 0));
    CbInfo info{};
    info.fn = acceptCb;
    info.ctx = 0xdeadbeef;
    queueIoOperation(&ctx, info, OpType::Accept, sock, 1, buf_t<u8>{});
    while(1) dequeueIoAndCb(&ctx);
    //io_uring_setup()
    return 0;
}
