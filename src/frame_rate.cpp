#include "frame_rate.h"

FrameRate::FrameRate()
{
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &prev);
	last_time = 0;
}
double FrameRate::GetTime()
{
	timespec curr;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &curr);
	last_time = curr.tv_sec - prev.tv_sec + (curr.tv_nsec - prev.tv_nsec) / 1000000000.0;
	prev = curr;
	return last_time;
}
double FrameRate::GetPrevTime()
{
	return last_time;
}