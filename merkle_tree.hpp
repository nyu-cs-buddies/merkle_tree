#ifndef MERKLE_TREE_HPP
#define MERKLE_TREE_HPP

#include <iostream>
#include <cstring>
#include <cmath>
#include <fstream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

extern int BLOCK_SIZE;

// Indicate the node to be the LEFT or RIGHT child of its parent
enum LeftOrRightSib {
  NA,
  LEFT,
  RIGHT
};

// GPU acceration bit masks
#define NO_ACCEL          1
#define ACCEL_CREATION    2
#define ACCEL_REDUCTION   4
#define ACCEL_RESERVED_1  8
#define ACCEL_RESERVED_2  16

// Hash algorithms
class Hasher {
 protected:
  int digest_size;

 public:
  virtual void get_hash(unsigned char *data, int data_len,
                        unsigned char *hash) = 0;
  virtual void get_hash(unsigned char* din, int block_size,
                        unsigned char* dout, int num_of_blocks) {}
  int hash_length() const {
    return digest_size;
  }
};

class SHA_256 : public Hasher {
 public:
  SHA_256();
  void get_hash(unsigned char* data,
                int data_len,
                unsigned char* hash) override;
};

class MD_5 : public Hasher {
 public:
  MD_5();
  void get_hash(unsigned char* data,
                int data_len,
                unsigned char* hash) override;
};

class SHA_256_GPU : public Hasher {
 public:
  SHA_256_GPU();
  void get_hash(unsigned char* data,
                int data_len,
                unsigned char* hash) override;
  void get_hash(unsigned char* din,
                int block_size,
                unsigned char* dout,
                int num_of_blocks) override;
};

class MD_5_GPU : public Hasher {
 public:
  MD_5_GPU();
  void get_hash(unsigned char* data,
                int data_len,
                unsigned char* hash) override;
  void get_hash(unsigned char* din,
                int block_size,
                unsigned char* dout,
                int num_of_blocks) override;
};

// Basic data block
class Block {
  public:
   unsigned char* data;
   Block();
};

// Collection of blocks
class Blocks {
  private:
   std::vector<Block> _blocks;

  public:
   std::vector<Block> const& blocks();
   Blocks() {}
   ~Blocks();
   Blocks(unsigned char* data, int data_len);
   void add_blocks(Blocks& new_blocks);
};

// MerkleNode and its constructors
class MerkleNode {
 public:
  MerkleNode* parent;
  MerkleNode* left;
  MerkleNode* right;
  LeftOrRightSib lr;
  unsigned char* hash;
  int digest_len;

  MerkleNode();
  MerkleNode(unsigned char* hash, int digest_len_);
  MerkleNode(std::string hash_str, Hasher* hasher);
  MerkleNode(const Block &block, Hasher* hasher);
  MerkleNode(MerkleNode* lhs, MerkleNode* rhs, Hasher* hasher);
  MerkleNode(MerkleNode cur_node, MerkleNode* sibling, Hasher* hasher);
  MerkleNode(MerkleNode cur_node, MerkleNode sibling, Hasher* hasher);

  // TODO(allenpthuang): potential memory leak here
  // ~MerkleNode() {
  //   delete hash;
  // }
  void print_hash();
  void print_info();
};


// MerkleTree, its constructors and verify functions
class MerkleTree {
 private:
  std::vector<MerkleNode*> hashes;
  std::unordered_map<std::string, MerkleNode*> hash_leaf_map;
  Hasher* hasher;

  void delete_tree_walker(MerkleNode* cur_node);
  MerkleNode* make_tree_from_hashes(std::vector<MerkleNode *>& cur_layer_nodes);
  MerkleNode* make_tree_from_blocks(Blocks& blocks);
  bool verify(MerkleNode cur_node, std::vector<MerkleNode*>& siblings);

 public:
  MerkleNode* root;
  void print();
  void print_root_hash();
  std::string root_hash();
  void xr();

  MerkleTree() {}
  MerkleTree(Hasher* hasher_);
  MerkleTree(Blocks& blocks_, Hasher* hasher_);
  MerkleTree(unsigned char* data, int data_len, Hasher* hasher_);
  MerkleTree(unsigned char* data, int data_len, Hasher* hasher_,
             unsigned short accel_mask);

  void delete_tree();
  void append(Blocks& new_blocks);
  void append(unsigned char* data, int data_len);

  std::vector<MerkleNode*> find_siblings(MerkleNode* leaf);
  std::vector<MerkleNode> find_siblings(std::string hash_str);

  bool verify(unsigned char* data, int data_len);
  bool verify(Block& block);
  bool verify(std::string hash_str);
  bool verify(std::string hash_str, std::vector<MerkleNode> &siblings,
              std::string root_hash);
};

// Utility functions
std::string hash_to_hex_string(unsigned char *hash, int size);
void hex_string_to_hash(std::string hash_str, unsigned char* hash, int size);


#endif /* MERKLE_TREE_HPP */
