#pragma once

#include "common.h"

#ifndef CL_MEM_DEVICE_ID_INTEL
#define CL_MEM_DEVICE_ID_INTEL 0x10011
#endif /* CL_MEM_DEVICE_ID_INTEL */

typedef CL_API_ENTRY cl_mem(CL_API_CALL *pfn_clCreateBufferWithPropertiesINTEL)(
    cl_context context,
    const cl_mem_properties_intel *properties,
    cl_mem_flags flags,
    size_t size,
    void *hostPtr,
    cl_int *errcodeRet);

class oclContext
{
private:
    cl_platform_id platform_ = nullptr;
    cl_device_id device_ = nullptr;
    std::vector<cl_device_id> devList_; // multi-device context
    cl_context context_ = nullptr;
    cl_command_queue queue_ = nullptr;
    cl_program program_ = nullptr;
    cl_kernel kernel_ = nullptr;
    pfn_clCreateBufferWithPropertiesINTEL clCreateBufferWithPropertiesINTEL_ = nullptr;

public:
    oclContext(/* args */);
    ~oclContext();

    cl_device_id device() { return device_; };
    cl_context context() { return context_; };
    cl_command_queue queue() { return queue_; };

    void init(int devIdx);
    void init(std::vector<int> device_list);
    void *initUSM(size_t elem_count, int offset);
    void readUSM(void *ptr, std::vector<uint32_t> &outBuf, size_t size);
    void freeUSM(void *ptr);
    void runKernel(char *programFile, char *kernelName, void *ptr0, void *ptr1, size_t elemCount, cl_event* se = nullptr, cl_event* we = nullptr, int sync = 2);
    void runKernel(char *programFile, char *kernelName, cl_mem buf0, cl_mem buf1, size_t elemCount, cl_event* se = nullptr, cl_event* we = nullptr, int sync = 2);

    cl_mem createBuffer(size_t size, const std::vector<uint32_t> &inbuf = std::vector<uint32_t>{});
    cl_mem createBuffer2(int devIdx, size_t size, const std::vector<uint32_t> &inbuf);
    uint64_t deriveHandle(cl_mem clbuf);
    cl_mem createFromHandle(uint64_t handle, size_t size);
    void readBuffer(cl_mem clbuf, std::vector<uint32_t> &outBuf, size_t size, size_t offset);
    void freeBuffer(cl_mem clbuf);
    void printBuffer(cl_mem clbuf, size_t count = 16, size_t offset = 0);
    size_t validateBuffer(cl_mem clbuf, size_t count, int factor);
};