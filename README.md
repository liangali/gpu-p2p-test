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
python ./auto.py ./p2p -l 0 -r 1 -n 4m
```