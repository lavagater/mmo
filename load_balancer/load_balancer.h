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
  LoadBalancer();
  void run();
  //network messages
  void EncryptionKey(char *buffer, unsigned n, sockaddr_in *addr);
  void Relay(char *buffer, unsigned n, sockaddr_in *addr);
private:
  Config config;
  SOCKET sock;
  NetworkStack stack;
  std::unordered_map<sockaddr_in, BitArray<HEADERSIZE>, SockAddrHash> flags;
  char buffer[MAXPACKETSIZE];
  sockaddr_in from;
  AsymetricEncryption encryptor;
  ProtocolLoader protocol;
  NetworkSignals network_signals;
};

#endif