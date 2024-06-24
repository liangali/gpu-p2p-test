#include <CL/cl.h>
#include <iostream>
#include <vector>

#include "ocl_context.h"
#include "lz_context.h"

void printBuf(std::vector<uint32_t> &buf, int size)
{
    for (int i = 0; i < size; i++) {
        printf("%d, ", buf[i]);
        if (i && i % 16 == 0)
            printf("\n");
    }  
    printf("\n");
}


int main(int argc, char** argv) 
{
    size_t elemCount = 1024;

    oclContext oclctx;
    oclctx.init(0);

    cl_mem clBuffer = oclctx.initBuffer(elemCount, 0);
    std::vector<uint32_t> hostBuf0(elemCount, 0);
    oclctx.readBuffer(clBuffer, hostBuf0, elemCount * sizeof(uint32_t));
    printBuf(hostBuf0, 16);
    uint64_t handle = oclctx.clBufToHandle(clBuffer);
    
    lzContext lzctx;
    lzctx.initZe(0);

    void* lzptr = lzctx.creatBufferFromHandle(handle, elemCount * sizeof(uint32_t));
    std::vector<uint32_t> hostBuf1(elemCount, 0);
    lzctx.copyBuffer(hostBuf1, lzptr, elemCount);
    printBuf(hostBuf1, 16);

    oclctx.freeBuffer(clBuffer);

    return 0;
}
