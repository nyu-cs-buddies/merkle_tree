# Merkle Tree
Merkle Tree by GPU

## Team Members
- Morris (Chun-Ting) Shen 
- Oscar Hsu
- Po-Yuan Huang
- Pin-Tsung Huang

## Code Locations
Currently, there are two versions with separate `README.md` in their directories:
- CPU: `merkle_tree/merkle_tree_cpu`
- GPU: `merkle_tree/merkle_tree_gpu`


## Preliminary Results
By offloading hashing algorithms to the GPU side, we are seeing signigicant
speed-ups in data sizes larger than 1GB, and it's most significant in 10GB with
block size set to 100KB in our preliminary tests.

The speed-up charts are as follow (x-axis is `block_size`):
- `data_len = 1000000000` (~ 1GB)
  ![1GB Speed-up Chart](pix/1GB_speedup.png)
- `data_len = 10000000000` (~ 10GB)
  ![10GB Speed-up Chart](pix/10GB_speedup.png)


## Generate/Load `TestData`
```
#include "testdata.hpp"

// cache_path is default to "cached_test_data" in benchmark programs.
TestData td(data_len, block_size, "CPU", cache_path);

// return a tuple of (string, unsigned char*, unsigned long long).
tie(config, data, data_len) = td.get_test_data();
```
`get_test_data()` return the tuple above directly if test data has been loaded
from cache files (or generated and then loaded).

Test data generation uses `<random>` with a fixed seed (`42`) from C++ library.

## Usage of `Timer`
```
#include "timer.hpp"

start_timer("Test_1_GPU");  // insert a config name here
// do_something_expensive();
stop_timer();

print_timer();
// Test_1_GPU:  55688 ms

print_timer_csv();
// Test_1_GPU,55688
```

## Credits
We use GPU versions of hash algorithms from
[CUDA Hashing Algorithms Collection](https://github.com/mochimodev/cuda-hashing-algos) by Matt Zweil & The Mochimo Core Contributor Team.
