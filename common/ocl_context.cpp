
#include "ocl_context.h"

oclContext::oclContext(/* args */)
{
}

oclContext::~oclContext()
{
    printf("Enter %s\n", __FUNCTION__);

    clReleaseKernel(kernel_);
    clReleaseProgram(program_);
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

    for (const auto &platform : platforms)
    {
        cl_uint num_devices = 0;
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);

        if (num_devices > 0)
        {
            platform_ = platform;
            printf("Platform %p has %d GPU devices\n", platform_, num_devices);

            std::vector<cl_device_id> devices(num_devices);
            err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices.data(), nullptr);
            CHECK_OCL_ERROR_EXIT(err, "clGetDeviceIDs");

            if (devIdx >= (num_devices))
            {
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

void oclContext::init(std::vector<int> device_list)
{
    if (device_list.size() < 2) {
        printf("ERROR: device_list must include 2 or more device index\n");
        exit(-1);
    }

    cl_int err;
    cl_uint num_platforms = 0;
    err = clGetPlatformIDs(0, nullptr, &num_platforms);
    CHECK_OCL_ERROR_EXIT(err, "clGetPlatformIDs");

    std::vector<cl_platform_id> platforms(num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
    CHECK_OCL_ERROR_EXIT(err, "clGetPlatformIDs");

    for (const auto &platform : platforms)
    {
        cl_uint num_devices = 0;
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);

        if (num_devices > 0)
        {
            platform_ = platform;
            printf("Platform %p has %d GPU devices\n", platform_, num_devices);

            if (!clCreateBufferWithPropertiesINTEL_) {
                clCreateBufferWithPropertiesINTEL_ = (pfn_clCreateBufferWithPropertiesINTEL)clGetExtensionFunctionAddressForPlatform(platform_, "clCreateBufferWithPropertiesINTEL");
                if (!clCreateBufferWithPropertiesINTEL_) {
                    printf("ERROR: cannot query interface clCreateBufferWithPropertiesINTEL\n");
                    exit(-1);
                }
            }

            std::vector<cl_device_id> devices(num_devices);
            err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices.data(), nullptr);
            CHECK_OCL_ERROR_EXIT(err, "clGetDeviceIDs");

            printf("all gpu devices: \n");
            for (size_t i = 0; i < num_devices; i++) {
                printf("gpu device index %d, cl_device_id = %p\n", i, devices[i]);
            }
            
            if (num_devices < device_list.size()) {
                printf("ERROR: available GPU device number (%d) is less than required device number (%s)\n", num_devices, device_list.size());
                exit(-1);
            }

            for (size_t i = 0; i < device_list.size(); i++) {
                assert(device_list[i] < num_devices);
                devList_.push_back(devices[device_list[i]]);
            }

            device_ = devices[device_list[0]];

            context_ = clCreateContext(NULL,  devList_.size(), devList_.data(), NULL, NULL, &err);
            CHECK_OCL_ERROR_EXIT(err, "clCreateContext");

            queue_ = clCreateCommandQueue(context_, device_, 0, &err);
            CHECK_OCL_ERROR_EXIT(err, "clCreateCommandQueue");

            char device_name[1024];
            err = clGetDeviceInfo(device_, CL_DEVICE_NAME, sizeof(device_name), device_name, nullptr);
            CHECK_OCL_ERROR_EXIT(err, "clGetDeviceInfo");
            printf("Created device for devIdx = %d on %s, device = %p, contex = %p, queue = %p\n", device_list[0], device_name, device_, context_, queue_);

            return;
        }
    }

    printf("ERROR: cannot find OpenCL GPU device!\n");
    exit(-1);
}

void *oclContext::initUSM(size_t elem_count, int offset)
{
    cl_int err;
    void *ptr = nullptr;

    std::vector<uint32_t> hostBuf(elem_count, 0);
    for (size_t i = 0; i < elem_count; i++)
        hostBuf[i] = offset + (i % 1024);

    size_t size = elem_count * sizeof(uint32_t);
    cl_uint alignment = 16;
    ptr = clDeviceMemAllocINTEL(context_, device_, nullptr, size, alignment, &err);
    CHECK_OCL_ERROR_EXIT(err, "clDeviceMemAllocINTEL failed")

    err = clEnqueueMemcpyINTEL(queue_, true, ptr, (void *)hostBuf.data(), size, 0, nullptr, nullptr);
    CHECK_OCL_ERROR_EXIT(err, "clEnqueueMemcpyINTEL failed");

    clFinish(queue_);

    return ptr;
}

void oclContext::readUSM(void *ptr, std::vector<uint32_t> &outBuf, size_t size)
{
    cl_int err;
    err = clEnqueueMemcpyINTEL(queue_, true, (void *)outBuf.data(), ptr, size, 0, nullptr, nullptr);
    CHECK_OCL_ERROR_EXIT(err, "clEnqueueMemcpyINTEL failed");
    clFinish(queue_);
}

void oclContext::freeUSM(void *ptr)
{
    cl_int err;
    err = clMemBlockingFreeINTEL(context_, ptr);
    CHECK_OCL_ERROR(err, "clMemBlockingFreeINTEL");
}

void oclContext::runKernel(char *kernelCode, char *kernelName, void *ptr0, void *ptr1, size_t elemCount, cl_event* se, cl_event* we, int sync)
{
    cl_int err;

    cl_uint knlcount = 1;
    const char *knlstrList[] = {kernelCode};
    size_t knlsizeList[] = {strlen(kernelCode)};

    program_ = clCreateProgramWithSource(context_, knlcount, knlstrList, knlsizeList, &err);
    CHECK_OCL_ERROR_EXIT(err, "clCreateProgramWithSource failed");

    std::string buildopt = "-cl-std=CL2.0";
    err = clBuildProgram(program_, 0, NULL, buildopt.c_str(), NULL, NULL);
    if (err < 0)
    {
        size_t logsize = 0;
        err = clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG, 0, NULL, &logsize);
        CHECK_OCL_ERROR_EXIT(err, "clGetProgramBuildInfo failed");

        std::vector<char> logbuf(logsize + 1, 0);
        err = clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG, logsize + 1, logbuf.data(), NULL);
        CHECK_OCL_ERROR_EXIT(err, "clGetProgramBuildInfo failed");
        printf("%s\n", logbuf.data());

        exit(1);
    }

    kernel_ = clCreateKernel(program_, kernelName, &err);
    CHECK_OCL_ERROR_EXIT(err, "clCreateKernel failed");

    err = clSetKernelArgMemPointerINTEL(kernel_, 0, ptr0);
    CHECK_OCL_ERROR_EXIT(err, "clSetKernelArg failed");

    err = clSetKernelArgMemPointerINTEL(kernel_, 1, ptr1);
    CHECK_OCL_ERROR_EXIT(err, "clSetKernelArg failed");

    size_t global_size[] = {elemCount};
    err = clEnqueueNDRangeKernel(queue_, kernel_, 1, nullptr, global_size, nullptr, (we == nullptr) ? 0 : 1, we, se);
    CHECK_OCL_ERROR_EXIT(err, "clEnqueueNDRangeKernel failed");

    if (sync == 1)
        clFlush(queue_);
    else if (sync == 2)
        clFinish(queue_);
}

void oclContext::runKernel(char *kernelCode, char *kernelName, cl_mem buf0, cl_mem buf1, size_t elemCount, cl_event* se, cl_event* we, int sync)
{
    cl_int err;

    cl_uint knlcount = 1;
    const char *knlstrList[] = {kernelCode};
    size_t knlsizeList[] = {strlen(kernelCode)};

    program_ = clCreateProgramWithSource(context_, knlcount, knlstrList, knlsizeList, &err);
    CHECK_OCL_ERROR_EXIT(err, "clCreateProgramWithSource failed");

    std::string buildopt = "-cl-std=CL2.0 -cl-intel-greater-than-4GB-buffer-required";
    err = clBuildProgram(program_, 0, NULL, buildopt.c_str(), NULL, NULL);
    if (err < 0)
    {
        size_t logsize = 0;
        err = clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG, 0, NULL, &logsize);
        CHECK_OCL_ERROR_EXIT(err, "clGetProgramBuildInfo failed");

        std::vector<char> logbuf(logsize + 1, 0);
        err = clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG, logsize + 1, logbuf.data(), NULL);
        CHECK_OCL_ERROR_EXIT(err, "clGetProgramBuildInfo failed");
        printf("%s\n", logbuf.data());

        exit(1);
    }

    kernel_ = clCreateKernel(program_, kernelName, &err);
    CHECK_OCL_ERROR_EXIT(err, "clCreateKernel failed");

    err = clSetKernelArg(kernel_, 0, sizeof(cl_mem), &buf0);
    CHECK_OCL_ERROR_EXIT(err, "clSetKernelArg failed");

    err = clSetKernelArg(kernel_, 1, sizeof(cl_mem), &buf1);
    CHECK_OCL_ERROR_EXIT(err, "clSetKernelArg failed");

    size_t global_size[] = {elemCount};
    err = clEnqueueNDRangeKernel(queue_, kernel_, 1, nullptr, global_size, nullptr, (we == nullptr) ? 0 : 1, we, se);
    CHECK_OCL_ERROR_EXIT(err, "clEnqueueNDRangeKernel failed");

    if (sync == 1)
        clFlush(queue_);
    else if (sync == 2)
        clFinish(queue_);
}

cl_mem oclContext::createBuffer(size_t size, const std::vector<uint32_t> &inbuf)
{
    cl_int err;

    cl_mem clbuf = clCreateBuffer(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOW_UNRESTRICTED_SIZE_INTEL, size, nullptr, &err);
    CHECK_OCL_ERROR_EXIT(err, "clCreateBuffer");

    if (!inbuf.empty())
    {
        err = clEnqueueWriteBuffer(queue_, clbuf, CL_TRUE, 0, size, inbuf.data(), 0, NULL, NULL);
        CHECK_OCL_ERROR_EXIT(err, "clEnqueueWriteBuffer failed");

        clFinish(queue_);
    }

    return clbuf;
}

cl_mem oclContext::createBuffer2(int devIdx, size_t size, const std::vector<uint32_t> &inbuf)
{
    cl_int err;

    const cl_mem_properties_intel memProperties[] = {
        CL_MEM_FLAGS,
        CL_MEM_READ_WRITE | CL_MEM_ALLOW_UNRESTRICTED_SIZE_INTEL,
        CL_MEM_DEVICE_ID_INTEL,
        (cl_mem_properties_intel)devList_[devIdx],
        0,
    };
    cl_mem clbuf = clCreateBufferWithPropertiesINTEL_(context_, memProperties, 0, size, nullptr, &err);
    CHECK_OCL_ERROR_EXIT(err, "clCreateBufferWithPropertiesINTEL");

    if (!inbuf.empty())
    {
        err = clEnqueueWriteBuffer(queue_, clbuf, CL_TRUE, 0, size, inbuf.data(), 0, NULL, NULL);
        CHECK_OCL_ERROR_EXIT(err, "clEnqueueWriteBuffer failed");

        clFinish(queue_);
    }

    return clbuf;
}

uint64_t oclContext::deriveHandle(cl_mem clbuf)
{
    cl_int err;
    uint64_t nativeHandle;
    err = clGetMemObjectInfo(clbuf, CL_MEM_ALLOCATION_HANDLE_INTEL, sizeof(nativeHandle), &nativeHandle, NULL);
    CHECK_OCL_ERROR(err, "clGetMemObjectInfo - CL_MEM_ALLOCATION_HANDLE_INTEL failed");

    return nativeHandle;
}

cl_mem oclContext::createFromHandle(uint64_t handle, size_t size)
{
    cl_int err;

	// Create extMemBuffer of type cl_mem from fd. 
	cl_mem_properties extMemProperties[] = {
        (cl_mem_properties)CL_EXTERNAL_MEMORY_HANDLE_DMA_BUF_KHR, 
		(cl_mem_properties)handle,
		0 
    }; 

    cl_mem extMemBuffer = clCreateBufferWithProperties(context_, extMemProperties, 0, size, NULL, &err);
    CHECK_OCL_ERROR(err, "clCreateBufferWithProperties failed");

    return extMemBuffer;
}

void oclContext::readBuffer(cl_mem clbuf, std::vector<uint32_t> &outBuf, size_t size, size_t offset)
{
    cl_int err;
    err = clEnqueueReadBuffer(queue_, clbuf, CL_TRUE, offset, size, outBuf.data(), 0, NULL, NULL);
    CHECK_OCL_ERROR_EXIT(err, "clEnqueueReadBuffer failed");
    clFinish(queue_);
}

void oclContext::freeBuffer(cl_mem clbuf)
{
    clReleaseMemObject(clbuf);
}

void oclContext::printBuffer(cl_mem clbuf, size_t count, size_t offset)
{
    std::vector<uint32_t> outBuf(count, 0);
    readBuffer(clbuf, outBuf, count*sizeof(uint32_t), offset);

    printf("The first %d elements in cl_mem = %p are: \n", count, clbuf);
    for (int i = 0; i < count; i++) {
        printf("%d, ", outBuf[i]);
        if (i && i % 16 == 0)
            printf("\n");
    }
    printf("\n");
}

size_t oclContext::validateBuffer(cl_mem clbuf, size_t count, int factor)
{
    size_t mismatchCount = 0;
    std::vector<uint32_t> outBuf(count, 0);
    readBuffer(clbuf, outBuf, count*sizeof(uint32_t), 0);

    for (int i = 0; i < count; i++) {
        if (outBuf[i] != (i%1024)*factor)
            mismatchCount++;
    }

    if (mismatchCount)
        printf("!!!! Validation failed !!!! count = %d, factor = %d, mismatch percentage = %f \n", count, factor, (float)mismatchCount/float(count));
    else
        printf("#### Validation passed #### count = %d, factor = %d\n", count, factor);
    
    return mismatchCount;
}
