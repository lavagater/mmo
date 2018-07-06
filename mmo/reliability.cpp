#include <string.h>

#include "reliability.h"
#include "logger.h"

Reliability::~Reliability()
{
  for (auto it = resends.begin(); it != resends.end(); ++it)
  {
    if (it->second)
    {
      for (unsigned i = 0; i < RESENDSIZE; ++i)
      {
        //delete any packets that wernt acked
        delete [] it->second[i].packet;
      }
    }
    delete [] it->second;
  }
  for (auto it = client_acks.begin(); it != client_acks.end(); ++it)
  {
    delete [] it->second;
  }
}

int Reliability::Send(char* buffer, int bytes, const sockaddr_in* dest, BitArray<HEADERSIZE> &flags)
{
  if (flags[ReliableFlag])
  {
    //shift buffer to the right by ack size
    for (int i = bytes+ACKSIZE-1; i >= ACKSIZE; --i)
    {
      buffer[i] = buffer[i-ACKSIZE];
    }
    //get the ack
    last_ack = next_ack[*dest];
    //update the next ack
    next_ack[*dest] += 1;
    //put the ack in the packet
    memcpy(buffer, &last_ack, ACKSIZE);
    //save the packet to be resent
    PacketSave *packets = resends[*dest];
    //if this is the first time we need to allocate the memory for packets
    if (!packets)
    {
      packets = new PacketSave[RESENDSIZE];
      resends[*dest] = packets;
      //set data to zero
      memset(packets, 0, RESENDSIZE * sizeof(PacketSave));
    }
    unsigned index = last_ack % RESENDSIZE;
    if (packets[index].packet)
    {
      //sent to many reliable messages
      LOG("ran out of acks message not sent" << std::endl);
      return RELIABLELIMIT;
    }
    //remove the old data
    delete [] packets[index].packet;
    //only allocate as much memory as we need
    packets[index].packet = new char[bytes + ACKSIZE];
    //copy buffer
    for (int i = 0; i < bytes + ACKSIZE; ++i)
    {
      packets[index].packet[i] = buffer[i];
    }
    //save the number of bytes to send
    packets[index].bytes = bytes + ACKSIZE;
    //save the flags
    packets[index].flags = flags;
    //save the time sent
    packets[index].time = stack->timer.GetTotalTime();
    return bytes + ACKSIZE;
  }
  else
  {
    return bytes;
  }
}
int Reliability::Receive(char* buffer, int bytes, sockaddr_in* location, BitArray<HEADERSIZE> &flags)
{
  //check if this message is reliable
  if (flags[ReliableFlag])
  {
    //check if its just the ack
    if (flags[AckNumberFlag])
    {
      //the contents of the buffer is just the ack number
      //make sure the number of bytes is correct
      if (bytes != ACKSIZE)
      {
        LOGW("MALEFORMEDPACKET" << std::endl);
        return MALEFORMEDPACKET;
      }
      //remove the saved message for the ack
      unsigned ack = 0;
      memcpy(&ack, buffer, ACKSIZE);
      if (resends[*location])
      {
        delete [] resends[*location][ack%RESENDSIZE].packet;
        //Mark that this packet no longer needs resending
        resends[*location][ack%RESENDSIZE].packet = 0;
      }
      return 0;
    }
    else
    {
      //check that the message is no to small, screw you hackers
      if (bytes < ACKSIZE)
      {
        LOGW("MALEFORMEDPACKET" << std::endl);
        return MALEFORMEDPACKET;
      }
      //extract the ack
      unsigned ack = 0;
      memcpy(&ack, buffer, ACKSIZE);
      for (int i = 0; i < bytes-ACKSIZE; ++i)
      {
        buffer[i] = buffer[i+ACKSIZE];
      }
      //check if we have already recieved this message
      unsigned *acks = client_acks[*location];
      if (!acks)
      {
        acks = new unsigned[RESENDSIZE];
        client_acks[*location] = acks;
        memset(acks, RESENDSIZE+1, RESENDSIZE*sizeof(unsigned));
      }
      if (acks[ack%RESENDSIZE] == ack)
      {
        //we have already seen this message
        //send ack message again
        BitArray<HEADERSIZE> temp;
        temp.SetBit(0);
        temp.SetBit(1);
        stack->Send(reinterpret_cast<char*>(&ack), ACKSIZE, location, temp, layer_id-1);
        return 0;
      }
      //save ack
      acks[ack%RESENDSIZE] = ack;
      //send ack message
      BitArray<HEADERSIZE> temp;
      temp.SetBit(ReliableFlag);
      temp.SetBit(AckNumberFlag);
      stack->Send(reinterpret_cast<char*>(&ack), ACKSIZE, location, temp, layer_id-1);
      return bytes - ACKSIZE;
    }
  }
  else
  {
    //if the mesage is not reliable then we have nothing to do here
    return bytes;
  }
}
void Reliability::Update(__attribute__((unused))double dt)
{
  double cur_time = stack->timer.GetTotalTime();
  //resend messages if they have expired
  for (auto it = resends.begin(); it != resends.end(); ++it)
  {
    if (it->second)
    {
      for (unsigned i = 0; i < RESENDSIZE; ++i)
      {
        //check if there is a reliable message
        if (it->second[i].packet)
        {
          //get the time to send message, a fraction of the ping time
          float resend_time = stack->connections[it->first].ping * RESENDFRACTION;
          //check if the time the message was sent was more than resend_tme ago
          if (cur_time - it->second[i].time >= resend_time)
          {
            //reset the send time, we added on the resend time because if the client did not get the first one and the second one
            //there is probably an issue with their connection and we probably should not bonbard them with resent packets
            it->second[i].time = cur_time + resend_time;
            //resend the message
            stack->Send(it->second[i].packet, it->second[i].bytes, &it->first, it->second[i].flags, layer_id-1);
          }
        }
      }
    }
  }
}
void Reliability::RemoveConnection(const sockaddr_in *addr)
{
  if (resends[*addr])
  {
    for (unsigned i = 0; i < RESENDSIZE; ++i)
    {
      //delete any packets that wernt acked
      delete [] resends[*addr][i].packet;
    }
    delete [] resends[*addr];
  }
  delete [] client_acks[*addr];
  auto resends_it = resends.find(*addr);
  if (resends_it != resends.end())
  {
    resends.erase(resends_it);
  }
  auto client_acks_it = client_acks.find(*addr);
  if (client_acks_it != client_acks.end())
  {
    client_acks.erase(client_acks_it);
  }
  auto next_ack_it = next_ack.find(*addr);
  if (next_ack_it != next_ack.end())
  {
    next_ack.erase(next_ack_it);
  }
}
int Reliability::get_ack()
{
  return last_ack;
}
bool Reliability::check_sent(int ack, sockaddr_in *dest)
{
  return resends[*dest][ack%RESENDSIZE].packet == 0;
}