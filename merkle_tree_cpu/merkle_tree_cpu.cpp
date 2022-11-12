#include <iostream>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include <openssl/sha.h>
#define BLOCK_SIZE 1024

using namespace std;
namespace fs = filesystem;

enum LeftOrRightSib {
  NA,
  LEFT,
  RIGHT
};

string hash_to_hex_string(unsigned char *hash, int size) {
  char temp[3];
  string result = "";
  for (int i = 0; i < size; i++) {
    snprintf(temp, 3, "%02x", hash[i]);
    result += temp[0];
    result += temp[1];
  }
  return result;
}

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
  MerkleNode* parent;
  MerkleNode* left;
  MerkleNode* right;
  LeftOrRightSib lr;
  unsigned char hash[SHA256_DIGEST_LENGTH];

  MerkleNode() : parent(nullptr), left(nullptr), right(nullptr), lr(NA) {}

  MerkleNode(string hash_str)
      : parent(nullptr), left(nullptr), right(nullptr), lr(NA) {
        unsigned char buf;
        for (int i = 0; i < hash_str.size(); i += 2) {
          sscanf(hash_str.c_str() + i, "%02x", &buf);
          hash[i / 2] = buf;
        }
      }

  MerkleNode(const Block &block)
      : parent(nullptr), left(nullptr), right(nullptr), lr(NA) {
    SHA256(block.data, BLOCK_SIZE, hash);
  }

  MerkleNode(MerkleNode* lhs, MerkleNode* rhs) : lr(NA) {
    unsigned char data[SHA256_DIGEST_LENGTH * 2];
    memcpy(data, lhs->hash, SHA256_DIGEST_LENGTH);
    memcpy(data + SHA256_DIGEST_LENGTH, rhs->hash, SHA256_DIGEST_LENGTH);
    SHA256(data, SHA256_DIGEST_LENGTH * 2, hash);
    left = lhs;
    right = rhs;
    lhs->parent = this;
    rhs->parent = this;
    lhs->lr = LEFT;
    rhs->lr = RIGHT;
  }

  MerkleNode(MerkleNode cur_node, MerkleNode* sibling)
      : parent(nullptr), left(nullptr), right(nullptr), lr(NA) {
    unsigned char data[SHA256_DIGEST_LENGTH * 2];
    if (sibling->lr == LEFT) {
      memcpy(data, sibling->hash, SHA256_DIGEST_LENGTH);
      memcpy(data + SHA256_DIGEST_LENGTH, cur_node.hash, SHA256_DIGEST_LENGTH);
    } else {
      memcpy(data, cur_node.hash, SHA256_DIGEST_LENGTH);
      memcpy(data + SHA256_DIGEST_LENGTH, sibling->hash, SHA256_DIGEST_LENGTH);
    } 
    SHA256(data, SHA256_DIGEST_LENGTH * 2, hash);
  }

  MerkleNode(MerkleNode cur_node, MerkleNode sibling)
      : parent(nullptr), left(nullptr), right(nullptr), lr(NA) {
    unsigned char data[SHA256_DIGEST_LENGTH * 2];
    if (sibling.lr == LEFT) {
      memcpy(data, sibling.hash, SHA256_DIGEST_LENGTH);
      memcpy(data + SHA256_DIGEST_LENGTH, cur_node.hash, SHA256_DIGEST_LENGTH);
    } else {
      memcpy(data, cur_node.hash, SHA256_DIGEST_LENGTH);
      memcpy(data + SHA256_DIGEST_LENGTH, sibling.hash, SHA256_DIGEST_LENGTH);
    } 
    SHA256(data, SHA256_DIGEST_LENGTH * 2, hash);
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
  unordered_map<string, MerkleNode*> hash_leaf_map;

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

  string root_hash() {
    return hash_to_hex_string(root->hash, SHA256_DIGEST_LENGTH);
  }

  void print_root_hash() {
    cout << root_hash() << endl;
  }

  MerkleNode* make_tree_from_blocks(Blocks& blocks) {
    if (blocks.blocks().empty()) {
      return nullptr;
    }
    vector<MerkleNode*> cur_layer_nodes;
    for (const auto& block : blocks.blocks()) {
      MerkleNode* to_add = new MerkleNode(block);
      cur_layer_nodes.push_back(to_add);
      string hash_str = hash_to_hex_string(to_add->hash, SHA256_DIGEST_LENGTH);
      hash_leaf_map[hash_str] = to_add;
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

  MerkleTree() {};

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

  vector<MerkleNode*> find_siblings(MerkleNode* leaf) {
    vector<MerkleNode*> result;
    MerkleNode* cur_node = leaf;
    while (cur_node->parent != nullptr) {
      if (cur_node->lr == LEFT) {
        result.push_back(cur_node->parent->right);
      } else {
        result.push_back(cur_node->parent->left);
      }
      cur_node = cur_node->parent;
    }
    return result;
  }

  vector<MerkleNode> find_siblings(string hash_str) {
    MerkleNode* cur_node;
    auto it = hash_leaf_map.find(hash_str);
    if (it != hash_leaf_map.end()) {
      cur_node = it->second;
    } else {
      return {};
    }

    vector<MerkleNode> result;
    while (cur_node != nullptr && cur_node->parent != nullptr) {
      MerkleNode tmp;
      if (cur_node->lr == LEFT) {
        tmp = (*cur_node->parent->right);
      } else {
        tmp = (*cur_node->parent->left);
      }
      result.push_back(tmp);
      cur_node = cur_node->parent;
    }
    return result;
  }

  bool verify(unsigned char* data, int data_len) {
    Blocks blocks_to_verify(data, data_len);
    for (auto block : blocks_to_verify.blocks()) {
      if (! verify(block)) {
        return false;
      }
    }
    return true;
  }

  bool verify(Block& block) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(block.data, BLOCK_SIZE, hash);
    return verify(hash_to_hex_string(hash, SHA256_DIGEST_LENGTH));
  }

  bool verify(string hash_str) {
    if (hash_str.size() != SHA256_DIGEST_LENGTH * 2) {
      return false;
    }
    if (hash_leaf_map.find(hash_str) == hash_leaf_map.end()) {
      return false;
    }
    MerkleNode* node = hash_leaf_map[hash_str];
    MerkleNode input = (*node);
    auto siblings = find_siblings(node);
    return verify(input, siblings);
  }

  bool verify(MerkleNode cur_node, vector<MerkleNode*>& siblings) {
    for (const auto& sibling : siblings) {
      cur_node = MerkleNode(cur_node, sibling);
    }
    if (memcmp(cur_node.hash, root->hash, SHA256_DIGEST_LENGTH) == 0) {
      return true;
    } else {
      return false;
    }
  }

  bool verify(string hash_str, vector<MerkleNode> &siblings,
              string root_hash) {
    MerkleNode cur_node(hash_str);
    for (const auto &sibling : siblings) {
      cur_node = MerkleNode(cur_node, sibling);
    }
    string calculated = hash_to_hex_string(cur_node.hash, SHA256_DIGEST_LENGTH);
    if (calculated == root_hash) {
      return true;
    } else {
      return false;
    }
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
  cout << "Root hash: ";
  merkle_tree.print_root_hash();

  int block_idx = 0;
  printf("===== Test Block #%d out of %lu blocks =====\n", block_idx + 1,
         blocks.blocks().size());

  auto block_to_verify = blocks.blocks()[block_idx];
  if (merkle_tree.verify(block_to_verify.data, BLOCK_SIZE)) {
    cout << "Yeah! Verified!" << endl;
  }
  if (merkle_tree.verify(block_to_verify)) {
    cout << "Yeah! Verified!" << endl;
  }

  cout << "==== Verify as a client ====" << endl;
  unsigned char client_hash[SHA256_DIGEST_LENGTH];
  SHA256(block_to_verify.data, BLOCK_SIZE, client_hash);
  string hash_str = hash_to_hex_string(client_hash, SHA256_DIGEST_LENGTH);
  string root_hash = merkle_tree.root_hash();
  cout << "hash_str of the block: " << hash_str << endl;
  cout << "root_hash: " << root_hash << endl;
  auto siblings = merkle_tree.find_siblings(hash_str);
  MerkleTree local_tree;
  if (local_tree.verify(hash_str, siblings, root_hash)) {
    cout << "Yeah! Verified!" << endl;
  }

  /*

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

  */

  return 0;
}
