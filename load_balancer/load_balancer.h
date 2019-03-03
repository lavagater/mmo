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


class LoadBalancer
{
public:
  LoadBalancer(Config &config);
  void run();
  //network messages
  void EncryptionKey(char *buffer, unsigned n, sockaddr_in *addr);
  void Relay(char *buffer, unsigned n, sockaddr_in *addr);
  void CreateAccount(char *buffer, unsigned n, sockaddr_in *addr);
  void Login(char *buffer, unsigned n, sockaddr_in *addr);
  void BadLogin(char *buffer, unsigned n, sockaddr_in *addr);
  void ChangePassword(char *buffer, unsigned n, sockaddr_in *addr);
  void QueryResponse(char *buffer, unsigned n, sockaddr_in *addr);
  void ForwardResponse(char *buffer, unsigned n, sockaddr_in *addr);
  void SendLoginMessage(sockaddr_in addr, char *data, unsigned size);
private:
  Config &config;
  SOCKET sock;
  NetworkStack stack;
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
  sockaddr_in account_database;
  AsymetricEncryption encryptor;
  ProtocolLoader protocol;
  NetworkSignals network_signals;
};

#endif