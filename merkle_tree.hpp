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

#include <openssl/sha.h>
extern int BLOCK_SIZE;

using namespace std;
/*
using std::vector;
using std::unordered_map;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ios;
using std::filesystem;
using std::queue;
*/

// Indicate the node to be the LEFT or RIGHT child of its parent
enum LeftOrRightSib {
  NA,
  LEFT,
  RIGHT
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
   vector<Block> _blocks;

  public:
   vector<Block> const& blocks();
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
  unsigned char hash[SHA256_DIGEST_LENGTH];

  MerkleNode();
  MerkleNode(string hash_str);
  MerkleNode(const Block &block);
  MerkleNode(MerkleNode* lhs, MerkleNode* rhs);
  MerkleNode(MerkleNode cur_node, MerkleNode* sibling);
  MerkleNode(MerkleNode cur_node, MerkleNode sibling);

  void print_hash();
};


// MerkleTree, its constructors and verify functions
class MerkleTree {
 private:
  vector<MerkleNode*> hashes;
  unordered_map<string, MerkleNode*> hash_leaf_map;

  void delete_tree_walker(MerkleNode* cur_node);
  MerkleNode* make_tree_from_hashes(vector<MerkleNode *>& cur_layer_nodes);
  MerkleNode* make_tree_from_blocks(Blocks& blocks);
  bool verify(MerkleNode cur_node, vector<MerkleNode*>& siblings);

 public:
  MerkleNode* root;
  void print();
  string root_hash();
  void print_root_hash();

  MerkleTree() {};
  MerkleTree(Blocks& blocks_);
  MerkleTree(unsigned char* data, int data_len);

  void delete_tree();
  void append(Blocks& new_blocks);
  void append(unsigned char* data, int data_len);

  vector<MerkleNode*> find_siblings(MerkleNode* leaf);
  vector<MerkleNode> find_siblings(string hash_str);

  bool verify(unsigned char* data, int data_len);
  bool verify(Block& block);
  bool verify(string hash_str);
  bool verify(string hash_str, vector<MerkleNode> &siblings,
              string root_hash);
};

// Utility functions
string hash_to_hex_string(unsigned char *hash, int size);
void hex_string_to_hash(string hash_str, unsigned char* hash, int size);


#endif /* MERKLE_TREE_HPP */
