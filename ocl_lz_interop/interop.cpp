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
    oclContext oclctx;
    oclctx.init(0);

    lzContext lzctx;
    lzctx.initZe(0);

    return 0;
}
