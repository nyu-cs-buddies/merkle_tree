#include <iostream>
#include "merkle_tree_cpu.hpp"

int main(int argc, char *argv[]) {
  unsigned char* data;
  int data_len = 0;
  if (argc == 1) {
    // no input file; use dummy data for demo.
    cerr << "Usage: ./merkle_tree_demo <filename>" << endl;
    cerr << "For demo, create data filed with '9527' with " << BLOCK_SIZE * 4
         << " bytes." << endl;
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