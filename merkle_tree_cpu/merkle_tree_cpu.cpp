#include <iostream>
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

  MerkleTree(Blocks& blocks) {
    root = make_tree_from_blocks(blocks);
  }
};

int main(int argc, char *argv[]) {
  unsigned char* data;
  int data_len = 0;
  if (argc == 1) {
    cerr << "Usage: ./merkle_tree_cpu <filename>" << endl;
    cerr << "For demo, creating data filed with '9527' with 4096 bytes." << endl;
    data = (unsigned char*) malloc(BLOCK_SIZE * 4 * sizeof(unsigned char));
    data_len = BLOCK_SIZE * 4;
    memset(data + BLOCK_SIZE * 0, 9, BLOCK_SIZE);
    memset(data + BLOCK_SIZE * 1, 5, BLOCK_SIZE);
    memset(data + BLOCK_SIZE * 2, 2, BLOCK_SIZE);
    memset(data + BLOCK_SIZE * 3, 7, BLOCK_SIZE);
  } else {
    fs::path p{argv[1]};
    if (! fs::exists(p)) {
      cerr << "File not found at: " << fs::absolute(p) << endl;
      exit(2);
    }
    cout << "path = " << fs::absolute(p) << endl;
    cout << "filesize = " << fs::file_size(p) << endl;
    data_len = fs::file_size(p);
    data = (unsigned char*) malloc(data_len * sizeof(unsigned char));
    ifstream is;
    is.open(p, ios::binary);
    is.read((char*)data, data_len);
  }
  Blocks blocks(data, data_len);
  MerkleTree merkle_tree(blocks);
  merkle_tree.print();
  return 0;
}