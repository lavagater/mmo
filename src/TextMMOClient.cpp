/*!
  \author
    Wyatt Lavigueure
  \date
    7/23/2017
  \brief
    Client side of the TextMMO.
*/
#include <vector>
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

int main(int argc, char **argv)
{
  if (argc == 1)
  {
    std::cout << "Usage:" << std::endl << argv[0] << " xpos1  ypos1  message1 [number of times to send, default one]" << std::endl;
    return 1;
  }
    //load in config file
  Config config;
  config.Init("TextMMOClient.conf");

  //setup the network stack
  Init();
  SOCKET sock = CreateSocket(IPPROTO_UDP);
  sockaddr_in local;
  std::string ip = static_cast<std::string>(config.properties["ip"]);
  //take any port it gives to me
  CreateAddress(ip.c_str(), 0, &local);
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
  //setup the server address
  sockaddr_in server;
  CreateAddress(ip.c_str(), static_cast<int>(config.properties["port"]), &server);
  //buffer for receiving messages
  char buffer[MAXPACKETSIZE];
  //the address we recieve from
  sockaddr_in from;

  //tell the server I exist and that I want them old messages
  stack.Send(buffer, 1, &server, flags);

  int num = 1;
  if (argc == 5)
  {
    num = atoi(argv[4]);
  }
  int counter = 0;
  //main loop
  while (1)
  {
    //send the message num times
    if (counter >= 100000 && num-- > 0)
    {
      //put info into the buffer
      *reinterpret_cast<float*>(buffer) = atoi(argv[1]);
      *reinterpret_cast<float*>(buffer+sizeof(float)) = atoi(argv[2]);
      strcpy(buffer+sizeof(float)*2, argv[3]);
      //send message to the server
      stack.Send(buffer, sizeof(float) * 2 + strlen(argv[3])+1, &server, flags);
      std::cout << "sent " << num << std::endl;
      counter = 0;
    }
    counter += 1;
    while(1)
    {
      //recieve message
      int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
      if (n > 0)
      {
        //print the text message
        std::cout << "xpos = " << *reinterpret_cast<float*>(buffer) << " ypos = " << *reinterpret_cast<float*>(buffer + sizeof(float));
        std::cout << " age = " << *reinterpret_cast<int*>(buffer + sizeof(float) *2) << std::endl;
        std::cout << buffer + sizeof(float) * 2 + sizeof(int) << std::endl << std::endl;
      }
      else
      {
        break;
      }
    }

    //update stack
    stack.Update();
  }
}