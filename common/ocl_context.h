#pragma  once

#include "common.h"



class oclContext
{
private:
    cl_platform_id platform_ = nullptr;
    cl_device_id device_ = nullptr;
    cl_context context_ = nullptr;
    cl_command_queue queue_ = nullptr;

public:
    oclContext(/* args */);
    ~oclContext();

    cl_device_id device() {return device_; };
    cl_context context() {return context_; };
    cl_command_queue queue() {return queue_; };

    void init(int devIdx);
    void* initUSM(size_t elem_count, int offset);
    void readUSM(void* ptr, std::vector<uint32_t> &outBuf, size_t size);
    void freeUSM(void* ptr);
    void runKernel(char* programFile, char* kernelName, void *ptr0, void *ptr1, size_t elemCount);

    int createHandle(size_t size);
};