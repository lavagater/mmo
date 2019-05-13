#include "logger.h"
#include "channel.h"
#include "string"


int Channel::Send(char* buffer, int bytes, const sockaddr_in* dest, __attribute__((unused))BitArray<HEADERSIZE> &flags)
{
  //create the connection for this address if not already created
  if (stack->connections.find(*dest) == stack->connections.end())
  {
    stack->connections.insert(std::make_pair(*dest, ConnectionState()));
    LOG("send new connection, id = " << stack->connections[*dest].connection_id);
  }
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
        LOGN("Sending pong");
        stack->Send(buffer, bytes, location, flags, layer_id);
        return 0;
      }
      else if (*buffer == Pong && (unsigned)bytes >= sizeof(double)+1)
      {
        //extract new ping
        double ping = stack->timer.GetTotalTime() - *reinterpret_cast<double*>(buffer+1);
        //rolling average
        stack->connections[*location].ping = ping;
        stack->connections[*location].time_since_ping = 0;
        return 0;
      }
    }
    else
    {
      LOGW("MALEFORMEDPACKET" << std::endl);
      return MALEFORMEDPACKET;
    }
  }
  //create the connection for this address if not already created
  if (stack->connections.find(*location) == stack->connections.end())
  {
    std::unordered_map<unsigned, int> ports;
    stack->connections.insert(std::make_pair(*location, ConnectionState()));
    if (ports[location->sin_port] > 0)
    {
      LOGW("Same port used again!! " << location->sin_port);
    }
    ports[location->sin_port] += 1;
    LOG("recv new connection, id = " << stack->connections[*location].connection_id);
  }
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
    }
    if (it->second.ping_timer <= 0)
    {
      //send a ping
      char buffer[1+sizeof(double)];
      buffer[0] = Ping;
      *reinterpret_cast<double*>(buffer+1) = stack->timer.GetTotalTime();
      BitArray<HEADERSIZE> flags;
      flags.SetBit(MessageTypeFlag);
      LOGN("Sending Ping");
      stack->Send(buffer, 1+sizeof(double), &it->first, flags, layer_id);
      //reset timer
      it->second.ping_timer = TIME_BETWEEN_PINGS;
    }
  }
  //remove the connections
  for (unsigned i = 0; i < to_remove.size(); ++i)
  {
    LOG(std::endl << "Connection removed, start id = " << stack->connections[*to_remove[i]].connection_id << std::endl);
    stack->RemoveConnection(to_remove[i]);
    LOG(std::endl << "Connection removed, finish" << std::endl);
  }
}
void Channel::RemoveConnection(__attribute__((unused))const sockaddr_in*addr)
{
  //nothing
}