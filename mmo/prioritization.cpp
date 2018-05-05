#include <string.h>

#include "prioritization.h"

Prioritization::Prioritization(double bandwidth)
{
  this->bandwidth = bandwidth;
}

Prioritization::~Prioritization()
{
}
int Prioritization::Send(__attribute__((unused))char* buffer, int bytes, __attribute__((unused))const sockaddr_in* dest, __attribute__((unused))BitArray<HEADERSIZE> &flags)
{
  //bandwidth is above the desired then dont send the packet
  if (stack->GetBandwidth() >= bandwidth)
  {
    return BANDWIDTHLIMIT;
  }
  else
  {
    //the bandwidth is good just send the packet
    return bytes;
  }
}
int Prioritization::Receive(__attribute__((unused))char* buffer, int bytes, __attribute__((unused))sockaddr_in* location, __attribute__((unused))BitArray<HEADERSIZE> &flags)
{
  return bytes;
}
void Prioritization::Update(__attribute__((unused))double dt)
{
}
void Prioritization::RemoveConnection(__attribute__((unused))const sockaddr_in*addr)
{
}