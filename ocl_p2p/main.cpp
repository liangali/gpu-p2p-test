#include <CL/cl.h>
#include <iostream>
#include <vector>

#include "context.h"

char read_kernel_code[] = " \
kernel void read_from_remote(global int *src1, global int *src2) \
{ \
  const int id = get_global_id(0); \
  src1[id] = src2[id] * 3; \
} \
";

char write_kernel_code[] = " \
kernel void write_to_remote(global int *src1, global int *src2)  \
{ \
  const int id = get_global_id(0); \
  src2[id] = src1[id] * 5; \
} \
";

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
    int local_gpu = 0, remote_gpu = 0, data_count = 1024;

    oclContext ctx0, ctx1;

    ctx0.init(local_gpu);
    ctx1.init(remote_gpu);

    void* buf0 = ctx0.initUSM(data_count, 0);
    void* buf1 = ctx0.initUSM(data_count, 1);
    printf("buf0 = %p, buf1 = %p\n", buf0, buf1);

    std::vector<uint32_t> hostBuf0(data_count, 0);
    ctx0.readUSM(buf0, hostBuf0, data_count * sizeof(uint32_t));
    printBuf(hostBuf0, 16);

    std::vector<uint32_t> hostBuf1(data_count, 0);
    ctx0.readUSM(buf1, hostBuf1, data_count * sizeof(uint32_t));
    printBuf(hostBuf1, 16);

    ctx0.runKernel(read_kernel_code, "read_from_remote", buf0, buf1, data_count);
    ctx0.readUSM(buf0, hostBuf0, data_count * sizeof(uint32_t));
    printBuf(hostBuf0, 16);

    // ctx0.runKernel(write_kernel_code, "write_to_remote", buf0, buf1, data_count);
    // ctx1.readUSM(buf1, hostBuf1, data_count * sizeof(uint32_t));
    // printBuf(hostBuf1, 16);

    ctx0.freeUSM(buf0);
    ctx0.freeUSM(buf1);
    return 0;
}
