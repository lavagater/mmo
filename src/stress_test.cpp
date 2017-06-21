#include <iostream>
#include <list>
#include <utility>

#include "wyatt_sock.h"
#include "meta.h"
#include "config.h"
#include "channel.h"
#include "frame_rate.h"
#include "blowfish.h"
#include "network_stack.h"
#include "bit_array.h"
#include "reliability.h"
#include "prioritization.h"
#include "encryption.h"

//send random stuff to network stack to make sure it does not crash
void FuzzTest(SOCKET sock, sockaddr_in *server)
{
  char buffer[1000];
  while(1)
  {
    int size = rand()%1000;
    for (int i = 0; i < size; ++i)
    {
      buffer[i] = rand();
    }
    Send(sock, buffer, size, server);
  }
}

void StressCleint(NetworkStack &stack, sockaddr_in *server, BitArray<HEADERSIZE> &flags)
{
  //string is the packet sent, while float is the time sent
  std::list<std::pair<std::string, double> > sent_packets;
  char buffer[MAXPACKETSIZE];
  sockaddr_in from;
  FrameRate time;
  int num_messages = 1000;
  do
  {
    if (num_messages > 0)
    {
      //send random message
      std::string message;
      int length = rand() % 1000;
      for (int i = 0; i < length; ++i)
      {
        //value between A and z
        message += 'A' + rand()%('z'-'A');
      }
      int n = stack.Send(message.c_str(), message.size(), server, flags);
      if (n > 0)
      {
        sent_packets.push_front(std::pair<std::string, double>(message, time.GetTotalTime()));
        num_messages -= 1;
      }
    }

    //receive message
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
    if (n > 0)
    {
      //make sure its correct and remove our message from sent_packets
      buffer[n] = 0;
      std::string msg(buffer);
      bool found = false;
      for (auto it = sent_packets.begin(); it != sent_packets.end(); ++it)
      {
        if (it->first == msg)
        {
          found = true;
          sent_packets.erase(it);
          std::cout <<sent_packets.size() << std::endl;
          break;
        }
      }
      if (found == false)
      {
        std::cout << "packet mismatch" << std::endl;
      }
    }
    stack.Update();
  }while(sent_packets.size());
}

void StressServer(NetworkStack &stack, BitArray<HEADERSIZE> &flags)
{
  char buffer[MAXPACKETSIZE];
  sockaddr_in from;
  while(1)
  {
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
    if (n > 0)
    {
      stack.Send(buffer, n, &from, flags);
    }
    stack.Update();
  }
}

int main(int argc, char **argv)
{
  srand(time(0));
  //load in the addresses of the other test machines
  Config config;
  config.Init("stress_test.conf");
  Init();
  SOCKET sock = CreateSocket(IPPROTO_UDP);
  sockaddr_in local;
  sockaddr_in dest;
  CreateAddress(0,static_cast<int>(config.properties["local_port"]),&local);
  if (static_cast<bool>(config.properties["is_server"]) == false)
  {
    CreateAddress(static_cast<std::string>(config.properties["ip"]).c_str(), static_cast<int>(config.properties["port"]),&dest);
  }
  Bind(sock, &local);
  SetNonBlocking(sock);
  NetworkStack stack(sock);
  Channel *channel = new Channel();
  Reliability *reliability = new Reliability();
  Encryption *encryption = new Encryption();
  stack.AddLayer(channel);
  stack.AddLayer(reliability);
  stack.AddLayer(encryption);
  BitArray<HEADERSIZE> flags;
  flags.SetBit(ReliableFlag);
  int test_num = 0;
  if (argc == 2)
  {
    test_num = atoi(argv[1]);
  }
  if (static_cast<bool>(config.properties["is_server"]))
  {
    StressServer(stack, flags);
  }
  else
  {
    switch(test_num)
    {
      case 0:
        StressCleint(stack, &dest, flags);
      break;
      case 1:
        FuzzTest(sock, &dest);
      break;
    }
  }
  Deinit();
}