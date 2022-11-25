#include "timer.hpp"

using namespace std;
using namespace std::chrono;

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
       << duration_cast<milliseconds>(elapsed).count()
       << " ms" << endl;
}

void print_timer_csv() {
  cout << timer_name << ","
       << duration_cast<milliseconds>(elapsed).count()
       << endl; 
}
