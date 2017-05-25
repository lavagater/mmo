#include <string.h>

#include "prioritization.h"

Prioritization::Prioritization(double bandwidth)
{
  this->bandwidth = bandwidth;
}

Prioritization::~Prioritization()
{
  for (auto it = saved.begin(); it != saved.end(); ++it)
  {
    for (unsigned i = 0; i < it->second.size(); ++i)
    {
      //delete any packets that wernt acked
      delete [] it->second[i].packet;
    }
  }
}
int Prioritization::Send(char* buffer, int bytes, const sockaddr_in* dest, BitArray<HEADERSIZE> &flags)
{
  //bandwidth is above the desired then dont send the packet
  if (stack->GetBandwidth() >= bandwidth)
  {
    //check if we can discard tis message
    if (!flags[ReliableFlag] && flags[NotImportant])
    {
      return 0;
    }
    //save the message to be sent later
    PacketSave packet;
    packet.packet = new char[bytes];
    memcpy(packet.packet, buffer, bytes);
    packet.bytes = bytes;
    packet.flags = flags;
    packet.time = stack->timer.GetTotalTime();
    //save the packet in the vector
    saved[*dest].push_back(packet);
    return 0;
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
  for (auto it = saved.begin(); it != saved.end(); ++it)
  {
    unsigned i;
    for (i = 0; i < it->second.size(); ++i)
    {
      //if the bandwidth is better send it
      if (stack->GetBandwidth() >= bandwidth)
      {
        stack->Send(it->second[i].packet, it->second[i].bytes, &it->first, it->second[i].flags);
        //delete the packet
        delete [] it->second[i].packet;
      }
      else
      {
        break;
      }
    }
    //remove all the sent packets from the vector
    it->second.erase(it->second.begin(), it->second.begin()+i);
  }
}
void Prioritization::RemoveConnection(const sockaddr_in*addr)
{
  for (unsigned i = 0; i < saved[*addr].size(); ++i)
  {
    //delete any packets that wernt acked
    delete [] saved[*addr][i].packet;
  }
  saved.erase(saved.find(*addr));
}