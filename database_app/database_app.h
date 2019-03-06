#ifndef DATABASE_APP_H
#define DATABASE_APP_H

#include "query.h"
#include "database.h"
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
#include "logger.h"
#include "asymetric_encryption.h"
#include "network_signals.h"
#include "dispatcher.h"

#include <memory>

class DatabaseApp
{
public:
  DatabaseApp(Config &conf, std::string dbfile);
  void OnRecieve(std::shared_ptr<char> data, unsigned size, sockaddr_in addr);
  void run();
  void QueryCall(char *buffer, int n, sockaddr_in *addr);

  Config &config;
  Database db;
  Query query;
  SOCKET sock;
  NetworkStack stack;
  Dispatcher dispatcher;
  std::unordered_map<sockaddr_in, BitArray<HEADERSIZE>, SockAddrHash> flags;
  char buffer[MAXPACKETSIZE];
  sockaddr_in from;
  AsymetricEncryption encryptor;
  ProtocolLoader protocol;
  NetworkSignals network_signals;
};

#endif