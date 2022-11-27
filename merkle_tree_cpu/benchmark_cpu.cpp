#include <string>
#include <tuple>
#include "../merkle_tree.hpp"
#include "../utils/testdata.hpp"
#include "../utils/timer.hpp"

using namespace std;

string PLATFORM = "CPU";
string CACHE_PATH = "cached_test_data";

int main(int argc, char *argv[]) {
  if (argc < 3) {
    cerr << "Usage: ./benchmark_cpu <data_len> <block_size> [--no-cache]"
         << endl;
    exit(1);
  } else if (argc == 4 && strcmp(argv[3], "--no-cache") == 0) {
    CACHE_PATH = "NO_CACHE";
  }
  string config = "";
  unsigned char* data = nullptr;
  unsigned long long data_len = stoull(argv[1]);
  BLOCK_SIZE = stoi(argv[2]);

  Hasher* hasher = new SHA_256();
  TestData td(data_len, BLOCK_SIZE, PLATFORM, CACHE_PATH);
  tie(config, data, data_len) = td.get_test_data();

  start_timer(config);
  MerkleTree mt(data, data_len, hasher);
  stop_timer();

  cerr << mt.root_hash() << endl; // to stderr
  print_timer_csv();
  return 0;
}
