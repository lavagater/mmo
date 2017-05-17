

#include "channel.h"


int Channel::Send(__attribute__((unused))char* buffer, int bytes, __attribute__((unused))const sockaddr_in* dest, __attribute__((unused))BitArray<HEADERSIZE> &flags)
{
  return bytes;
}

int Channel::Receive(__attribute__((unused))char* buffer, int bytes, __attribute__((unused))sockaddr_in* location, __attribute__((unused))BitArray<HEADERSIZE> &flags)
{
  return bytes;
}

void Channel::Update(__attribute__((unused))double dt)
{

}