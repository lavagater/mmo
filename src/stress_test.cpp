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

void DataBaseTest(NetworkStack &stack, sockaddr_in *server, BitArray<HEADERSIZE> &flags, int test_num)
{
  //find all the elements with my test num from the data base
  char buffer[MAXPACKETSIZE];
  //the message type, find request
  buffer[0] = 3;
  //the row to find
  *reinterpret_cast<unsigned*>(buffer+1) = 5;
  //the value
  *reinterpret_cast<int*>(buffer+1+sizeof(unsigned)) = test_num;
  //send the request
  stack.Send(buffer, 1 + sizeof(unsigned) + sizeof(int), server, flags);
  std::vector<unsigned> ids;
  //wait for the response('s), and get the id's
  while(1)
  {
    int n = stack.Receive(buffer, MAXPACKETSIZE, server);
    if (n > 0)
    {
      //first part of packet is the row
      //the second part is 1 byte for size of data then the data
      unsigned skip = sizeof(unsigned) + 1 + buffer[sizeof(unsigned)+1];
      //then the next byte is the number of is'd found
      //then an array of all the id's
      //if the number of id's is 100 then there is another message comming 
      //add all the id's we got to the list
      for (int i = 0; i < buffer[skip]; ++i)
      {
        ids.push_back(*reinterpret_cast<unsigned*>(buffer + skip + 1 + sizeof(unsigned) * i));
      }
      //if the number of id's is 100 then there is another packet
      if (buffer[skip] != 100)
      {
        //we have all the id's exit loop
        break;
      }
    }
    stack.Update();
  }
  //now use the id's
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
      case 2:
        DataBaseTest(stack, &dest, flags, static_cast<int>(config.properties["database_test_number"]));
      break;
    }
  }
  Deinit();
}