#include <CL/cl.h>
#include <iostream>
#include <vector>

#include "ocl_context.h"
#include "lz_context.h"

void simple_interop()
{
    size_t elemCount = 1024 * 1024;
    std::vector<uint32_t> initBuf(elemCount, 0);
    for (size_t i = 0; i < elemCount; i++)
        initBuf[i] = (i % 1024);

    // initialize opencl
    oclContext oclctx;
    oclctx.init(0);

    // initialize level-zero
    lzContext lzctx;
    lzctx.initZe(0);

    // create opencl buffer and derive dma-buf handle from it
    cl_mem clBuffer = oclctx.createBuffer(elemCount * sizeof(uint32_t), initBuf);
    oclctx.printBuffer(clBuffer);
    uint64_t handle = oclctx.deriveHandle(clBuffer);

    // create level-zero device memory from the handle
    void *lzptr = lzctx.createFromHandle(handle, elemCount * sizeof(uint32_t));
    lzctx.printBuffer(lzptr);

    oclctx.freeBuffer(clBuffer);
}

int main(int argc, char **argv)
{
    // simple_interop();

    size_t elemCount = 1024 * 1024;
    std::vector<uint32_t> initBuf(elemCount, 0);
    for (size_t i = 0; i < elemCount; i++)
        initBuf[i] = (i % 1024);

    // initialize two opencl contexts, oclctx0 on GPU0 and oclctx1 on GPU1
    oclContext oclctx0, oclctx1;
    oclctx0.init(0);
    oclctx1.init(1);

    // initialize two level-zero contexts, lzctx0 on GPU0 and lzctx1 on GPU1
    lzContext lzctx0, lzctx1;
    lzctx0.initZe(0);
    lzctx1.initZe(1);

    // create two opencl buffers on the device memory of GPU0 and GPU1 respectively
    cl_mem clbuf0 = oclctx0.createBuffer(elemCount * sizeof(uint32_t), initBuf);
    cl_mem clbuf1 = oclctx1.createBuffer(elemCount * sizeof(uint32_t), initBuf);
    oclctx0.printBuffer(clbuf0);
    oclctx1.printBuffer(clbuf1);

    // derive the dma-buf handles from opencl buffers
    uint64_t handle0 = oclctx0.deriveHandle(clbuf0);
    uint64_t handle1 = oclctx1.deriveHandle(clbuf1);

    // create two level-zero device memory on GPU0/GPU1 based on the dma-buf handles
    void *lzptr0 = lzctx0.createFromHandle(handle0, elemCount * sizeof(uint32_t));
    void *lzptr1 = lzctx1.createFromHandle(handle1, elemCount * sizeof(uint32_t));
    lzctx0.printBuffer(lzptr0);
    lzctx1.printBuffer(lzptr1);

    // run p2p data transfer kernel: GPU0 read data from GPU1
    lzctx0.runKernel("../../lz_p2p/test_kernel_dg2.spv", "local_read_from_remote", lzptr1, lzptr0, elemCount);

    // run p2p data transfer kernel: GPU0 write data to GPU1
    lzctx0.runKernel("../../lz_p2p/test_kernel_dg2.spv", "local_write_to_remote", lzptr1, lzptr0, elemCount);

    // print the content of the original opencl buffers, the data was changed after above level-zero kernel execution.
    oclctx0.printBuffer(clbuf0);
    oclctx1.printBuffer(clbuf1);

    oclctx0.freeBuffer(clbuf0);
    oclctx1.freeBuffer(clbuf1);

    return 0;
}
