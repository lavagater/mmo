#include <string.h>

#include "reliability.h"

int Reliability::Send(char* buffer, int bytes, sockaddr_in* dest, BitArray<HEADERSIZE> &flags)
{
  //fisrt flag is the reliability
  if (flags[0])
  {
    //shift buffer to the right by ack size
    for (int i = bytes+ACKSIZE-1; i >= ACKSIZE; --i)
    {
      buffer[i] = buffer[i-ACKSIZE];
    }
    //get the ack
    unsigned ack = next_ack[*dest];
    //update the next ack
    next_ack[*dest] += 1;
    //put the ack in the packet
    memcpy(buffer, &ack, ACKSIZE);
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
    unsigned index = ack % RESENDSIZE;
    //remove the old data
    delete [] packets[index].packet;
    //only allocate as much memory as we need
    packets[index].packet = new char[bytes + ACKSIZE];
    //copy buffer
    for (int i = 0; i < bytes + ACKSIZE; ++i)
    {
      packets[index].packet[i] = buffer[i];
    }
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
  if (flags[0])
  {
    //check if its just the ack
    if (flags[1])
    {
      //the contents of the buffer is just the ack number
      //make sure the number of bytes is correct
      if (bytes != ACKSIZE)
      {
        return -1;
      }
      //remove the saved message for the ack
      unsigned ack;
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
        return -1;
      }
      //extract the ack
      unsigned ack;
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
        memset(acks, 0, RESENDSIZE);
      }
      if (acks[ack%RESENDSIZE] == ack)
      {
        //we have already seen this message
        //send ack message again
        BitArray<HEADERSIZE> temp;
        temp.SetBit(0);
        temp.SetBit(1);
        stack->Send(reinterpret_cast<char*>(&ack), ACKSIZE, location, temp);
        return 0;
      }
      //save ack
      acks[ack%RESENDSIZE] = ack;
      //send ack message
      BitArray<HEADERSIZE> temp;
      temp.SetBit(0);
      temp.SetBit(1);
      stack->Send(reinterpret_cast<char*>(&ack), ACKSIZE, location, temp);
      return bytes - ACKSIZE;
    }
  }
  else
  {
    //if the mesage is not reliable then we have nothing to do here
    return bytes;
  }
}
void Reliability::Update(double dt)
{
  dt += 1;
  //resend messages if they have expired
}