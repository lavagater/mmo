#include <iostream>

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

int main(int argc, char **argv)
{
  //figure out which test machin i am
  if (argc != 2)
  {
    return -1;
  }
  //which ip/port this app is running
  unsigned local = atoi(argv[1]);
  //load in the addresses of the other test machines
  Config config;
  config.Init("stress_test.conf");
  Init();
  std::vector<sockaddr_in> connections;
  //add the addresses
  for (unsigned i = 0; config.properties.find(std::string("ip")+std::to_string(i)) != config.properties.end();++i)
  {
    sockaddr_in addr;
    CreateAddress(((std::string)config.properties[std::string("ip")+std::to_string(i)]).c_str(), 
                  (int)config.properties[std::string("port")+std::to_string(i)], &addr);
    connections.push_back(addr);
  }
  SOCKET sock = CreateSocket(IPPROTO_UDP);
  Bind(sock, &connections[local]);
  SetNonBlocking(sock);
  unsigned bandwidth = 1000;
  NetworkStack stack(sock);
  Channel *channel = new Channel();
  Reliability *reliability = new Reliability();
  Prioritization *priority = new Prioritization(bandwidth);
  Encryption *encryption = new Encryption();
  stack.AddLayer(channel);
  stack.AddLayer(reliability);
  stack.AddLayer(priority);
  stack.AddLayer(encryption);
  BitArray<HEADERSIZE> flags;
  flags.SetBit(ReliableFlag);
  Deinit();
}