

#include "channel.h"


int Channel::Send(__attribute__((unused))char* buffer, int bytes, __attribute__((unused))sockaddr_in* dest)
{
  return bytes;
}

int Channel::Receive(__attribute__((unused))char* buffer, int bytes, __attribute__((unused))sockaddr_in* location)
{
  return bytes;
}

void Channel::Update(__attribute__((unused))double dt)
{

}