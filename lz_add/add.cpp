#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <new>
#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <fstream>
#include <memory>
#include <iomanip>

#include "ze_api.h"

const char *kernel_spv_file_dg2 = "../add_kernel_dg2.spv";
const char *kernel_func_name = "vector_add";

ze_result_t result;
const ze_device_type_t type = ZE_DEVICE_TYPE_GPU;
ze_driver_handle_t pDriver = nullptr;
ze_device_handle_t pDevice = nullptr;
ze_context_handle_t context;
ze_command_list_handle_t command_list = nullptr;
ze_command_queue_handle_t command_queue = nullptr;
int dma_buf_fd = 0;

#define CHECK_ZE_STATUS(err, msg)                                                                                  \
    if (err < 0)                                                                                                   \
    {                                                                                                              \
        printf("ERROR: %s failed with err = 0x%08x, in function %s, line %d\n", msg, err, __FUNCTION__, __LINE__); \
        exit(0);                                                                                                   \
    }                                                                                                              \
    else                                                                                                           \
    {                                                                                                              \
        printf("INFO[ZE]: %s succeed\n", msg);                                                                     \
    }

inline ze_device_handle_t findDevice(
    ze_driver_handle_t pDriver,
    ze_device_type_t type)
{
    // get all devices
    uint32_t deviceCount = 0;
    zeDeviceGet(pDriver, &deviceCount, nullptr);

    std::vector<ze_device_handle_t> devices(deviceCount);
    zeDeviceGet(pDriver, &deviceCount, devices.data());

    ze_device_handle_t found = nullptr;

    // for each device, find the first one matching the type
    for (uint32_t device = 0; device < deviceCount; ++device)
    {
        auto phDevice = devices[device];

        ze_device_properties_t device_properties = {};
        device_properties.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
        zeDeviceGetProperties(phDevice, &device_properties);

        if (type == device_properties.type)
        {
            found = phDevice;

            ze_driver_properties_t driver_properties = {};
            driver_properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
            zeDriverGetProperties(pDriver, &driver_properties);

            std::cout << "Found " << std::to_string(type) << " device..." << "\n";
            std::cout << "Driver version: " << driver_properties.driverVersion << "\n";

            ze_api_version_t version = {};
            zeDriverGetApiVersion(pDriver, &version);
            // std::cout << "API version: " << to_string(version) << "\n";

            // std::cout << to_string(device_properties) << "\n";

            ze_device_compute_properties_t compute_properties = {};
            compute_properties.stype = ZE_STRUCTURE_TYPE_DEVICE_COMPUTE_PROPERTIES;
            zeDeviceGetComputeProperties(phDevice, &compute_properties);
            // std::cout << to_string(compute_properties) << "\n";

            uint32_t memoryCount = 0;
            zeDeviceGetMemoryProperties(phDevice, &memoryCount, nullptr);
            auto pMemoryProperties = new ze_device_memory_properties_t[memoryCount];
            for (uint32_t mem = 0; mem < memoryCount; ++mem)
            {
                pMemoryProperties[mem].stype = ZE_STRUCTURE_TYPE_DEVICE_MEMORY_PROPERTIES;
                pMemoryProperties[mem].pNext = nullptr;
            }
            zeDeviceGetMemoryProperties(phDevice, &memoryCount, pMemoryProperties);
            for (uint32_t mem = 0; mem < memoryCount; ++mem)
            {
                // std::cout << to_string( pMemoryProperties[ mem ] ) << "\n";
            }
            delete[] pMemoryProperties;

            ze_device_memory_access_properties_t memory_access_properties = {};
            memory_access_properties.stype = ZE_STRUCTURE_TYPE_DEVICE_MEMORY_ACCESS_PROPERTIES;
            zeDeviceGetMemoryAccessProperties(phDevice, &memory_access_properties);
            // std::cout << to_string( memory_access_properties ) << "\n";

            uint32_t cacheCount = 0;
            zeDeviceGetCacheProperties(phDevice, &cacheCount, nullptr);
            auto pCacheProperties = new ze_device_cache_properties_t[cacheCount];
            for (uint32_t cache = 0; cache < cacheCount; ++cache)
            {
                pCacheProperties[cache].stype = ZE_STRUCTURE_TYPE_DEVICE_CACHE_PROPERTIES;
                pCacheProperties[cache].pNext = nullptr;
            }
            zeDeviceGetCacheProperties(phDevice, &cacheCount, pCacheProperties);
            for (uint32_t cache = 0; cache < cacheCount; ++cache)
            {
                // std::cout << to_string( pCacheProperties[ cache ] ) << "\n";
            }
            delete[] pCacheProperties;

            ze_device_image_properties_t image_properties = {};
            image_properties.stype = ZE_STRUCTURE_TYPE_DEVICE_IMAGE_PROPERTIES;
            zeDeviceGetImageProperties(phDevice, &image_properties);
            // std::cout << to_string( image_properties ) << "\n";

            break;
        }
    }

    return found;
}

int initZe()
{
    size_t size = 0;
    size_t alignment = 0;
    result = zeInit(0);
    CHECK_ZE_STATUS(result, "zeInit");

    uint32_t driverCount = 0;
    result = zeDriverGet(&driverCount, nullptr);
    CHECK_ZE_STATUS(result, "zeDriverGet");
    printf("INFO: driver count = %d\n", driverCount);

    std::vector<ze_driver_handle_t> drivers(driverCount);
    result = zeDriverGet(&driverCount, drivers.data());
    CHECK_ZE_STATUS(result, "zeDriverGet");

    for (uint32_t driver = 0; driver < driverCount; ++driver)
    {
        pDriver = drivers[driver];
        pDevice = findDevice(pDriver, type);
        if (pDevice)
        {
            printf("INFO: find device handle = 0x%08llx\n", (uint64_t)pDevice);
            break;
        }
    }

    if (!pDevice)
    {
        printf("ERROR: cannot find a proper device\n");
        return -1;
    }

    // Create the context
    ze_context_desc_t context_desc = {};
    context_desc.stype = ZE_STRUCTURE_TYPE_CONTEXT_DESC;
    result = zeContextCreate(pDriver, &context_desc, &context);
    CHECK_ZE_STATUS(result, "zeContextCreate");

    // Create command list
    ze_command_list_desc_t descriptor_cmdlist = {};
    descriptor_cmdlist.stype = ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC;
    descriptor_cmdlist.pNext = nullptr;
    descriptor_cmdlist.flags = 0;
    descriptor_cmdlist.commandQueueGroupOrdinal = 0;
    result = zeCommandListCreate(context, pDevice, &descriptor_cmdlist, &command_list);
    CHECK_ZE_STATUS(result, "zeCommandListCreate");

    // Create command queue
    ze_command_queue_desc_t descriptor_cmdqueue = {};
    descriptor_cmdqueue.stype = ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC;
    descriptor_cmdqueue.pNext = nullptr;
    descriptor_cmdqueue.flags = 0;
    descriptor_cmdqueue.mode = ZE_COMMAND_QUEUE_MODE_DEFAULT;
    descriptor_cmdqueue.priority = ZE_COMMAND_QUEUE_PRIORITY_NORMAL;
    descriptor_cmdqueue.ordinal = 0;
    descriptor_cmdqueue.index = 0;
    ze_device_properties_t properties = {};
    result = zeDeviceGetProperties(pDevice, &properties);
    CHECK_ZE_STATUS(result, "zeDeviceGetProperties");
    result = zeCommandQueueCreate(context, pDevice, &descriptor_cmdqueue, &command_queue);
    CHECK_ZE_STATUS(result, "zeCommandQueueCreate");

    return 0;
}

int readKernel(std::vector<char> &binary)
{
    FILE *fp = nullptr;
    size_t nsize = 0;
    fp = fopen(kernel_spv_file_dg2, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        nsize = (size_t)ftell(fp);
        fseek(fp, 0, SEEK_SET);

        binary.resize(nsize + 1);
        memset(binary.data(), 0, binary.size());
        fread(binary.data(), sizeof(unsigned char), nsize, fp);

        fclose(fp);
        return 0;
    }

    printf("ERROR: cannot open kernel spv file %\n", kernel_spv_file_dg2);
    return -1;
}

int testCopyMem()
{
    const size_t buf_size = 1 * 1024 * 1024;
    std::vector<uint8_t> host_src(buf_size, 0);
    std::vector<uint8_t> host_dst(buf_size, 0);
    for (size_t i = 0; i < buf_size; i++)
    {
        host_src[i] = i % 256;
    }

    void *tmp_mem = nullptr;
    ze_device_mem_alloc_desc_t tmp_desc = {
        ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC,
        nullptr,
        0, // flags
        0  // ordinal
    };
    result = zeMemAllocDevice(context, &tmp_desc, buf_size, 1, pDevice, &tmp_mem);
    CHECK_ZE_STATUS(result, "zeMemAllocDevice");

    // host_src --> tmp_mem
    result = zeCommandListAppendMemoryCopy(command_list, tmp_mem, host_src.data(), buf_size, nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendMemoryCopy");

    result = zeCommandListAppendBarrier(command_list, nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendBarrier");

    // tmp_mem --> host_dst
    result = zeCommandListAppendMemoryCopy(command_list, host_dst.data(), tmp_mem, buf_size, nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendMemoryCopy");

    result = zeCommandListAppendBarrier(command_list, nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendBarrier");

    result = zeCommandListClose(command_list);
    CHECK_ZE_STATUS(result, "zeCommandListClose");

    result = zeCommandQueueExecuteCommandLists(command_queue, 1, &command_list, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandQueueExecuteCommandLists");

    result = zeCommandQueueSynchronize(command_queue, UINT64_MAX);
    CHECK_ZE_STATUS(result, "zeCommandQueueSynchronize");

    bool copy_passed = true;
    for (size_t i = 0; i < buf_size; i++)
    {
        if (host_dst[i] != (i % 256))
        {
            copy_passed = false;
            break;
        }
    }

    if (copy_passed)
        printf("INFO: copy test passed. size = %d \n", buf_size);
    else
        printf("INFO: copy test failed!!! size = %d \n", buf_size);

    return 0;
}

void* allocDeviceMem(size_t size)
{
    void* buf = nullptr;
    ze_device_mem_alloc_desc_t device_desc = { 
        ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC, 
        nullptr, 
        0, 
        0 
    };

    result = zeMemAllocDevice(context, &device_desc, size, 1, pDevice, &buf);
    CHECK_ZE_STATUS(result, "zeMemAllocDevice");

    return buf;
}

int testVectorAdd()
{
    // Create module
    std::vector<char> kernel_binary;
    if (readKernel(kernel_binary) != 0)
        return -1;
    ze_module_handle_t module = nullptr;
    ze_module_desc_t module_desc = {};
    module_desc.stype = ZE_STRUCTURE_TYPE_MODULE_DESC;
    module_desc.pNext = nullptr;
    module_desc.format = ZE_MODULE_FORMAT_IL_SPIRV;
    module_desc.inputSize = static_cast<uint32_t>(kernel_binary.size());
    module_desc.pInputModule = reinterpret_cast<const uint8_t *>(kernel_binary.data());
    module_desc.pBuildFlags = nullptr;
    result = zeModuleCreate(context, pDevice, &module_desc, &module, nullptr);
    CHECK_ZE_STATUS(result, "zeModuleCreate");

    // Create kernel
    ze_kernel_handle_t function = nullptr;
    ze_kernel_desc_t function_desc = {};
    function_desc.stype = ZE_STRUCTURE_TYPE_KERNEL_DESC;
    function_desc.pNext = nullptr;
    function_desc.flags = 0;
    function_desc.pKernelName = kernel_func_name;
    result = zeKernelCreate(module, &function_desc, &function);
    CHECK_ZE_STATUS(result, "zeKernelCreate");

    const size_t elem_count = 1024;
    std::vector<uint32_t> host_src(elem_count, 0);
    std::vector<uint32_t> host_dst(elem_count, 0);
    for (size_t i = 0; i < elem_count; i++)
        host_src[i] = i;

    void *src1_buf = allocDeviceMem(elem_count*sizeof(uint32_t));
    void *src2_buf = allocDeviceMem(elem_count*sizeof(uint32_t));
    void *dst_buf = allocDeviceMem(elem_count*sizeof(uint32_t));
#if 1
    // initialize src1 buffer
    result = zeCommandListAppendMemoryCopy(command_list, src1_buf, host_src.data(), elem_count*sizeof(uint32_t), nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendMemoryCopy");

    result = zeCommandListAppendBarrier(command_list, nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendBarrier");

    result = zeCommandListClose(command_list);
    CHECK_ZE_STATUS(result, "zeCommandListClose");

    result = zeCommandQueueExecuteCommandLists(command_queue, 1, &command_list, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandQueueExecuteCommandLists");

    result = zeCommandQueueSynchronize(command_queue, UINT64_MAX);
    CHECK_ZE_STATUS(result, "zeCommandQueueSynchronize");

    // initialize src2 buffer
    result = zeCommandListAppendMemoryCopy(command_list, src2_buf, host_src.data(), elem_count*sizeof(uint32_t), nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendMemoryCopy");

    result = zeCommandListAppendBarrier(command_list, nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendBarrier");

    result = zeCommandListClose(command_list);
    CHECK_ZE_STATUS(result, "zeCommandListClose");

    result = zeCommandQueueExecuteCommandLists(command_queue, 1, &command_list, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandQueueExecuteCommandLists");

    result = zeCommandQueueSynchronize(command_queue, UINT64_MAX);
    CHECK_ZE_STATUS(result, "zeCommandQueueSynchronize");
#endif
    // set kernel arguments
    result = zeKernelSetArgumentValue(function, 0, sizeof(src1_buf), &src1_buf);
    CHECK_ZE_STATUS(result, "zeKernelSetArgumentValue");
    
    result = zeKernelSetArgumentValue(function, 1, sizeof(src2_buf), &src2_buf);
    CHECK_ZE_STATUS(result, "zeKernelSetArgumentValue");

    result = zeKernelSetArgumentValue(function, 2, sizeof(dst_buf), &dst_buf);
    CHECK_ZE_STATUS(result, "zeKernelSetArgumentValue");

    ze_group_count_t group_count = { elem_count, 1, 1 };
    result = zeCommandListAppendLaunchKernel(command_list, function, &group_count, nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendLaunchKernel");

    result = zeCommandListAppendBarrier(command_list, nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendBarrier");

    result = zeCommandListClose(command_list);
    CHECK_ZE_STATUS(result, "zeCommandListClose");

    result = zeCommandQueueExecuteCommandLists(command_queue, 1, &command_list, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandQueueExecuteCommandLists");

    result = zeCommandQueueSynchronize(command_queue, UINT64_MAX);
    CHECK_ZE_STATUS(result, "zeCommandQueueSynchronize");

    result = zeCommandListAppendMemoryCopy(command_list, host_dst.data(), dst_buf, elem_count*sizeof(uint32_t), nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendMemoryCopy");

    result = zeCommandListClose(command_list);
    CHECK_ZE_STATUS(result, "zeCommandListClose");

    result = zeCommandQueueExecuteCommandLists(command_queue, 1, &command_list, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandQueueExecuteCommandLists");

    result = zeCommandQueueSynchronize(command_queue, UINT64_MAX);
    CHECK_ZE_STATUS(result, "zeCommandQueueSynchronize");

    printf("Print first 256 bytes of dst buf: ");
    for (size_t i = 0; i < 256; i++) {
        if (i % 16 == 0)
            printf("\n");
        printf("%03d, ", host_dst[i]);
    }
    printf("\n");

    int mismatch_count = 0;
    for (size_t i = 0; i < elem_count; i++)
        if (host_dst[i] != i*2)
            mismatch_count++;

    if(!mismatch_count)
        printf("INFO: copy test passed. elem_count = %d \n", elem_count);
    else
        printf("INFO: copy test failed!!! elem_count = %d, mismatch_count = %d \n", elem_count, mismatch_count);
    
    return 0;
}

int main()
{
    initZe();

    // testCopyMem();

    testVectorAdd();

    printf("done\n");

    return 0;
}