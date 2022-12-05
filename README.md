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

## Acceleration Methods
Currently, we have accelerated four Merkle Tree operations:
- Creation (`ACCEL_CREATION`): from raw data, make Merkle leaf nodes.
- Reducation (`ACCEL_REDUCTION`): from all Merkle leaf nodes, derive root node.
  * Use this without `ACCEL_LINK` will leave linking of `MerkleNode`s
    (`left`, `right` and `parent` pointers) on the CPU side.
- Link (`ACCEL_LINK`): enable linking on the GPU side.
- Hashmap (`ACCEL_HASHMAP`): move the creation and lookup of the hashmap,
  which maps a hash (or `hash_str`) to a `MerkleNode*`, enabling inclusion
  proof on the GPU side.

**(Only applies to GPU version!)**
To set the acceleration mode, use a bitmask:
```C++
unsigned short accel_mask = NO_ACCEL;
MerkleTree mt(data, data_len, hasher, accel_mask);
// no acceleration happens, pure CPU.
```
or
```C++
unsigned short accel_mask = ACCEL_CREATION;
MerkleTree mt(data, data_len, hasher, accel_mask);
// creation acceleration
```
or
```C++
unsigned short accel_mask = 0;
accel_mask |= ACCEL_CREATION;
accel_mask |= ACCEL_REDUCTION;
accel_mask |= ACCEL_LINK;
accel_mask |= ACCEL_HASHMAP;
MerkleTree mt(data, data_len, hasher, accel_mask);
// full GPU acceleration
```


## Preliminary Results
By offloading hashing algorithms to the GPU side, we are seeing signigicant
speed-ups in data sizes larger than 1GB, and it's most significant in 10GB with
block size set to 100 bytes in our preliminary tests.

The best speed-up we can achieve at the moment is around **50x** with a fully-fuctional Merkle Tree.

The speed-up charts are as follow (x-axis is `block_size`):

#### Creation
- `data_len = 1000000000` (~ 1GB)
  ![1GB Speed-up Chart](pix/1GB_speedup_lv1.png)
- `data_len = 10000000000` (~ 10GB)
  ![10GB Speed-up Chart](pix/10GB_speedup_lv1.png)

#### Creation + Reduction
- `data_len = 1000000000` (~ 1GB)
  ![1GB Speed-up Chart](pix/1GB_speedup_lv2.png)
- `data_len = 10000000000` (~ 10GB)
  ![10GB Speed-up Chart](pix/10GB_speedup_lv2.png)

#### Creation + Reduction + Link
- `data_len = 1000000000` (~ 1GB)
  ![1GB Speed-up Chart](pix/1GB_speedup_lv3.png)
- `data_len = 10000000000` (~ 10GB)
  ![10GB Speed-up Chart](pix/10GB_speedup_lv3.png)

#### Creation + Reduction + Link + Hashmap (Full GPU Acceleration)
- `data_len = 1000000000` (~ 1GB)
  ![1GB Speed-up Chart](pix/1GB_speedup_lv4_full.png)
<!-- - `data_len = 10000000000` (~ 10GB)
  ![10GB Speed-up Chart](pix/10GB_speedup_lv4_full.png) -->

These are some even faster speed-ups for a partial-function Merkle Tree.

#### Root Hash Only
- `data_len = 1000000000` (~ 1GB)
  ![1GB Speed-up Chart](pix/1GB_speedup_root_hash_only.png)
- `data_len = 10000000000` (~ 10GB)
  ![10GB Speed-up Chart](pix/10GB_speedup_root_hash_only.png)

#### Root Hash + Link (MerkleTree traversable)
- `data_len = 1000000000` (~ 1GB)
  ![1GB Speed-up Chart](pix/1GB_speedup_root_hash_link.png)
- `data_len = 10000000000` (~ 10GB)
  ![10GB Speed-up Chart](pix/10GB_speedup_root_hash_link.png)


## Generate/Load `TestData`
```C++
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
```C++
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
We use and modify GPU versions of
- Hash algorithms from
[CUDA Hashing Algorithms Collection](https://github.com/mochimodev/cuda-hashing-algos) by Matt Zweil & The Mochimo Core Contributor Team.
- a Hashmap library from
[SimpleGPUHashTable](https://github.com/nosferalatu/SimpleGPUHashTable) by David Farrell.
