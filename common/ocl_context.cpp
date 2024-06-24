
#include "ocl_context.h"

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

void oclContext::readUSM(void *ptr, std::vector<uint32_t> &outBuf, size_t size)
{
    cl_int err;
    err = clEnqueueMemcpyINTEL(queue_, true, (void*)outBuf.data(), ptr, size, 0, nullptr, nullptr);
    CHECK_OCL_ERROR_EXIT(err, "clEnqueueMemcpyINTEL failed");
    clFinish(queue_);
}

void oclContext::freeUSM(void *ptr)
{
    cl_int err;
    err = clMemBlockingFreeINTEL(context_, ptr);
    CHECK_OCL_ERROR(err, "clMemBlockingFreeINTEL");
}

void oclContext::runKernel(char *kernelCode, char *kernelName, void *ptr0, void *ptr1, size_t elemCount)
{
   cl_int err;

    cl_uint knlcount = 1;
    const char* knlstrList[] = { kernelCode };
    size_t knlsizeList[] = { strlen(kernelCode) };

    cl_program program = clCreateProgramWithSource(context_, knlcount, knlstrList, knlsizeList, &err);
    CHECK_OCL_ERROR_EXIT(err, "clCreateProgramWithSource failed");

    std::string buildopt = "-cl-std=CL2.0";
    err = clBuildProgram(program, 0, NULL, buildopt.c_str(), NULL, NULL);
    if (err < 0)
    {
        size_t logsize = 0;
        err = clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, 0, NULL, &logsize);
        CHECK_OCL_ERROR_EXIT(err, "clGetProgramBuildInfo failed");

        std::vector<char> logbuf(logsize+1, 0);
        err = clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, logsize+1, logbuf.data(), NULL);
        CHECK_OCL_ERROR_EXIT(err, "clGetProgramBuildInfo failed");
        printf("%s\n", logbuf.data());

        exit(1);
    }

    cl_kernel kernel = clCreateKernel(program, kernelName, &err);
    CHECK_OCL_ERROR_EXIT(err, "clCreateKernel failed");


    err = clSetKernelArgMemPointerINTEL(kernel, 0, ptr0);
    CHECK_OCL_ERROR_EXIT(err, "clSetKernelArg failed");

    err = clSetKernelArgMemPointerINTEL(kernel, 1, ptr1);
    CHECK_OCL_ERROR_EXIT(err, "clSetKernelArg failed");

    size_t global_size[] = { elemCount };
    err = clEnqueueNDRangeKernel(queue_, kernel, 1, nullptr, global_size, nullptr, 0, nullptr, nullptr);
    CHECK_OCL_ERROR_EXIT(err, "clEnqueueNDRangeKernel failed");
    clFinish(queue_);

    clReleaseKernel(kernel);
    clReleaseProgram(program);
}

int oclContext::createHandle(size_t size)
{
    cl_int err;
        
    cl_mem clbuf = clCreateBuffer(context_, CL_MEM_READ_WRITE, size, nullptr, &err);
    CHECK_OCL_ERROR_EXIT(err, "clCreateBuffer");

    uint64_t nativeHandle;
    err = clGetMemObjectInfo(clbuf, CL_MEM_ALLOCATION_HANDLE_INTEL, sizeof(nativeHandle), &nativeHandle, NULL);
    CHECK_OCL_ERROR(err, "clGetMemObjectInfo - CL_MEM_ALLOCATION_HANDLE_INTEL failed");
    printf("MemINFO: CL_MEM_ALLOCATION_HANDLE_INTEL = 0x%lx\n", nativeHandle);

    return nativeHandle;
}
