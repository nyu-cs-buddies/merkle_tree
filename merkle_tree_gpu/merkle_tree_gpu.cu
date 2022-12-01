#include "../merkle_tree.hpp"
#include "../cuda_hash_lib/md5.cuh"
#include "../cuda_hash_lib/sha256.cuh"
#include <cassert>
#include <cmath>
#include <openssl/sha.h>
#include <openssl/md5.h>
using namespace std;

int BLOCK_SIZE = 1024;

// hash algorithms - GPU versions
// SHA256
SHA_256_GPU::SHA_256_GPU() {
  digest_size = SHA256_DIGEST_LENGTH;
}

void SHA_256_GPU::get_hash(unsigned char* data,
                            int data_len,
                            unsigned char* hash) {
  SHA256(data, data_len, hash);
}

void SHA_256_GPU::get_hash(unsigned char* din,
                            int block_size,
                            unsigned char* dout,
                            int num_of_blocks) {
  int threadsPerBlock = 1024;
  int numOfBlocks = ceil(double(num_of_blocks)/ threadsPerBlock);
  dim3 dimGrid(numOfBlocks);
  dim3 dimBlock(threadsPerBlock);
  kernel_sha256_hash<<<dimGrid, dimBlock>>>(din, block_size,
                                            dout, num_of_blocks);
}

// MD5
MD_5_GPU::MD_5_GPU() {
  digest_size = MD5_DIGEST_LENGTH;
}

void MD_5_GPU::get_hash(unsigned char* data,
                        int data_len,
                        unsigned char* hash) {
  MD5(data, data_len, hash);
}

void MD_5_GPU::get_hash(unsigned char* din,
                        int block_size,
                        unsigned char* dout,
                        int num_of_blocks) {
  int threadsPerBlock = 1024;
  int numOfBlocks = ceil(double(num_of_blocks)/ threadsPerBlock);
  dim3 dimGrid(numOfBlocks);
  dim3 dimBlock(threadsPerBlock);
  kernel_md5_hash<<<dimGrid, dimBlock>>>(din, block_size,
                                         dout, num_of_blocks);
}


string hash_to_hex_string(unsigned char *hash, int size) {
  char temp[3];
  string result = "";
  for (int i = 0; i < size; i++) {
    snprintf(temp, 3, "%02x", hash[i]);
    result += temp;
  }
  return result;
}

void hex_string_to_hash(string hash_str, unsigned char* hash, int size) {
  int hash_str_size = hash_str.size();
  if (hash_str_size / 2 > size) {
    return;
  }
  unsigned char buf;
  for (int i = 0; i < hash_str_size; i += 2) {
    sscanf(hash_str.c_str() + i, "%02hhx", &buf);
    hash[i / 2] = buf;
  }
}

SHA_256::SHA_256() {
  digest_size = SHA256_DIGEST_LENGTH;
}

void SHA_256::get_hash(unsigned char* data,
                        int data_len,
                        unsigned char* hash) {
  SHA256(data, data_len, hash);
}


MD_5::MD_5() {
  digest_size = MD5_DIGEST_LENGTH;
}

void MD_5::get_hash(unsigned char* data,
                    int data_len,
                    unsigned char* hash) {
  MD5(data, data_len, hash);
}


//
// Class Block
//
Block::Block() {
  data = (unsigned char *)calloc(BLOCK_SIZE, sizeof(unsigned char));
}

//
// Class Blocks
//

// Blocks destructor
// TODO(allenpintsung): make this more smart with Rule of Five
Blocks::~Blocks() {
  for (auto &block : _blocks) {
    delete (block.data);
  }
}
vector<Block> const &Blocks::blocks() { return _blocks; }
Blocks::Blocks(unsigned char *data, int data_len) {
  int num_of_blocks = data_len / BLOCK_SIZE;
  int offset = 0;
  for (int i = 0; i < num_of_blocks; i++) {
    Block b;
    memcpy(b.data, data + offset, BLOCK_SIZE);
    offset += BLOCK_SIZE;
    _blocks.push_back(b);
  }
  if (offset < data_len) {
    Block b;
    memcpy(b.data, data + offset, data_len - offset);
    _blocks.push_back(b);
  }
}
void Blocks::add_blocks(Blocks &new_blocks) {
  _blocks.insert(_blocks.end(), new_blocks.blocks().begin(),
                 new_blocks.blocks().end());
}

//
// class MerkleNode
//
MerkleNode::MerkleNode()
    : parent(nullptr), left(nullptr), right(nullptr), lr(NA),
      hash(nullptr), digest_len(0) {}

MerkleNode::MerkleNode(unsigned char* hash_, int digest_len_)
    : parent(nullptr), left(nullptr), right(nullptr), lr(NA),
      hash(nullptr), digest_len(digest_len_) {
  hash = new unsigned char[digest_len];
  memset(hash, 0, digest_len);
  memcpy(hash, hash_, digest_len);
}

// make a MerkleNode with a specific hash_str
MerkleNode::MerkleNode(string hash_str, Hasher* hasher)
    : parent(nullptr), left(nullptr), right(nullptr), lr(NA),
      digest_len(hasher->hash_length()) {
  assert(hash_str.size() == digest_len * 2);
  hash = (unsigned char*)calloc(digest_len, sizeof(unsigned char));
  hex_string_to_hash(hash_str, hash, digest_len);
}

// make a MerkleNode from a block
MerkleNode::MerkleNode(const Block &block, Hasher* hasher)
    : parent(nullptr), left(nullptr), right(nullptr), lr(NA),
      digest_len(hasher->hash_length()) {
  hash = (unsigned char*)calloc(digest_len, sizeof(unsigned char));
  hasher->get_hash(block.data, BLOCK_SIZE, hash);
}

// make a parent MerkleNode from two child MerkleNodes (lhs, rhs)
MerkleNode::MerkleNode(MerkleNode *lhs, MerkleNode *rhs, Hasher* hasher)
    : parent(nullptr), lr(NA), digest_len(hasher->hash_length()) {
  hash = (unsigned char*)calloc(digest_len, sizeof(unsigned char));
  unsigned char* data =
      (unsigned char *)calloc(digest_len * 2, sizeof(unsigned char));
  memcpy(data, lhs->hash, digest_len);
  memcpy(data + digest_len, rhs->hash, digest_len);
  hasher->get_hash(data, digest_len * 2, hash);
  left = lhs;
  right = rhs;
  lhs->parent = this; // connect parent
  rhs->parent = this;
  lhs->lr = LEFT; // indicate left or right child
  rhs->lr = RIGHT;
}

// make a parent MerkleNode from an existing MerkleNode and its siblings,
// with info of left or right indicator.
MerkleNode::MerkleNode(MerkleNode cur_node, MerkleNode *sibling, Hasher* hasher)
    : parent(nullptr), left(nullptr), right(nullptr), lr(NA),
      digest_len(hasher->hash_length()) {
  hash = (unsigned char*)calloc(digest_len, sizeof(unsigned char));
  unsigned char* data =
      (unsigned char *)calloc(digest_len * 2, sizeof(unsigned char));
  if (sibling->lr == LEFT) {
    memcpy(data, sibling->hash, digest_len);
    memcpy(data + digest_len, cur_node.hash, digest_len);
  } else {
    memcpy(data, cur_node.hash, digest_len);
    memcpy(data + digest_len, sibling->hash, digest_len);
  }
  hasher->get_hash(data, digest_len * 2, hash);
}

// make a parent MerkleNode from an existing MerkleNode and its siblings,
// with info of left or right indicator.
MerkleNode::MerkleNode(MerkleNode cur_node, MerkleNode sibling, Hasher* hasher)
    : parent(nullptr), left(nullptr), right(nullptr), lr(NA),
      digest_len(hasher->hash_length()) {
  hash = (unsigned char*)calloc(digest_len, sizeof(unsigned char));
  unsigned char* data =
      (unsigned char *)calloc(digest_len * 2, sizeof(unsigned char));
  if (sibling.lr == LEFT) {
    memcpy(data, sibling.hash, digest_len);
    memcpy(data + digest_len, cur_node.hash, digest_len);
  } else {
    memcpy(data, cur_node.hash, digest_len);
    memcpy(data + digest_len, sibling.hash, digest_len);
  }
  hasher->get_hash(data, digest_len * 2, hash);
}

// print the hash of a MerkleNode in hex string format
void MerkleNode::print_hash() {
  for (int i = 0; i < digest_len; i++) {
    printf("%02x", hash[i]);
  }
  cout << endl;
}

// print the infomation of a MerkleNode
void MerkleNode::print_info() {
  string parent_hash;
  if (parent != nullptr){
    parent_hash = hash_to_hex_string(parent->hash, digest_len);
  } else {
    parent_hash = "";
  }
  cout << "parent hash: " << parent_hash << endl;
  cout << "l or r: " << lr << endl;
  cout <<   hash_to_hex_string(hash, digest_len) << endl;
}

//
// Class MerkleTree
//
void MerkleTree::delete_tree_walker(MerkleNode *cur_node) {
  if (cur_node == nullptr) {
    return;
  }
  delete_tree_walker(cur_node->left);
  delete_tree_walker(cur_node->right);
  delete (cur_node);
}

// produce a MerkleTree from hashes
MerkleNode *
MerkleTree::make_tree_from_hashes(vector<MerkleNode *>& cur_layer_nodes) {
  int cur_layer_nodes_size = cur_layer_nodes.size();
  while (cur_layer_nodes_size > 1) {
    cur_layer_nodes_size = cur_layer_nodes.size();
    int count = 0;
    for (int i = 0; i < cur_layer_nodes_size - 1; i = i + 2) {
      cur_layer_nodes[count] =
          new MerkleNode(cur_layer_nodes[i], cur_layer_nodes[i + 1], hasher);
      count++;
    }
    if (count > 0 && cur_layer_nodes_size % 2 != 0) {
      cur_layer_nodes[count] = cur_layer_nodes[cur_layer_nodes_size - 1];
      cur_layer_nodes.resize(count + 1);
    } else {
      cur_layer_nodes.resize(count);
    }
  }
  assert(cur_layer_nodes[0]->parent == nullptr);
  return cur_layer_nodes[0];
}

// produce a MerkleTree from Blocks and assign the head to root
MerkleNode *MerkleTree::make_tree_from_blocks(Blocks &blocks) {
  if (blocks.blocks().empty()) {
    return nullptr;
  }
  vector<MerkleNode *> cur_layer_nodes;
  for (const auto &block : blocks.blocks()) {
    MerkleNode *to_add = new MerkleNode(block, hasher);
    cur_layer_nodes.push_back(to_add);
    string hash_str = hash_to_hex_string(to_add->hash, hasher->hash_length());
    hashes.push_back(to_add);
    hash_leaf_map[hash_str] = to_add;
  }
  return make_tree_from_hashes(cur_layer_nodes);
}

// helper functions in verification process
bool MerkleTree::verify(MerkleNode cur_node, vector<MerkleNode *> &siblings) {
  for (const auto &sibling : siblings) {
    cur_node = MerkleNode(cur_node, sibling, hasher);
  }
  if (memcmp(cur_node.hash, root->hash, hasher->hash_length()) == 0) {
    return true;
  } else {
    return false;
  }
}

// print a MerkleTree, layer by layer, left to right
void MerkleTree::print() {
  queue<MerkleNode *> q;
  q.push(root);
  int layer = 0;
  while (!q.empty() && q.front() != nullptr) {
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

// return a string contains the root hash of the MerkleTree in hex string format
string MerkleTree::root_hash() {
  return hash_to_hex_string(root->hash, hasher->hash_length());
}

// print the root hash in hex string format
void MerkleTree::print_root_hash() { cout << root_hash() << endl; }

// constructor with only Hasher
MerkleTree::MerkleTree(Hasher* hasher_) : hasher(hasher_) {}

// constructor using Blocks
MerkleTree::MerkleTree(Blocks& blocks_, Hasher* hasher_) : hasher(hasher_) {
  root = make_tree_from_blocks(blocks_);
}

// constructor using data in unsigned char and data_len
MerkleTree::MerkleTree(unsigned char* data, int data_len, Hasher* hasher_)
    : hasher(hasher_) {
  // Blocks blocks(data, data_len);
  // root = make_tree_from_blocks(blocks);

  // Note(allenpthuang): for temporary testing
  // SHA_256_GPU gpu_hasher;
  int num_of_blocks = (data_len % BLOCK_SIZE) ? data_len / BLOCK_SIZE + 1 : data_len / BLOCK_SIZE;
  int in_bytes = num_of_blocks * BLOCK_SIZE;
  int out_bytes = num_of_blocks * hasher->hash_length();

  unsigned char *out = (unsigned char *)calloc(out_bytes, sizeof(unsigned char));
  unsigned char *dout, *din;
  cudaMalloc((void**) &dout, out_bytes);
  cudaMalloc((void**) &din, in_bytes);
  cudaMemset(din, 0, in_bytes);
  cudaMemcpy(din, data, data_len, cudaMemcpyHostToDevice);
  hasher->get_hash(din, BLOCK_SIZE, dout, num_of_blocks);
  // kernel_sha256_hash<<<1, num_of_blocks>>>(din, BLOCK_SIZE, dout, num_of_blocks);
  cudaMemcpy(out, dout, out_bytes, cudaMemcpyDeviceToHost);
  cudaFree(dout); cudaFree(din);

  // out has all hashes
  vector<MerkleNode *> cur_layer_nodes;
  for (int i = 0; i < num_of_blocks; ++i) {
    string hash_str = hash_to_hex_string(out + i * hasher->hash_length(),
                                         hasher->hash_length());
    MerkleNode *to_add = new MerkleNode(hash_str, hasher);
    cur_layer_nodes.push_back(to_add);
    hashes.push_back(to_add);
    hash_leaf_map[hash_str] = to_add;
  }

  root = make_tree_from_hashes(hashes);
}

int next_pow_of_2(int input) {
  int r = 0;
  while (input >>= 1) {
    r++;
  }
  return r;
}

// Further acceleration
MerkleTree::MerkleTree(unsigned char* data, int data_len, Hasher* hasher_,
                       unsigned short accel_mask)
    : hasher(hasher_) {
  if ((accel_mask & NO_ACCEL) == NO_ACCEL) {
    Blocks blocks(data, data_len);
    root = make_tree_from_blocks(blocks);
    return;
  }
  int num_of_blocks = (data_len % BLOCK_SIZE) ? data_len / BLOCK_SIZE + 1 : data_len / BLOCK_SIZE;
  int in_bytes = num_of_blocks * BLOCK_SIZE;
  // TODO(allenpthuang): need to know how to calc out_bytes
  int out_bytes = num_of_blocks * hasher->hash_length() * 2 * 2;
  // int out_bytes = pow(2, next_pow_of_2(num_of_blocks)) * hasher->hash_length() * 2;

  unsigned char *out = (unsigned char *)calloc(out_bytes, sizeof(unsigned char));
  unsigned char *dout, *din;
  cudaMalloc((void**) &dout, out_bytes);
  cudaMalloc((void**) &din, in_bytes);

  if (! (din && dout)) {
    cerr << "Error allocating device memory for din and dout!" << endl;
    exit(1);
  }

  unsigned int *dparents, *dlefts, *drights;
  LeftOrRightSib *dlrs;
  if ((accel_mask & ACCEL_LINK) == ACCEL_LINK) {
    // TODO(allenpthuang): ditto, need to know how to calc.
    arr_size = num_of_blocks * 2 * 2;
    parents = (unsigned int *)calloc(arr_size, sizeof(unsigned int));
    lefts = (unsigned int *)calloc(arr_size, sizeof(unsigned int));
    rights = (unsigned int *)calloc(arr_size, sizeof(unsigned int));
    lrs = (LeftOrRightSib *)calloc(arr_size, sizeof(LeftOrRightSib));
    cudaMalloc((void**) &dparents, arr_size * sizeof(unsigned int));
    cudaMalloc((void**) &dlefts, arr_size * sizeof(unsigned int));
    cudaMalloc((void**) &drights, arr_size * sizeof(unsigned int));
    cudaMalloc((void**) &dlrs, arr_size * sizeof(LeftOrRightSib));
    if (! (dparents && dlefts && drights && dlrs)) {
      cerr << "Error allocating device memory for din and dout!" << endl;
      exit(1);
    }
  }

  cudaMemset(din, 0, in_bytes);
  cudaMemcpy(din, data, data_len, cudaMemcpyHostToDevice);

  unsigned char *dout_left;
  unsigned char *dout_right = dout;

  hasher->get_hash(din, BLOCK_SIZE, dout, num_of_blocks);

  if ((accel_mask & (ACCEL_CREATION | ACCEL_REDUCTION))
        == (ACCEL_CREATION | ACCEL_REDUCTION)) {
    bool attached = false;
    for (auto n = num_of_blocks; n > 0; n /= 2) {
      dout_left = dout_right;
      if (attached) {
        n += 1;
        attached = false;
      }
      dout_right += n * hasher->hash_length();

      int threadsPerBlock = 1024;
      int numOfBlocks = ceil(double(n / 2) / threadsPerBlock);
      dim3 dimGrid(numOfBlocks);
      dim3 dimBlock(threadsPerBlock);
      if ((accel_mask & ACCEL_LINK) == ACCEL_LINK) {
        kernel_sha256_hash_link<<<dimGrid, dimBlock>>>(dout_left,
                                                       hasher->hash_length(),
                                                       dout_right,
                                                       n / 2,
                                                       dout,
                                                       dparents,
                                                       dlefts,
                                                       drights,
                                                       dlrs);
      } else {
        kernel_sha256_hash_cont<<<dimGrid, dimBlock>>>(dout_left,
                                                       hasher->hash_length(),
                                                       dout_right,
                                                       n / 2);
      }
      if (n / 2 > 0 && n % 2 != 0) {
        unsigned char *attach_pos = dout_right + (n / 2) * hasher->hash_length();
        unsigned char *copy_pos = dout_left + (n - 1) * hasher->hash_length();
        cudaMemcpy(attach_pos, copy_pos, hasher->hash_length(),
                  cudaMemcpyDeviceToDevice);
        attached = true;
      }
    }

    cudaMemcpy(out, dout, out_bytes, cudaMemcpyDeviceToHost);
    cudaFree(dout); cudaFree(din);

    if ((accel_mask & ACCEL_LINK) == ACCEL_LINK) {
      cudaMemcpy(parents, dparents,
                 arr_size * sizeof(unsigned long), cudaMemcpyDeviceToHost);
      cudaMemcpy(lefts, dlefts,
                 arr_size * sizeof(unsigned long), cudaMemcpyDeviceToHost);
      cudaMemcpy(rights, drights,
                 arr_size * sizeof(unsigned long), cudaMemcpyDeviceToHost);
      cudaMemcpy(lrs, dlrs,
                 arr_size * sizeof(LeftOrRightSib), cudaMemcpyDeviceToHost);
      cudaFree(dparents);
      cudaFree(dlefts);
      cudaFree(drights);
      cudaFree(dlrs);
    }

    unsigned char *result_out = out + (dout_right - dout) - hasher->hash_length();
    MerkleNode* root_node = new MerkleNode(result_out, hasher->hash_length());
    root = root_node;
    return;
  }

  if ((accel_mask & ACCEL_CREATION) == ACCEL_CREATION) {
    cudaMemcpy(out, dout, out_bytes, cudaMemcpyDeviceToHost);
    cudaFree(dout);
    cudaFree(din);

    // out has all hashes
    vector<MerkleNode *> cur_layer_nodes;
    for (int i = 0; i < num_of_blocks; ++i) {
      string hash_str = hash_to_hex_string(out + i * hasher->hash_length(),
                                           hasher->hash_length());
      MerkleNode *to_add = new MerkleNode(hash_str, hasher);
      cur_layer_nodes.push_back(to_add);
      hashes.push_back(to_add);
      hash_leaf_map[hash_str] = to_add;
    }

    root = make_tree_from_hashes(hashes);
    return;
  }
}

// delete the MerkleTree
void MerkleTree::delete_tree() {
  delete_tree_walker(root);
  root = nullptr;
}

// TODO(allenpthuang): Naive way to append blocks! Should be more efficient.
void MerkleTree::append(Blocks &new_blocks) {
  for (const auto& block : new_blocks.blocks()) {
    MerkleNode* to_add = new MerkleNode(block, hasher);
    hashes.push_back(to_add);
  }
  delete_tree();
  root = make_tree_from_hashes(hashes);
}

void MerkleTree::append(unsigned char* data, int data_len) {
  Blocks blocks_to_append(data, data_len);
  append(blocks_to_append);
}

// return a vector of the pointer to the sibling MerkleNodes along
// the path to the root.
vector<MerkleNode *> MerkleTree::find_siblings(MerkleNode *leaf) {
  vector<MerkleNode *> result;
  MerkleNode *cur_node = leaf;
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

// return a vector of the sibling MerkleNodes along the path to the root.
vector<MerkleNode> MerkleTree::find_siblings(string hash_str) {
  MerkleNode *cur_node;
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

// verify whether a piece of data exists in the MerkleTree
bool MerkleTree::verify(unsigned char *data, int data_len) {
  Blocks blocks_to_verify(data, data_len);
  for (auto block : blocks_to_verify.blocks()) {
    if (!verify(block)) {
      return false;
    }
  }
  return true;
}

// verify whether a block of data exists in the MerkleTree
bool MerkleTree::verify(Block &block) {
  unsigned char* hash =
      (unsigned char*)calloc(hasher->hash_length(), sizeof(unsigned char));
  hasher->get_hash(block.data, BLOCK_SIZE, hash);
  return verify(hash_to_hex_string(hash, hasher->hash_length()));
}

// verify whether a hash_str of some data exists in the MerkleTree
bool MerkleTree::verify(string hash_str) {
  if (hash_str.size() != hasher->hash_length() * 2) {
    return false;
  }
  if (hash_leaf_map.find(hash_str) == hash_leaf_map.end()) {
    return false;
  }
  MerkleNode *node = hash_leaf_map[hash_str];
  MerkleNode input = (*node);
  auto siblings = find_siblings(node);
  return verify(input, siblings);
}


// verify whether a hash_str of some data exists in the MerkleTree,
// using only sibling MerkleNodes and the root hash.
bool MerkleTree::verify(string hash_str, vector<MerkleNode> &siblings,
                        string root_hash) {
  MerkleNode cur_node(hash_str, hasher);
  for (auto &sibling : siblings) {
    sibling.print_hash();
    cur_node = MerkleNode(cur_node, sibling, hasher);
  }
  string calculated = hash_to_hex_string(cur_node.hash, hasher->hash_length());
  cout << "check root hash" << endl;
  if (calculated == root_hash) {
    return true;
  } else {
    return false;
  }
}
