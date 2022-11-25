#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <random>
#include <tuple>

void start_timer(std::string name);
void stop_timer();
void print_timer();
void print_timer_csv();

#endif /* TIMER_HPP */
