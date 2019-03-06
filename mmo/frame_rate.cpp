#include "frame_rate.h"

FrameRate::FrameRate()
{
  start = std::chrono::high_resolution_clock::now();
	prev = std::chrono::high_resolution_clock::now();
	last_time = 0;
}
double FrameRate::GetTime()
{
	std::chrono::system_clock::time_point curr = std::chrono::high_resolution_clock::now();
	last_time = std::chrono::duration_cast<std::chrono::nanoseconds>(curr-prev).count() / 1000000000.0;
	prev = curr;
	return last_time;
}
double FrameRate::GetPrevTime()
{
	return last_time;
}
double FrameRate::GetTotalTime()
{
  std::chrono::system_clock::time_point curr = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(curr - start).count() / 1000000000.0;
}