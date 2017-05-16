/*
  \author
    Wyatt Lavigueure
  \date
    5/13/2017
  \brief
    Reliablity layer of the network stack. If a packet is marked as reliable then it will be resent. If we send RESENDSIZE
    number of messages to a single client before a message is acknowleged it will be removed and no longer resent, If
    that is a problem then increase RESENDSIZE
*/
#ifndef RELI_H
#define RELI_H

#include "network_stack.h"

//the number of acks to save at a time
#define RESENDSIZE 300

//size in bytes of the acknowlegement number, max size is sizeof(unsigned)
#define ACKSIZE 2

/*!
  \brief
    This class saves all the information needed to resend a packet
*/
class PacketSave
{
public:
  /*!
    \brief
      The array of characters representing the packet. This is allocated memory and the packet is copied into this
  */
  char *packet;
  /*!
    \brief
      The flags for the packet
  */
  BitArray<HEADERSIZE> flags;
  /*!
    \brief
      The time the packet was sent
  */
  double time;
};
/*!
  \brief
    Reliability layer. The second to bottom of the network stack.
*/
class Reliability : public NetworkLayer
{
public:
  /*!
    \brief
      If reliable flag is set, Saves the sent message for incase it needs to be resent and adds an ack to the front of the packet
  */
  int Send(char* buffer, int bytes, sockaddr_in* dest, BitArray<HEADERSIZE> &flags);
  /*!
    \brief
      If the packet has the reliability flag set then send back a packet with the ack number to show that we got the packet
      if the packet has the ack flag set then it is just an ack number the message that was saved to be resent with that ack
      is removed and the packet does not go through the rest of the stack 
  */
  int Receive(char* buffer, int bytes, sockaddr_in* location, BitArray<HEADERSIZE> &flags);
  /*!
    \brief
      Checks if a packet should be resent and sends it
  */
  void Update(double dt);
private:
  //these are acknowlegement numbers from client messages, so that we dont look at a message we have already seen
  //the size of teh array is RESENDSIZE
  std::unordered_map<sockaddr_in, unsigned *, SockAddrHash> client_acks;
  //these are acknowlegement numbers that we last sent, so that we can get a new ack for a message to a client
  std::unordered_map<sockaddr_in, unsigned, SockAddrHash> next_ack;
  //these are packets to resend, the size of the array is RESENDSIZE
  std::unordered_map<sockaddr_in, PacketSave*, SockAddrHash> resends;
};

#endif