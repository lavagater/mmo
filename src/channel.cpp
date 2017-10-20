#include <iostream>

#include "channel.h"


int Channel::Send(__attribute__((unused))char* buffer, int bytes, const sockaddr_in* dest, __attribute__((unused))BitArray<HEADERSIZE> &flags)
{
  //create the connection for this address if not already created
  stack->connections.insert(std::make_pair(*dest, ConnectionState()));
  return bytes;
}

int Channel::Receive(char* buffer, int bytes, sockaddr_in* location, BitArray<HEADERSIZE> &flags)
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
        *buffer = Pong;
        stack->Send(buffer, bytes, location, flags, layer_id);
        return 0;
      }
      else if (*buffer == Pong && (unsigned)bytes >= sizeof(double)+1)
      {
        //extract new ping
        double ping = stack->timer.GetTotalTime() - *reinterpret_cast<double*>(buffer+1);
        //rolling average
        stack->connections[*location].ping = stack->connections[*location].ping * 0.9 + ping * 0.1;
        stack->connections[*location].time_since_ping = 0;
        return 0;
      }
    }
    else
    {
      return MALEFORMEDPACKET;
    }
  }
  //create the connection for this address if not already created
  stack->connections.insert(std::make_pair(*location, ConnectionState()));
  return bytes;
}

void Channel::Update(double dt)
{
  std::vector<const sockaddr_in*> to_remove;
  for (auto it = stack->connections.begin(); it != stack->connections.end(); ++it)
  {
    it->second.ping_timer -= dt;
    it->second.time_since_ping += dt;
    if (it->second.time_since_ping > DISCONNECT_TIME)
    {
      //add the connection to be removed
      to_remove.push_back(&it->first);
      std::cout << std::endl << "Connection unresponsive***************" << std::endl;
      abort();
    }
    if (it->second.ping_timer <= 0)
    {
      //send a ping
      char buffer[1+sizeof(double)];
      buffer[0] = Ping;
      *reinterpret_cast<double*>(buffer+1) = stack->timer.GetTotalTime();
      BitArray<HEADERSIZE> flags;
      flags.SetBit(MessageTypeFlag);
      stack->Send(buffer, 1+sizeof(double), &it->first, flags, layer_id);
      //reset timer
      it->second.ping_timer = TIME_BETWEEN_PINGS;
    }
  }
  //remove the connections
  for (unsigned i = 0; i < to_remove.size(); ++i)
  {
    stack->RemoveConnection(to_remove[i]);
  }
}
void Channel::RemoveConnection(__attribute__((unused))const sockaddr_in*addr)
{
  //nothing
}