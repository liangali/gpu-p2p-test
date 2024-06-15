# level-zero-p2p

lz_add

```bash
cd lz_add/build
cmake ..
make
./add

# run with drm trace
cp ../../*.py ./
python ./auto.py ./add
```

lz_p2p

```bash
cd lz_p2p/build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-O0 -g"
make
./p2p

# run with drm trace
cp ../../*.py ./
python ./auto.py ./p2p
```