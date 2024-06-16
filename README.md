# level-zero-p2p

lz_add

```bash
cd lz_add/build
cmake ..
make
./add
```

lz_p2p

```bash
cd lz_p2p/build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-O0 -g"
make
./p2p -l 0 -r 1 -n 4m

# run with drm trace
sudo apt update
sudo apt install trace-cmd
cd lz_p2p/build
cp ../../auto.py ./
python ./auto.py "./p2p -l 0 -r 1 -n 4m"
```

Results

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