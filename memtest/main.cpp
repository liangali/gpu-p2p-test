#include <CL/cl.h>
#include <iostream>
#include <vector>

#include "ocl_context.h"

char test_kernel_code[] = " \
kernel void test_kernel(global int *buf0, global int *buf1) \
{ \
  const int id = get_global_id(0); \
  int offset = 1024*1024*1024; \
  buf1 += offset; \
  buf1[id] = buf0[id] * 2; \
} \
kernel void test_kernel2(global int *buf0, global int *buf1) \
{ \
  const int id = get_global_id(0); \
  int offset = 1024*1024*1024; \
  buf1 += offset; \
  buf0[id] = buf1[id] * 3; \
} \
";


int main(int argc, char** argv) 
{
    size_t elemCount = 1024*1024;
    std::vector<uint32_t> initBuf(elemCount, 0);
    for (size_t i = 0; i < elemCount; i++)
        initBuf[i] = (i % 1024);

    oclContext oclctx;
    oclctx.init(0);

    cl_mem buf0 = oclctx.createBuffer(elemCount * sizeof(uint32_t), initBuf);

    size_t sizeInBytes = 1.5 * 1024 * 1024 * 1024 * sizeof(uint32_t);
    cl_mem buf1 = oclctx.createBuffer(sizeInBytes);
    printf("buf size = %lld, buf handle = %p\n", sizeInBytes, buf1);

    oclctx.runKernel(test_kernel_code, "test_kernel", buf0, buf1, elemCount);
    oclctx.runKernel(test_kernel_code, "test_kernel2", buf0, buf1, elemCount);
    oclctx.printBuffer(buf0, 16, 0);

    oclctx.freeBuffer(buf0);
    oclctx.freeBuffer(buf1);

    return 0;
}
