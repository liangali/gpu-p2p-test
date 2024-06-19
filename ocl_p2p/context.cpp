
#include "context.h"

oclContext::oclContext(/* args */)
{
}

oclContext::~oclContext()
{
    printf("Enter %s\n", __FUNCTION__);

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
            printf("Platform has %d GPU devices, platform handle = %p\n", num_devices, platform_);

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

            char device_name[1024];
            err = clGetDeviceInfo(device_, CL_DEVICE_NAME, sizeof(device_name), device_name, nullptr);
            CHECK_OCL_ERROR_EXIT(err, "clGetDeviceInfo");

            printf("Created OpenCL GPU device for devIdx = %d on %s, device handle = %p\n", devIdx, device_name, device_);

            return;
        }
    }

    printf("ERROR: cannot find OpenCL GPU device!\n");
    exit(-1);
}
