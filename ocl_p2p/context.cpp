
#include "context.h"

oclContext::oclContext(/* args */)
{
}

oclContext::~oclContext()
{
    printf("Enter %s\n", __FUNCTION__);

    clReleaseCommandQueue(queue_);
    clReleaseContext(context_);
}

void oclContext::init(int devIdx)
{
    cl_int err;
    cl_uint num_platforms = 0;
    err = clGetPlatformIDs(0, nullptr, &num_platforms);
    CHECK_OCL_ERROR_EXIT(err, "clGetPlatformIDs");
    
    std::vector<cl_platform_id> platforms(num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
    CHECK_OCL_ERROR_EXIT(err, "clGetPlatformIDs");

    for (const auto &platform : platforms) {
        cl_uint num_devices = 0;
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);

        if (num_devices > 0) {
            platform_ = platform;
            printf("Platform %p has %d GPU devices\n", platform_, num_devices);

            std::vector<cl_device_id> devices(num_devices);
            err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices.data(), nullptr);
            CHECK_OCL_ERROR_EXIT(err, "clGetDeviceIDs");

            if (devIdx >= (num_devices)) {
                printf("ERROR: don't have OpenCL GPU device for devIdx = %d!\n", devIdx);
                exit(-1);
            }

            device_ = devices[devIdx];

            context_ = clCreateContext(NULL, 1, &device_, NULL, NULL, &err);
            CHECK_OCL_ERROR_EXIT(err, "clCreateContext");

            queue_ = clCreateCommandQueue(context_, device_, 0, &err);
            CHECK_OCL_ERROR_EXIT(err, "clCreateCommandQueue");

            char device_name[1024];
            err = clGetDeviceInfo(device_, CL_DEVICE_NAME, sizeof(device_name), device_name, nullptr);
            CHECK_OCL_ERROR_EXIT(err, "clGetDeviceInfo");

            printf("Created device for devIdx = %d on %s, device = %p, contex = %p, queue = %p\n", devIdx, device_name, device_, context_, queue_);

            return;
        }
    }

    printf("ERROR: cannot find OpenCL GPU device!\n");
    exit(-1);
}

void* oclContext::initUSM(size_t elem_count, int offset)
{
    cl_int err;
    void* ptr = nullptr;

    std::vector<uint32_t> hostBuf(elem_count, 0);
    for (size_t i = 0; i < elem_count; i++)
        hostBuf[i] = offset + (i%1024);

    size_t size = elem_count * sizeof(uint32_t);
    cl_uint alignment = 16;
    ptr = clDeviceMemAllocINTEL(context_, device_, nullptr, size, alignment, &err);
    CHECK_OCL_ERROR_EXIT(err, "clDeviceMemAllocINTEL failed")

    err = clEnqueueMemcpyINTEL(queue_, true, ptr, (void*)hostBuf.data(), size, 0, nullptr, nullptr);
    CHECK_OCL_ERROR_EXIT(err, "clEnqueueMemcpyINTEL failed");

    clFinish(queue_);

    return ptr;
}

void oclContext::freeUSM(void *ptr)
{
    cl_int err;
    err = clMemBlockingFreeINTEL(context_, ptr);
    CHECK_OCL_ERROR(err, "clMemBlockingFreeINTEL");
}
