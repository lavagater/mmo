

#include "channel.h"


int Channel::Send(__attribute__((unused))char* buffer, int bytes, __attribute__((unused))const sockaddr_in* dest, __attribute__((unused))BitArray<HEADERSIZE> &flags)
{
  return bytes;
}

int Channel::Receive(char* buffer, int bytes, __attribute__((unused))sockaddr_in* location, BitArray<HEADERSIZE> &flags)
{
  //check if this is a ping/pong message
  if (flags[MessageTypeFlag])
  {
    //make sure there is actaully space
    if (bytes >= 1)
    {
      if (*buffer == Ping)
      {
        //send a pong message

        return 0;
      }
      else if (*buffer == Pong)
      {
        //extract new ping

        return 0;
      }
    }
    else
    {
      stack->last_error = MALEFORMEDPACKET;
      return -1;
    }
  }
  return bytes;
}

void Channel::Update(__attribute__((unused))double dt)
{

}