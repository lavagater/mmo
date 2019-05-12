#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

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
#include "dispatcher.h"
#include "account_manager.h"

#include <memory>

class LoadBalancer
{
public:
  LoadBalancer(Config &config);
  void run();
  //called from dispatcher so no raw memory is used, only shared pointers and copies
  void OnRecieve(std::shared_ptr<char> data, unsigned size, sockaddr_in addr, BitArray<HEADERSIZE> sent_flags);
  //network messages
  void Relay(char *buffer, unsigned n, sockaddr_in *addr);
  void EncryptionKey(char *buffer, unsigned n, sockaddr_in *addr);
  void QueryResponse(char *buffer, unsigned n, sockaddr_in *addr);
  void ForwardResponse(char *buffer, unsigned n, sockaddr_in *addr, BitArray<HEADERSIZE> flags);
  void OnClientDisconnect(const sockaddr_in *addr);
  void RemoveClient(sockaddr_in addr);
  sockaddr_in GetZone(std::string zone_name);
  std::string GetZone(sockaddr_in *addr);
//private:  Let them all have it
  Config &config;
  SOCKET sock;
  NetworkStack stack;
  Dispatcher dispatcher;
  std::unordered_map<sockaddr_in, BitArray<HEADERSIZE>, SockAddrHash> flags;
  std::unordered_map<sockaddr_in, std::string, SockAddrHash> zones;
  std::vector<sockaddr_in> zone_array;
  //TODO: listen for disconnect signal and erase the client
  std::unordered_map<sockaddr_in, unsigned, SockAddrHash> clients;
  std::unordered_map<unsigned, sockaddr_in> clients_by_id;
  std::unordered_map<unsigned, std::function<void(char *, unsigned)> > query_callbacks;
  unsigned query_id;
  char buffer[MAXPACKETSIZE];
  sockaddr_in from;
  sockaddr_in players_database;
  sockaddr_in account_database;
  AsymetricEncryption encryptor;
  ProtocolLoader protocol;
  NetworkSignals network_signals;
  AccountManager account_manager;
};

#endif