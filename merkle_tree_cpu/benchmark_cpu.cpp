#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <tuple>
#include "../merkle_tree.hpp"

using namespace std;
using namespace std::chrono;

class TestData {
 private:
  int random_seed = 42;
  string config = "";
  unsigned char* data = nullptr;
  unsigned long long data_len = 0;
  unsigned long long block_size = 0;

 public:
  TestData(unsigned long long data_len_, unsigned long long block_size_)
    : data_len(data_len_), block_size(block_size_) {
      data = new unsigned char[data_len];
      if (data == nullptr) {
        cerr << "Error allocating memory of size " << data_len
             << " bytes!" << endl;
        exit(1);
      }
      config = to_string(data_len) + "," + to_string(block_size);
  }

  ~TestData() {
    delete data;
  }

  tuple<string, unsigned char*, unsigned long long> make_test_data() {
    default_random_engine rng(random_seed);
    uniform_int_distribution<int> rng_dist(0, 255);

    for (auto i = 0; i < data_len; i++) {
      data[i] = (unsigned char)(rng_dist(rng));
    }

    return {config, data, data_len};
  }

  tuple<string, unsigned char*, unsigned long long> get_test_data() {
    assert(data != nullptr);
    return {config, data, data_len};
  }
};

auto start = high_resolution_clock::now();
auto elapsed = start - start;
string timer_name = "";
void start_timer(string name) {
  timer_name = name;
  start = high_resolution_clock::now();
}

void stop_timer() {
  auto end = high_resolution_clock::now();
  elapsed = end - start;
}

void print_timer() {
  cout << left << setw(16) << timer_name << ": "
       << right << setw(12)
       << duration_cast<std::chrono::milliseconds>(elapsed).count()
       << " ms" << endl;
}

int main() {
  Hasher* hasher = new SHA_256();
  TestData td(55688877, 1024);
  auto [config, data, data_len] = td.make_test_data();

  start_timer(config);
  MerkleTree mt(data, data_len, hasher);
  stop_timer();

  mt.print_root_hash();
  print_timer();
  return 0;
}