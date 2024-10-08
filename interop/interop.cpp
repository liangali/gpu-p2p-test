
#include <CL/cl.h>
#include <iostream>
#include <vector>

#include "ocl_context.h"
#include "lz_context.h"

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
  src2[id] = src1[id] * 2; \
} \
";

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

int ocl_l0_interop_p2p()
{
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
    lzctx0.runKernel("../../lz_p2p/test_kernel_dg2.spv", "read_from_remote", lzptr1, lzptr0, elemCount);

    // run p2p data transfer kernel: GPU0 write data to GPU1
    lzctx0.runKernel("../../lz_p2p/test_kernel_dg2.spv", "write_to_remote", lzptr1, lzptr0, elemCount);

    // print the content of the original opencl buffers, the data was changed after above level-zero kernel execution.
    oclctx0.printBuffer(clbuf0);
    oclctx1.printBuffer(clbuf1);

    oclctx0.freeBuffer(clbuf0);
    oclctx1.freeBuffer(clbuf1);

    return 0;
}

int ocl_p2p_sync_host_and_event(int argc, char **argv)
{
    int sync = (argc == 2) ? atoi(argv[1]) : 1; // sync =1, add clFlush after enqueue by default

    size_t elemCount = 16 * 1024 * 1024;
    std::vector<uint32_t> initBuf(elemCount, 0);
    for (size_t i = 0; i < elemCount; i++)
        initBuf[i] = (i % 1024);

    // initialize two opencl contexts, oclctx0 on GPU0 and oclctx1 on GPU1
    oclContext oclctx0, oclctx1;
    oclctx0.init(0);
    oclctx1.init(1);

    // create clbuf0 on GPU0 and clbuf1 on GPU1
    cl_mem clbuf0 = oclctx0.createBuffer(elemCount * sizeof(uint32_t), initBuf);
    cl_mem clbuf1 = oclctx1.createBuffer(elemCount * sizeof(uint32_t), initBuf);
    cl_mem clbuf3 = oclctx1.createBuffer(elemCount * sizeof(uint32_t), initBuf);
    oclctx0.printBuffer(clbuf0);
    oclctx1.printBuffer(clbuf1);

    // derive the handle from clbuf1
    uint64_t handle1 = oclctx1.deriveHandle(clbuf1);

    // create clbuf2 from handle1
    cl_mem clbuf2 = oclctx0.createFromHandle(handle1, elemCount * sizeof(uint32_t));

    cl_event event;
    
    // use oclctx0 to launch a kernel on GPU0 to write data to remote buffer clbuf2 on GPU1
    oclctx0.runKernel(write_kernel_code, "write_to_remote", clbuf0, clbuf2, elemCount, &event, nullptr, sync); // clbuf0 * 2 --> clbuf2 (clbuf1)
    // oclctx0.validateBuffer(clbuf2, elemCount, 2);
    // oclctx0.printBuffer(clbuf2);
    
    // use oclctx1 to read the content of the original clbuf1
    // oclctx1.printBuffer(clbuf1);

    // use oclctx0 to launch a kernel on GPU0 to read data from remote buffer clbuf2 on GPU1
    oclctx1.runKernel(read_kernel_code, "read_from_remote", clbuf3, clbuf1, elemCount, nullptr, &event, sync); // clbuf1 * 3 --> clbuf3
    oclctx1.validateBuffer(clbuf3, elemCount, 6);
    oclctx1.printBuffer(clbuf3);

    oclctx0.freeBuffer(clbuf0);
    oclctx1.freeBuffer(clbuf1);

    return 0;
}

int ocl_p2p_sync_multi_devie_ctx_event(int argc, char **argv)
{
    int sync = (argc == 2) ? atoi(argv[1]) : 1; // sync =1, add clFlush after enqueue by default

    size_t elemCount = 16 * 1024 * 1024;
    std::vector<uint32_t> initBuf(elemCount, 0);
    for (size_t i = 0; i < elemCount; i++)
        initBuf[i] = (i % 1024);

    // initialize two opencl contexts
    oclContext oclctx0, oclctx1;
    oclctx0.init({0, 1}); // ctx0 is created on {GPU0, GPU1}, GPU0 is the preferred device for memory placement
    oclctx1.init({1, 0}); // ctx1 is created on {GPU1, GPU0}, GPU1 is the preferred device for memory placement

    cl_mem clbuf0 = oclctx0.createBuffer2(0, elemCount * sizeof(uint32_t), initBuf); // buf0 is allocated on GPU0's local memory
    cl_mem clbuf1 = oclctx0.createBuffer2(0, elemCount * sizeof(uint32_t), initBuf); // buf1 is allocated on GPU0's local memory
    cl_mem clbuf2 = oclctx1.createBuffer2(1, elemCount * sizeof(uint32_t), initBuf); // buf2 is allocated on GPU1's local memory
    cl_mem clbuf3 = oclctx1.createBuffer2(1, elemCount * sizeof(uint32_t), initBuf); // buf3 is allocated on GPU1's local memory
    oclctx0.printBuffer(clbuf0);
    oclctx1.printBuffer(clbuf2);

    // derive the handle from clbuf1
    uint64_t handle2 = oclctx1.deriveHandle(clbuf2);

    // create clbuf2 from handle2
    cl_mem clbuf2_shared = oclctx0.createFromHandle(handle2, elemCount * sizeof(uint32_t));

    cl_event event;
    
    // use oclctx0 to launch a kernel on GPU0 to write data to remote buffer clbuf2 on GPU1
    oclctx0.runKernel(write_kernel_code, "write_to_remote", clbuf0, clbuf2_shared, elemCount, &event, nullptr, sync); // clbuf0 * 2 --> clbuf2

    // oclctx0.runKernel(write_kernel_code, "write_to_remote", clbuf0, clbuf1, elemCount, &event, nullptr, sync); // clbuf0 * 2 --> clbuf1

    // use oclctx0 to launch a kernel on GPU0 to read data from remote buffer clbuf2 on GPU1
    oclctx1.runKernel(read_kernel_code, "read_from_remote", clbuf3, clbuf2, elemCount, nullptr, &event, sync); // clbuf2 * 3 --> clbuf3
    oclctx1.validateBuffer(clbuf3, elemCount, 6);
    oclctx1.printBuffer(clbuf3);

    oclctx0.freeBuffer(clbuf0);
    oclctx0.freeBuffer(clbuf1);
    oclctx1.freeBuffer(clbuf2);
    oclctx1.freeBuffer(clbuf3);

    return 0;
}

int main(int argc, char **argv)
{
    //simple_interop();

    //ocl_l0_interop_p2p();

    //ocl_p2p_sync_host_and_event(argc, argv);

    ocl_p2p_sync_multi_devie_ctx_event(argc, argv);

    return 0;
}