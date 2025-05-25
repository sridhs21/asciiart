# ASCII Art
Simple program which converts image to ascii art.

### How to run and build:

```
sudo apt-get install libopencv-dev

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

./ascii_converter test.png 50 0    # 200 chars wide, minimal charset
./ascii_converter test.png 100 1   # 150 chars wide, detailed charset
./ascii_converter test.png 150 2   # 200 chars wide, block charset
```
