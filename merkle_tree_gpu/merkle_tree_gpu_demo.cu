#include <fstream>
#include <iostream>
#include <unistd.h>
#include "../merkle_tree.hpp"

using namespace std;

void print_node(MerkleNode* node) {
  cout << hash_to_hex_string(node->hash, node->digest_len) << endl;
}

int main(int argc, char *argv[]) {
  // Hasher can be SHA_256 or MD_5 at the moment.
  Hasher* hasher = new SHA_256_GPU();

  BLOCK_SIZE = 1024;
  unsigned char* data;
  int data_len = 0;
  if (argc == 1) {
    // no input file; use dummy data for demo.
    cerr << "Usage: ./merkle_tree_gpu_demo <BLOCK_SIZE> <filename>" << endl;
    cerr << "For demo, create data filled with '9527' with " << BLOCK_SIZE * 4
         << " bytes." << endl;
    data = (unsigned char *)malloc(BLOCK_SIZE * 4 * sizeof(unsigned char));
    data_len = BLOCK_SIZE * 4;
    memset(data + BLOCK_SIZE * 0, 9, BLOCK_SIZE);
    memset(data + BLOCK_SIZE * 1, 5, BLOCK_SIZE);
    memset(data + BLOCK_SIZE * 2, 2, BLOCK_SIZE);
    memset(data + BLOCK_SIZE * 3, 7, BLOCK_SIZE);
  } else if (argc == 3) {
    BLOCK_SIZE = atoi(argv[1]);
    // input filepath provided
    ifstream is;
    is.open(argv[2], ios::binary | ios::ate);
    if (! is.good()) {
      cerr << "File not found at: " << argv[2] << endl;
      exit(2);
    }
    // show file info and read into a buffer
    data_len = is.tellg();
    cout << "path = " << argv[2] << endl;
    cout << "filesize = " << data_len << endl;

    data = (unsigned char *)malloc(data_len * sizeof(unsigned char));
    is.clear();
    is.seekg(0, std::ios::beg);
    is.read((char*)data, data_len);
  } else {
    cerr << "Usage: ./merkle_tree_gpu_demo <BLOCK_SIZE> <filename>" << endl;
    cerr << "Or ./merkle_tree_gpu_demo to demo with dummy data." << endl;
    exit(1);
  }

  // make Blocks from data for further demo
  Blocks blocks(data, data_len);

  // make a MerkleTree from data
  // MerkleTree merkle_tree(data, data_len, hasher);
  unsigned short ACCEL_MASK = ACCEL_CREATION | ACCEL_REDUCTION | ACCEL_LINK | ACCEL_HASHMAP;
  MerkleTree merkle_tree(data, data_len, hasher, ACCEL_MASK);
  cout << "===== Read all at once. =====" << endl;
  cout << "BLOCK_SIZE = " << BLOCK_SIZE << endl;
  merkle_tree.print();
  cout << "Root hash: ";
  merkle_tree.print_root_hash();

  // cout << "=========== P/L/R pointers testing zone =============" << endl;

  // MerkleNode* root = merkle_tree.root;
  // cout << "Root" << endl;
  // print_node(root);
  // cout << "Left child" << endl;
  // print_node(root->left);
  // cout << "Right child" << endl;
  // print_node(root->right);
  // cout << "Right child's parent!" << endl;
  // print_node(root->right->parent);
  // cout << "Right child's right child!" << endl;
  // print_node(root->right->right);
  // cout << "Right child's right child's parent!" << endl;
  // print_node(root->right->right->parent);

  // cout << "=========== P/L/R ends ==============================" << endl;

  int block_idx = 0;
  auto block_to_verify = blocks.blocks()[0];
  printf("===== Test Block #%d out of %lu blocks =====\n", block_idx + 1,
         blocks.blocks().size());

  // if (merkle_tree.verify(block_to_verify.data, BLOCK_SIZE)) {
  //   cout << "Yeah! Verified!" << endl;
  // }
  if (merkle_tree.verify(block_to_verify)) {
    cout << "Yeah! Verified!" << endl;
  }

  cout << "==== Verify as a client ====" << endl;
  unsigned char* client_hash =
      (unsigned char*)calloc(hasher->hash_length(), sizeof(unsigned char));
  hasher->get_hash(block_to_verify.data, BLOCK_SIZE, client_hash);
  string hash_str = hash_to_hex_string(client_hash, hasher->hash_length());
  string root_hash = merkle_tree.root_hash();
  cout << "hash_str of the block: " << hash_str << endl;
  cout << "root_hash: " << root_hash << endl;
  auto siblings = merkle_tree.find_siblings(hash_str);
  MerkleTree local_tree(hasher);
  if (local_tree.verify(hash_str, siblings, root_hash)) {
    cout << "Yeah! Verified!" << endl;
  }

  /*

  // split input data into two halves; the second half is appended later.
  int num_of_blocks = ceil((double)data_len / BLOCK_SIZE);
  if (num_of_blocks > 1) {
    int first_size = BLOCK_SIZE * (num_of_blocks / 2);
    Blocks old_blocks(data, first_size);
    MerkleTree merkle_tree_to_append(old_blocks);
    cout << "===== Read half first, and append the other half. =====" << endl;
    cout << "=== Merkle Tree of the first half ===" << endl;
    merkle_tree_to_append.print();

    Blocks new_blocks(data + first_size, data_len - first_size);
    merkle_tree_to_append.append(new_blocks);
    cout << "=== Merkle Tree of the first half + the second half ===" << endl;
    merkle_tree_to_append.print();
  }

  */
  
  delete hasher;

  return 0;
}
