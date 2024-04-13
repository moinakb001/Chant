#include <stdio.h>
#include <stdlib.h>
#include <liburing.h>
#include <unistd.h>
#include <sys/mman.h>
#include "types.hpp"

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

enum class DataType: u64 {
    Accept = 0,
    Read = 1,
    Write = 2,
};

struct UserData {
    u64 type: 4;
    u64 data: 60;
};



int main()
{
    io_uring ring;
    io_uring_params params{};
    s32 ret;
    params.flags = IORING_SETUP_COOP_TASKRUN;
    AOE(io_uring_queue_init_params(ringDepth, &ring, &params));
    auto bufs = io_uring_setup_buf_ring(&ring, numBufs, bufBgid, 0, (int*) &ret);
    AOE(ret);
    BufferGroup *pBufsData;
    posix_memalign((void **) &pBufsData, 4096, sizeof(BufferGroup));
    //BufferGroup *pBufsData = (BufferGroup *) mmap(NULL, sizeof(BufferGroup), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    for(u64 i = 0; i < numBufs; i++)
    {
        io_uring_buf_ring_add(bufs, &pBufsData->buffers[i], bufSize, i + 1, io_uring_buf_ring_mask(numBufs), i);
    }
    io_uring_buf_ring_advance(bufs, numBufs);
    
    auto sqe = io_uring_get_sqe(&ring);
    io_uring_cqe *pCqe;
   /*io_uring_sqe_set_data64(sqe, 0xdead);
    io_uring_prep_provide_buffers(sqe, &pBufsData->buffers[0], bufSize, numBufs, bufBgid, 1);
    io_uring_submit(&ring);
    
    AOE(io_uring_wait_cqe(&ring, &pCqe));
    printf("flags %x res %d 0x%llx\n", pCqe->flags, pCqe->res, io_uring_cqe_get_data64(pCqe));
    sqe = io_uring_get_sqe(&ring);*/
    io_uring_prep_read(sqe, /*fd*/0, 0, 1, -1);
    io_uring_sqe_set_data64(sqe, 0x1);
    sqe->buf_group = bufBgid;
	sqe->flags |= IOSQE_BUFFER_SELECT | IOSQE_ASYNC;
    io_uring_submit(&ring);
    while(1)
    {
        io_uring_cqe *pCqe;
        AOE(io_uring_wait_cqe(&ring, &pCqe));
        if ( pCqe->res < 0)
        {
            printf("ERROR: %d\n", -pCqe->res);
            return -1;
        }
        if(io_uring_cqe_get_data64(pCqe) == 0x2)
        {
            auto buf = io_uring_cqe_get_data64(pCqe) >> 2;
            io_uring_cqe_seen(&ring, pCqe);
            io_uring_buf_ring_add(bufs, &pBufsData->buffers[buf-1], bufSize, buf, io_uring_buf_ring_mask(numBufs), 0);
            io_uring_buf_ring_advance(bufs, 1);
            continue;
        }
        if ( pCqe->res == 0)
        {
            auto sqe = io_uring_get_sqe(&ring);
            io_uring_prep_close(sqe, 1);
            sqe->flags |= IOSQE_IO_DRAIN;
            io_uring_sqe_set_data64(sqe, 0x3);
            io_uring_cqe_seen(&ring, pCqe);
            AOE(io_uring_submit(&ring));
            AOE(io_uring_wait_cqe(&ring, &pCqe));
            while(io_uring_cqe_get_data64(pCqe) != 0x3)
            {
                io_uring_cqe_seen(&ring, pCqe);
            }
            return 0;
        }
        if(pCqe->flags & IORING_CQE_F_BUFFER)
        {
            auto bufid = pCqe->flags >> 16;
            auto wsqe = io_uring_get_sqe(&ring);
            io_uring_prep_write(wsqe, /*fd*/1, &pBufsData->buffers[bufid-1], pCqe->res, -1);
            wsqe->flags |= IOSQE_IO_LINK | IOSQE_CQE_SKIP_SUCCESS;
            wsqe = io_uring_get_sqe(&ring);
            io_uring_prep_write(wsqe, /*fd*/1, &pBufsData->buffers[bufid-1], pCqe->res, -1);
            io_uring_sqe_set_data64(wsqe, 0x2 | (bufid << 2));
        }
        if(!(pCqe->flags & IORING_CQE_F_MORE))
        {
            //printf("THERE\n");
            auto sqe = io_uring_get_sqe(&ring);
            io_uring_prep_read(sqe, /*fd*/0, 0, 1, -1);
            io_uring_sqe_set_data64(sqe, 0x1);
            sqe->buf_group = bufBgid;
            sqe->flags |= IOSQE_BUFFER_SELECT | IOSQE_ASYNC;
        }
        io_uring_cqe_seen(&ring, pCqe);
        AOE(io_uring_submit(&ring));

    }
    

    //io_uring_setup()
    return 0;
}
