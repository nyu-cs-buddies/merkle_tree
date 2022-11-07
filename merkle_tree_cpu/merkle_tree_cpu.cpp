#include <iostream>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <queue>
#include <vector>

#include <openssl/sha.h>
#define BLOCK_SIZE 1024

using namespace std;
namespace fs = filesystem;

class Block {
  public:
   unsigned char data[BLOCK_SIZE];
   Block() {
    memset(&data, 0, BLOCK_SIZE);
   }
};

class Blocks {
  private:
   vector<Block> _blocks;

  public:
   vector<Block> const& blocks() {
    return _blocks;
   }
   Blocks() {}
   Blocks(unsigned char* data, int data_len) {
    int num_of_blocks = data_len / BLOCK_SIZE;
    int offset = 0;
    for (int i = 0; i < num_of_blocks; i++) {
      Block b;
      memcpy(&b.data, data + offset, BLOCK_SIZE);
      offset += BLOCK_SIZE;
      _blocks.push_back(b);
    }
    if (offset < data_len) {
      Block b;
      memcpy(&b.data, data + offset, data_len - offset);
      _blocks.push_back(b);
    }
   }
   void add_blocks(Blocks& new_blocks) {
    _blocks.insert(_blocks.end(), new_blocks.blocks().begin(),
                   new_blocks.blocks().end());
   }
};

class MerkleNode {
 public:
  MerkleNode* left;
  MerkleNode* right;
  unsigned char hash[SHA256_DIGEST_LENGTH];

  MerkleNode() : left(nullptr), right(nullptr) {}

  MerkleNode(const Block& block) : left(nullptr), right(nullptr) {
    SHA256(block.data, BLOCK_SIZE, hash);
  }

  MerkleNode(MerkleNode* lhs, MerkleNode* rhs) {
    unsigned char data[SHA256_DIGEST_LENGTH * 2];
    memcpy(data, lhs->hash, SHA256_DIGEST_LENGTH);
    memcpy(data + SHA256_DIGEST_LENGTH, rhs->hash, SHA256_DIGEST_LENGTH);
    SHA256(data, SHA256_DIGEST_LENGTH * 2, hash);
    left = lhs;
    right = rhs;
  }

  void print_hash() {
    for (const auto& h : hash) {
      printf("%02x", h);
    }
    cout << endl;
  }
};

class MerkleTree {
 private:
  MerkleNode* root;
  Blocks blocks;

 public:
  void print() {
    queue<MerkleNode*> q;
    q.push(root);
    int layer = 0;
    while (! q.empty() && q.front() != nullptr) {
      cout << "Layer " << layer << ":" << endl;
      int size = q.size();
      while (size > 0) {
        auto node = q.front();
        q.pop();
        node->print_hash();
        if (node->left != nullptr) {
          q.push(node->left);
        }
        if (node->right != nullptr) {
          q.push(node->right);
        }
        size--;
      }
      layer++;
   }
  }

  MerkleNode* make_tree_from_blocks(Blocks& blocks) {
    if (blocks.blocks().empty()) {
      return nullptr;
    }
    vector<MerkleNode*> cur_layer_nodes;
    for (const auto& block : blocks.blocks()) {
      cur_layer_nodes.push_back(new MerkleNode(block));
    }
    
    while (cur_layer_nodes.size() > 1) {
      int count = 0;
      for (int i = 0; i < cur_layer_nodes.size() - 1; i = i + 2) {
        cur_layer_nodes[count] =
            new MerkleNode(cur_layer_nodes[i], cur_layer_nodes[i + 1]);
        count++;
      }
      if (count > 0 && cur_layer_nodes.size() % 2 != 0) {
        cur_layer_nodes[count] = cur_layer_nodes[cur_layer_nodes.size() - 1];
        cur_layer_nodes.resize(count + 1);
      } else {
        cur_layer_nodes.resize(count);
      }
    }
    return cur_layer_nodes[0];
  }

  MerkleTree(Blocks& blocks_) {
    blocks = blocks_;
    root = make_tree_from_blocks(blocks);
  }

  void delete_tree_walker(MerkleNode* cur_node) {
    if (cur_node == nullptr) {
      return;
    }
    delete_tree_walker(cur_node->left);
    delete_tree_walker(cur_node->right);
    delete(cur_node);
  }

  void delete_tree() {
    delete_tree_walker(root);
    root = nullptr;
  }

  // TODO(allenpthuang): Naive way to insert blocks! Should be more efficient.
  void insert(Blocks& new_blocks) {
    blocks.add_blocks(new_blocks);
    delete_tree();
    root = make_tree_from_blocks(blocks);
  }
};

int main(int argc, char *argv[]) {
  unsigned char* data;
  int data_len = 0;
  if (argc == 1) {
    // no input file; use dummy data for demo.
    cerr << "Usage: ./merkle_tree_cpu <filename>" << endl;
    cerr << "For demo, create data filed with '9527' with 4096 bytes." << endl;
    data = (unsigned char *)malloc(BLOCK_SIZE * 4 * sizeof(unsigned char));
    data_len = BLOCK_SIZE * 4;
    memset(data + BLOCK_SIZE * 0, 9, BLOCK_SIZE);
    memset(data + BLOCK_SIZE * 1, 5, BLOCK_SIZE);
    memset(data + BLOCK_SIZE * 2, 2, BLOCK_SIZE);
    memset(data + BLOCK_SIZE * 3, 7, BLOCK_SIZE);
  } else {
    // input filepath provided
    fs::path p{argv[1]};
    if (! fs::exists(p)) {
      cerr << "File not found at: " << fs::absolute(p) << endl;
      exit(2);
    }
    // show file info and read into a buffer
    cout << "path = " << fs::absolute(p) << endl;
    cout << "filesize = " << fs::file_size(p) << endl;
    data_len = fs::file_size(p);
    data = (unsigned char *)malloc(data_len * sizeof(unsigned char));
    ifstream is;
    is.open(p, ios::binary);
    is.read((char*)data, data_len);
  }

  // make blocks and make a merkle tree from them
  Blocks blocks(data, data_len);
  MerkleTree merkle_tree(blocks);
  cout << "===== Read all at once. =====" << endl;
  merkle_tree.print();

  // split input data into two halves; the second half is inserted later.
  int num_of_blocks = ceil((double)data_len / BLOCK_SIZE);
  if (num_of_blocks > 1) {
    int first_size = BLOCK_SIZE * (num_of_blocks / 2);
    Blocks old_blocks(data, first_size);
    MerkleTree merkle_tree_to_insert(old_blocks);
    cout << "===== Read half first, and append the other half. =====" << endl;
    cout << "=== Merkle Tree of the first half ===" << endl;
    merkle_tree_to_insert.print();

    Blocks new_blocks(data + first_size, data_len - first_size);
    merkle_tree_to_insert.insert(new_blocks);
    cout << "=== Merkle Tree of the first half + the second half ===" << endl;
    merkle_tree_to_insert.print();
  }

  return 0;
}
