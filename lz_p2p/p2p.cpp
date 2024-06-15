#include <stdio.h>
#include <stdlib.h>

#include "context.h"

void printBuf(std::vector<uint32_t> &buf, int size)
{
    for (int i = 0; i < size; i++) {
        printf("%d, ", buf[i]);
        if (i % 16 == 0)
            printf("\n");
    }  
    printf("\n");
}

int main()
{
    lzContext ctx0, ctx1;
    ctx0.initZe(0);
    ctx1.initZe(1);

    queryP2P(ctx0.device(), ctx1.device());
    queryP2P(ctx1.device(), ctx0.device());

    void* buf0 = ctx0.initBuffer(1024);
    void* buf1 = ctx1.initBuffer(1024);
    printf("buf0 = %p, buf1 = %p\n", buf0, buf1);

    std::vector<uint32_t> hostBuf0(1024, 0);
    ctx0.copyBuffer(hostBuf0);
    printBuf(hostBuf0, 16);

    std::vector<uint32_t> hostBuf1(1024, 0);
    ctx1.copyBuffer(hostBuf1);
    printBuf(hostBuf0, 16);

    ctx0.runKernel("../add_kernel_dg2.spv", "vector_add", buf1);
    ctx0.copyBuffer(hostBuf0);
    printBuf(hostBuf0, 16);

    printf("done\n");
    return 0;
}