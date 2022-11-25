#include "testdata.hpp"

using namespace std;
namespace fs = filesystem;

void TestData::generate_test_data() {
  cerr << "Generating test data and storing it to a cache file." << endl;
  default_random_engine rng(random_seed);
  uniform_int_distribution<int> rng_dist(0, 255);

  for (auto i = 0; i < data_len; i++) {
    data[i] = (unsigned char)(rng_dist(rng));
  }
  string test_data_path = cache_path;
  fs::path p{test_data_path};
  fs::create_directory(p);
  p /= to_string(data_len) + ".dat";
  ofstream os;
  os.open(p, ios::binary);
  os.write((char *)data, data_len);
}

bool TestData::load_test_data() {
  string test_data_path = cache_path + "/" + to_string(data_len) + ".dat";
  fs::path p{test_data_path};
  if (!fs::exists(p)) {
    cerr << "Cache not found at: " << fs::absolute(p) << endl;
    return false;
  }
  ifstream is;
  is.open(p, ios::binary);
  is.read((char *)data, data_len);
  return true;
}

TestData::TestData(unsigned long long data_len_, unsigned long long block_size_,
                   string platform_, string cache_path_)
    : data_len(data_len_), block_size(block_size_), platform(platform_),
      cache_path(cache_path_) {
  data = new unsigned char[data_len];
  if (data == nullptr) {
    cerr << "Error allocating memory of size " << data_len << " bytes!" << endl;
    exit(1);
  }
  config = platform + "," + to_string(data_len) + "," + to_string(block_size);
}

TestData::~TestData() { delete data; }

tuple<string, unsigned char *, unsigned long long> TestData::make_test_data() {
  assert(data != nullptr);
  if (!load_test_data()) {
    generate_test_data();
  }
  return {config, data, data_len};
}

tuple<string, unsigned char *, unsigned long long> TestData::get_test_data() {
  assert(data != nullptr);
  return {config, data, data_len};
}
