#ifndef TESTDATA_HPP
#define TESTDATA_HPP

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

class TestData {
 private:
  int random_seed = 42;
  std::string config = "";
  unsigned char* data = nullptr;
  unsigned long long data_len = 0;
  unsigned long long block_size = 0;
  std::string platform = "";
  std::string cache_path = "";
  bool data_loadded = false;

  void generate_test_data();
  bool load_test_data();
  std::tuple<std::string, unsigned char*, unsigned long long> make_test_data();

 public:
  TestData(unsigned long long data_len_, unsigned long long block_size_,
           std::string platform_, std::string cache_path_);
  ~TestData();

  std::tuple<std::string, unsigned char*, unsigned long long> get_test_data();
};

#endif /* TESTDATA_HPP */
