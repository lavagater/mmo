#include "timer.h"
#include <iostream>

std::map<std::string, double> Timer::times;

Timer::Timer(std::string fn) : fn(fn)
{
  t = std::chrono::high_resolution_clock::now();
  if (times.find(fn) == times.end())
  {
    times[fn] = 0;
  }
}
Timer::~Timer()
{
  times[fn] += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t).count();
}
void Timer::Display()
{
  for (auto it = times.begin(); it != times.end(); ++it)
  {
    std::cout << it->first << ": " << it->second << std::endl;
  }
}
