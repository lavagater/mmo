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

int main()
{
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

  //tell the server i exist and that i want them old messages
  stack.Send(buffer, 1, &server, flags);

  //send message, only send one message because its blocing
  float x;
  float y;
  std::cout << "Enter a x position : ";
  std::cin >> x;
  std::cout << "Enter a y position : ";
  std::cin >> y;
  std::cout << "Enter message :" << std::endl;
  std::string str;
  std::cin >> str;
  //put info into the buffer
  *reinterpret_cast<float*>(buffer) = x;
  *reinterpret_cast<float*>(buffer+sizeof(float)) = y;
  memcpy(buffer+sizeof(float)*2, str.c_str(), str.length()+1);
  //send message to the server
  stack.Send(buffer, sizeof(float) * 2 + str.length()+1, &server, flags);
  //main loop
  while (1)
  {
    while(1)
    {
      //recieve message
      int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
      if (n > 0)
      {
        //print the text message
        std::cout << buffer + sizeof(float) * 2 + sizeof(int) << std::endl;
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