# Merkle Tree - GPU Version
GPU implementation of Merkle Tree.

## Current Status: WIP
Project can now be compiled on cuda2@cims with `nvcc`, but no real GPU
acceleration is applied yet.

## Usage - Demo
```
module load gcc-11.2  # check `module avail gcc`
../make
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
### Create a MerkleTree from raw data
`MerkleTree merkle_tree(data, data_len);`
- `data`: `unsigned char *`
- `data_len`: `int`

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

#### Verify with a hash string and the original MerkleTree
`bool verified = merkle_tree.verify(hash_str);`

#### Verify with a hash string and only sibling MerkleNodes
```
MerkleTree local_tree;
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
