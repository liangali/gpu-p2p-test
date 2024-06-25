#include <CL/cl.h>
#include <iostream>
#include <vector>

#include "ocl_context.h"
#include "lz_context.h"

int main(int argc, char **argv)
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

    return 0;
}
