#include <iostream>
#include <string>
#include <tuple>
#include "../merkle_tree.hpp"
#include "../utils/testdata.hpp"
#include "../utils/timer.hpp"

using namespace std;

string PLATFORM = "GPU";
string CACHE_PATH = "cached_test_data";

int main(int argc, char *argv[]) {
  if (argc < 3) {
    cerr << "Usage: ./benchmark_gpu <data_len> <block_size>" << endl;
    exit(1);
  }
  string config = "";
  unsigned char* data = nullptr;
  unsigned long long data_len = stoull(argv[1]);
  BLOCK_SIZE = stoi(argv[2]);

  Hasher* hasher = new SHA_256_GPU();
  TestData td(data_len, BLOCK_SIZE, PLATFORM, CACHE_PATH);
  tie(config, data, data_len) = td.make_test_data();

  start_timer(config);
  MerkleTree mt(data, data_len, hasher);
  stop_timer();

  cerr << mt.root_hash() << endl; // to stderr
  print_timer_csv();
  return 0;
}
