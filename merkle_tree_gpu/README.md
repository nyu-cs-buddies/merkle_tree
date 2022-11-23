# Merkle Tree - GPU Version
GPU implementation of Merkle Tree.

## Current Status: WIP
- Hashing has already been moved to GPU.

## Usage - Demo
Note: Makefile now `module load cuda-11.4 gcc-11.2` if you are on CUDA machines at NYU.
```
../make gpu
```

Use it with a file with the `BLOCK_SIZE` setting:
```
../bin/merkle_tree_gpu_demo <BLOCK_SIZE> <filename>
```

Use it without any arguments to demo with dummy data:
```
../bin/merkle_tree_gpu_demo 
```

## Usage
### Choose a Hash Algorithm
Currently, there are two hash algorithms available:
- SHA256
- MD5

Before building the Merkle Tree, instantiate a `Hasher` first:
```
// SHA256
Hasher* hasher = new SHA_256_GPU();
```
or
```
// MD5
Hasher* hasher = new MD_5_GPU();
```

With `hasher` created in the previous section, we have:
`MerkleTree merkle_tree(data, data_len, hasher);`
- `data`: `unsigned char *`
- `data_len`: `int`
- `hasher`: `Hasher *`

The resulting MerkleTree is at `merkle_tree.root`, and its root hash is
`merkle_tree.root_hash()`.

### Append data to an existing MerkleTree, get an updated MerkleTree.
`merkle_tree.append(data, data_len);`
- `data`: `unsigned char *`
- `data_len`: `int`

The new tree is rooted at `merkle_tree.root`.

### Inclusive Proof: Verify whether data is in the MerkleTree
Suppose we have a block of data to verify, we first obtain the hash string
of the block.
```
unsigned char hash[SHA256_DIGEST_LENGTH];
SHA256(data, BLOCK_SIZE, hash); // length of data has to be exact one block.
string hash_str = hash_to_hex_string(data, SHA256_DIGEST_LENGTH);
```

Or, alternatively
```
unsigned char* hash =
      (unsigned char*)calloc(hasher->hash_length(), sizeof(unsigned char));
hasher->get_hash(data, BLOCK_SIZE, hash);
string hash_str = hash_to_hex_string(hash, hasher->hash_length());
string root_hash = merkle_tree.root_hash();
```

#### Verify with a hash string and the original MerkleTree
`bool verified = merkle_tree.verify(hash_str);`

#### Verify with a hash string and only sibling MerkleNodes
```
// need a hasher too
Hasher* client_hasher = new SHA_256_GPU();
MerkleTree local_tree(client_hasher);
bool verified = local_tree.verify(hash_str, siblings, root_hash);
```
Where `hash_str` is from our block to be verified, `root_hash` is what
we already have, and `siblings` from the following:
`vector<MerkleTree> siblings = merkle_tree.find_siblings(hash_str);`

### Verify with raw data
This breaks input data into blocks in size of `BLOCK_SIZE`, and then
verifies them all.
`merkle_tree.verify(data, data_len);`
- `data`: `unsigned char *`
- `data_len`: `int`
