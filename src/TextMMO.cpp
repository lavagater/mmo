/*!
  \author
    Wyatt Lavigueure
  \date
    7/23/2017
  \brief
    A proram to test the database, this program will take messages with 2D position and a string of text and save it to a database
    when a new client is connected the saved messages are sent to each client
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
#include "protocol.h"

int main()
{
    //load in config file
  Config config;
  config.Init("TextMMO.conf");

  //setup the network stack
  Init();
  SOCKET sock = CreateSocket(IPPROTO_UDP);
  sockaddr_in local;
  CreateAddress(0, static_cast<int>(config.properties["port"]),&local);
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

  //create the addres for the database
  sockaddr_in db_addr;
  std::string ip = static_cast<std::string>(config.properties["db_ip"]);
  CreateAddress(ip.c_str(), static_cast<int>(config.properties["db_port"]), &db_addr);

  //main loop
  while (1)
  {
    //using the black magic number
    int n = stack.Receive(buffer, 666, &from);
    if (n > 0)
    {
      if (from == db_addr)
      {
        //mesage from the bd
      }
      //got a beautyful message
      else
      {
        //this is a new message
        std::cout << buffer + sizeof(float) * 2 << std::endl;
        //shift the string over by one int
        for (unsigned i = n-1; i >= sizeof(float)*2; --i)
        {
          buffer[i+sizeof(int)] = buffer[i];
        }
        *reinterpret_cast<int*>(buffer + sizeof(float)*2) = 0;
        //send the message to every connection except the db, and the person who ent it
        for (auto it = stack.connections.begin(); it != stack.connections.end(); ++it)
        {
          if (it->first == db_addr || it->first == from)
          {
            continue;
          }
          stack.Send(buffer, n+sizeof(int), &it->first, flags);
        }
        //put the message into the database
        //char buf[MAXPACKETSIZE];
        //set message
        //buf[0] = 1;

      }
    }
  }
  stack.Update();
}