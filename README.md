# level-zero-p2p

## build projects

```bash
sudo apt update
sudo apt install opencl-headers
sudo apt install ocl-icd-opencl-dev

cd gpu-p2p-test
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-O0 -g"
make
```

## run tests

```bash
cd build/lz_p2p
./lzp2p -l 0 -r 1 -n 4m

cd build/ocl_p2p
./oclp2p

cd build/interop
./interop

# run with drm trace
sudo apt update
sudo apt install trace-cmd
cd build/lz_p2p
cp ../../auto.py ./
python ./auto.py "./lzp2p -l 0 -r 1 -n 4m"
```

lz_p2p Results

```
(base) gta@xxxxxx:~/data/lz_samples/level-zero-p2p/lz_p2p/build$ ./p2p -l 0 -r 1 -n 16m
#### Input parameters: loca_ gpu idx = 0, remote_gpu idx = 1, data_count = 16777216
INFO: Enter lzContext 
INFO: Enter lzContext 
INFO: driver count = 1
#### device count = [0/2], devcie_name = Intel(R) Data Center GPU Flex 170
Found 1 device...
Driver version: 17002962
API version: 65539
INFO: find device handle = 0x5654b5a964d0
INFO: driver count = 1
#### device count = [0/2], devcie_name = Intel(R) Data Center GPU Flex 170
#### device count = [1/2], devcie_name = Intel(R) Data Center GPU Flex 170
Found 1 device...
Driver version: 17002962
API version: 65539
INFO: find device handle = 0x5654b5a9e190
queryP2P, dev0 = 0x5654b5a964d0, dev1 = 0x5654b5a9e190, flags = 1
queryP2P, dev0 = 0x5654b5a9e190, dev1 = 0x5654b5a964d0, flags = 1
buf0 = 0xffff80ac00200000, buf1 = 0xffff80d600200000
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 
Kernel timestamp statistics (prior to V1.2): 
        Global start : 1157526896 cycles
        Kernel start: 333686 cycles
        Kernel end: 8036576 cycles
        Global end: 1165229784 cycles
        timerResolution: 52 ns
        Kernel duration : 7702890 cycles
        Kernel Time: 400550.280000 us
#### gpuKernelTime = 400550.280000, elemCount = 16777216, Bandwidth = 0.167542 GB/s
3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 
Kernel timestamp statistics (prior to V1.2): 
        Global start : 1165369906 cycles
        Kernel start: 8139744 cycles
        Kernel end: 8303944 cycles
        Global end: 1165534104 cycles
        timerResolution: 52 ns
        Kernel duration : 164200 cycles
        Kernel Time: 8538.400000 us
#### gpuKernelTime = 8538.400000, elemCount = 16777216, Bandwidth = 7.859653 GB/s
15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210, 225, 240, 
done
INFO: Enter ~lzContext 
INFO: Enter ~lzContext
```

ocl_p2p Results

```
Platform 0x5594724d5970 has 2 GPU devices
Created device for devIdx = 0 on Intel(R) Data Center GPU Flex 170, device = 0x5594724d5a40, contex = 0x5594724dad20, queue = 0x55947154d260
Platform 0x5594724d5970 has 2 GPU devices
Created device for devIdx = 0 on Intel(R) Data Center GPU Flex 170, device = 0x5594724d5a40, contex = 0x55947154eb10, queue = 0x5594724e51e0
buf0 = 0xffff8081fff60000, buf1 = 0xffff8081ffdf0000
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 
ERROR: oclContext::runKernel, line = 138, clSetKernelArg failed! err = -50 (CL_INVALID_ARG_VALUE)
```

interop P2P results

```
(base) gta@DUT7113ATSM:~/data/gpu_p2p_code/gpu-p2p-test/build/interop$ ./interop 
Platform 0x5637b9daf750 has 2 GPU devices
Created device for devIdx = 0 on Intel(R) Data Center GPU Flex 170, device = 0x5637b9daf820, contex = 0x5637b9db4b00, queue = 0x5637b8bc4070
Platform 0x5637b9daf750 has 2 GPU devices
Created device for devIdx = 1 on Intel(R) Data Center GPU Flex 170, device = 0x5637b9db1c00, contex = 0x5637b8bc5920, queue = 0x5637b9dc0460
INFO: Enter lzContext 
INFO: Enter lzContext 
INFO: driver count = 1
#### device count = [0/2], devcie_name = Intel(R) Data Center GPU Flex 170
Found 1 device...
Driver version: 17002962
API version: 65539
INFO: find device handle = 0x5637bb33af20
INFO: driver count = 1
#### device count = [0/2], devcie_name = Intel(R) Data Center GPU Flex 170
#### device count = [1/2], devcie_name = Intel(R) Data Center GPU Flex 170
Found 1 device...
Driver version: 17002962
API version: 65539
INFO: find device handle = 0x5637bb342e20
The first 16 elements in cl_mem = 0x5637bb36ac50 are: 
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
The first 16 elements in cl_mem = 0x5637bb370230 are: 
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
handle = 9, bufSize = 4194304
MemAllocINFO: memory = 0xffff81d5ff600000, stype = 0, pNext = 0x00000000, type = 2, id = 0x00000002, pagesize = 0
handle = 10, bufSize = 4194304
MemAllocINFO: memory = 0xffff81ffff400000, stype = 0, pNext = 0x00000000, type = 2, id = 0x00000003, pagesize = 0
The first 16 elements in level-zero ptr = 0xffff81d5ff600000 are: 
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
The first 16 elements in level-zero ptr = 0xffff81ffff400000 are: 
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
Kernel timestamp statistics (prior to V1.2): 
        Global start : 3927982536 cycles
        Kernel start: 31692 cycles
        Kernel end: 398610 cycles
        Global end: 3928349454 cycles
        timerResolution: 52 ns
        Kernel duration : 366918 cycles
        Kernel Time: 19079.736000 us
#### gpuKernelTime = 19079.736000, elemCount = 1048576, Bandwidth = 0.219830 GB/s
Kernel timestamp statistics (prior to V1.2): 
        Global start : 3928364018 cycles
        Kernel start: 399084 cycles
        Kernel end: 414358 cycles
        Global end: 3928379292 cycles
        timerResolution: 52 ns
        Kernel duration : 15274 cycles
        Kernel Time: 794.248000 us
#### gpuKernelTime = 794.248000, elemCount = 1048576, Bandwidth = 5.280849 GB/s
The first 16 elements in cl_mem = 0x5637bb36ac50 are: 
0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 
The first 16 elements in cl_mem = 0x5637bb370230 are: 
0, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210, 225, 
INFO: Enter ~lzContext 
INFO: Enter ~lzContext 
Enter ~oclContext
Enter ~oclContext
```
