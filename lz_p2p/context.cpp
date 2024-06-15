
#include "context.h"

lzContext::lzContext()
{
    printf("INFO: Enter %s \n", __FUNCTION__);
}

lzContext::~lzContext()
{
    printf("INFO: Enter %s \n", __FUNCTION__);
}

ze_device_handle_t lzContext::findDevice(ze_driver_handle_t pDriver, ze_device_type_t type, int devIdx)
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
        printf("#### device count = [%d/%d], devcie_name = %s\n", device, deviceCount, device_properties.name);

        if (type == device_properties.type && device == devIdx)
        {
            found = phDevice;

            ze_driver_properties_t driver_properties = {};
            driver_properties.stype = ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES;
            zeDriverGetProperties(pDriver, &driver_properties);

            std::cout << "Found " << std::to_string(type) << " device..." << "\n";
            std::cout << "Driver version: " << driver_properties.driverVersion << "\n";

            ze_api_version_t version = {};
            zeDriverGetApiVersion(pDriver, &version);
            std::cout << "API version: " << std::to_string(version) << "\n";
            // std::cout << std::to_string(device_properties) << "\n";

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
                //std::cout << to_string( pMemoryProperties[ mem ] ) << "\n";
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

int lzContext::initZe(int devIdx)
{
    ze_result_t result;
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
        pDevice = findDevice(pDriver, type, devIdx);
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

void* lzContext::initBuffer(size_t elem_count)
{
    ze_result_t result; 
    elemCount = elem_count;

    std::vector<uint32_t> hostBuf(elemCount, 0);
    for (size_t i = 0; i < elemCount; i++)
        hostBuf[i] = i;

    ze_device_mem_alloc_desc_t device_desc = { 
        ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC, 
        nullptr, 
        0, 
        0 
    };
    result = zeMemAllocDevice(context, &device_desc, elemCount*sizeof(uint32_t), 1, pDevice, &devBuf);
    CHECK_ZE_STATUS(result, "zeMemAllocDevice");

    result = zeCommandListAppendMemoryCopy(command_list, devBuf, hostBuf.data(), elemCount*sizeof(uint32_t), nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendMemoryCopy");

    result = zeCommandListAppendBarrier(command_list, nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendBarrier");

    result = zeCommandListClose(command_list);
    CHECK_ZE_STATUS(result, "zeCommandListClose");

    result = zeCommandQueueExecuteCommandLists(command_queue, 1, &command_list, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandQueueExecuteCommandLists");

    result = zeCommandQueueSynchronize(command_queue, UINT64_MAX);
    CHECK_ZE_STATUS(result, "zeCommandQueueSynchronize");

    result = zeCommandListReset(command_list);
    CHECK_ZE_STATUS(result, "zeCommandListReset");

    return devBuf;
}

void lzContext::copyBuffer(std::vector<uint32_t> &hostBuf)
{
    ze_result_t result; 

    result = zeCommandListAppendMemoryCopy(command_list, hostBuf.data(), devBuf, elemCount*sizeof(uint32_t), nullptr, 0, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandListAppendMemoryCopy");

    result = zeCommandListClose(command_list);
    CHECK_ZE_STATUS(result, "zeCommandListClose");

    result = zeCommandQueueExecuteCommandLists(command_queue, 1, &command_list, nullptr);
    CHECK_ZE_STATUS(result, "zeCommandQueueExecuteCommandLists");

    result = zeCommandQueueSynchronize(command_queue, UINT64_MAX);
    CHECK_ZE_STATUS(result, "zeCommandQueueSynchronize");

    result = zeCommandListReset(command_list);
    CHECK_ZE_STATUS(result, "zeCommandListReset");

}

void queryP2P(ze_device_handle_t dev0, ze_device_handle_t dev1)
{
    ze_result_t result; 
    ze_device_p2p_properties_t p2pProperties = {};
    p2pProperties.stype = ZE_STRUCTURE_TYPE_DEVICE_P2P_PROPERTIES;
    p2pProperties.pNext = nullptr;
    p2pProperties.flags = 0;
    result = zeDeviceGetP2PProperties(dev0, dev1, &p2pProperties);

    printf("%s, dev0 = %p, dev1 = %p, flags = %d\n", __FUNCTION__, dev0, dev1, p2pProperties.flags);
}
