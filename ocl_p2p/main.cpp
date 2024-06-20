#include <CL/cl.h>
#include <iostream>
#include <vector>

#include "context.h"

int main(int argc, char** argv) 
{
    int local_gpu = 0, remote_gpu = 1, data_count = 1024;

    oclContext ctx0, ctx1;

    ctx0.init(local_gpu);
    ctx1.init(remote_gpu);

    void* buf0 = ctx0.initUSM(data_count, 0);
    void* buf1 = ctx1.initUSM(data_count, 1);
    printf("buf0 = %p, buf1 = %p\n", buf0, buf1);

    ctx0.freeUSM(buf0);
    ctx1.freeUSM(buf1);
    return 0;
}
