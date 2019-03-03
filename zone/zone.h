#ifndef ZONE_H
#define ZONE_H

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
#include "load_balancer_protocol.h"
#include "logger.h"
#include "network_signals.h"
#include <unordered_set>

class Zone
{
public:
  Zone(Config &config);
  void run();
  //network messages
  
  /** these are tests thngs foir sample game **/
  void OnZoneTest(char *buffer, unsigned n, sockaddr_in *addr);
  void Login(char *buffer, unsigned n, sockaddr_in *addr);
  void GameUpdate(double dt);
  class Player
  {
    public:
    double x_pos = 0;
    double y_pos = 0;
    double x_dest = 0;
    double y_dest = 0;
    unsigned id;
    sockaddr_in *lb_addr;
  };
  std::unordered_map<unsigned, Player> players;

  /********************************************/

private:
  Config &config;
  SOCKET sock;
  NetworkStack stack;
  std::unordered_map<sockaddr_in, BitArray<HEADERSIZE>, SockAddrHash> flags;
  std::unordered_map<unsigned, std::function<void(char *, unsigned)> > query_callbacks;
  unsigned query_id;
  char buffer[MAXPACKETSIZE];
  sockaddr_in from;
  sockaddr_in account_database;
  //set of the known addresses from start, i.e. databases
  std::unordered_set<sockaddr_in, SockAddrHash> known_addr;
  //set of load balancers
  std::unordered_set<sockaddr_in, SockAddrHash> load_balancers;
  AsymetricEncryption encryptor;
  ProtocolLoader protocol;
  NetworkSignals network_signals;
};

#endif