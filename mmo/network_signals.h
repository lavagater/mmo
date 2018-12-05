#ifndef NETWORK_SIGNALS_H
#define NETWORK_SIGNALS_H

#include "protocol.h"
#include "signals.h"
#include "wyatt_sock.h"

#include <unordered_map>

//lol i guess a class was not really nessecary but oh well
class NetworkSignals
{
public:
  std::unordered_map<MessageType, Signals<char*, unsigned, sockaddr_in*>> signals;
};

#endif