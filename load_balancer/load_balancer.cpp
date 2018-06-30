/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  The entry point for the load balancer executable

*/
/*****************************************************************************/
#include <math.h>
#include <iostream>

#include "meta.h"
#include "config.h"
#include "channel.h"
#include "network_stack.h"
#include "blowfish.h"
#include "network_stack.h"
#include "bit_array.h"
#include "reliability.h"
#include "prioritization.h"
#include "encryption.h"
#include "protocol.h"
#include "logger.h"

int main()
{
  //load in config file
  Config config;
  config.Init("resources/load_balancer.conf");
  LOG("config loaded" << std::endl);
  //setup the network stack
  Init();
  SOCKET sock = CreateSocket(IPPROTO_UDP);
  sockaddr_in local;
  CreateAddress(0,static_cast<int>(config.properties["port"]),&local);
  Bind(sock, &local);
  SetNonBlocking(sock);
  NetworkStack stack(sock);
  Channel *channel = new Channel();
  Reliability *reliability = new Reliability();
  Encryption *encryption = new Encryption();
  stack.AddLayer(channel);
  stack.AddLayer(reliability);
  stack.AddLayer(encryption);
  //the flags for sending messages
  BitArray<HEADERSIZE> flags;
  flags.SetBit(ReliableFlag);
  //buffer for receiving messages
  char buffer[MAXPACKETSIZE];
  //the address we recieve from
  sockaddr_in from;
  LOG("entering while loop" << std::endl);
  //main loop
  while(true)
  {
    //check for messages
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
    //make sure the message is big enough 1 byte for message type 2 unsigned's
    if (n > 0)
    {
      LOG("Recieved message of length " << n);
      //handle message
      switch (buffer[0])
      {
        case EncryptionKey:
        {
        //get the length of key
        short length = *reinterpret_cast<short*>(buffer+1);
        unsigned *data = reinterpret_cast<unsigned*>(buffer + 1 + sizeof(short));
        ((Encryption*)(stack.layers[2]))->blowfish[from] = BlowFish(data, length);
        flags.SetBit(EncryptFlag);
        }
        break;
        default:
        {
          buffer[n] = 0;
          LOG("sending back: " << buffer << std::endl);
          stack.Send(buffer, n, &from, flags);
        }
        break;
      }
    }
    stack.Update();
  }
  return 420;
}