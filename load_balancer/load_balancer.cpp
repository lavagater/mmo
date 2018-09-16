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
#include "load_balancer_protocol.h"
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
  std::unordered_map<sockaddr_in, BitArray<HEADERSIZE>, SockAddrHash> flags;
  //buffer for receiving messages
  char buffer[MAXPACKETSIZE];
  //the address we recieve from
  sockaddr_in from;
  AsymetricEncryption encryptor;
  LOG("entering while loop" << std::endl);
  //main loop
  while(true)
  {
    //check for messages
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
    flags[from].SetBit(ReliableFlag);
    if (n >= message_type_size)
    {
      LOG("Recieved message of length " << n);
      Protocol type;
      //make sure the type is zeroed out because, the protocol size 
      //might be larger than the type size
      memset(&type, 0, sizeof(Protocol));
      memcpy(&type, buffer, message_type_size);
      //handle message
      switch (type)
      {
        case EncryptionKey:
        {
          char key[MAXPACKETSIZE];
          short length = ReadEncryptionMessage(buffer, n, key, encryptor);
          //if the message was malformed it returns -1 length then we ignore this meddage
          if (length == -1)
          {
            break;
          }
          ((Encryption*)(stack.layers[2]))->blowfish[from] = BlowFish((unsigned int *)key, length*sizeof(unsigned int));
          flags[from].SetBit(EncryptFlag);
          //send back a message saying that i got the key
          stack.Send(buffer, message_type_size, &from, flags[from]);
        }
        break;
        default:
        {
          buffer[n] = 0;
          LOG("sending back: " << buffer << std::endl);
          int send_err = stack.Send(buffer, n, &from, flags[from]);
          if (send_err < 0)
          {
            LOGW("Send error code = " << send_err);
          }
        }
        break;
      }
    }
    else if (n != EBLOCK && n != 0)
    {
      LOGW("recv Error code " << n);
    }
    stack.Update();
  }
  return 420;
}