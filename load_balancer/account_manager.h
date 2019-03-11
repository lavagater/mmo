/*!
  \author
    Wyatt Lavigueure
  \date
    3/06/2019
  \brief
    Does all the account related work for the load balancer, login/create account/change password.
*/
#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

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

#include <unordered_map>

class LoadBalancer;
class AccountManager
{
public:
  void SetUp(LoadBalancer *load_balancer);
  void CreateAccount(char *buffer, unsigned n, sockaddr_in *addr);
  void Login(char *buffer, unsigned n, sockaddr_in *addr);
  void BadLogin(char *buffer, unsigned n, sockaddr_in *addr);
  void ChangePassword(char *buffer, unsigned n, sockaddr_in *addr);
  void QueryResponse(char *buffer, unsigned n, sockaddr_in *addr);
  void SendLoginMessage(sockaddr_in addr, char *data, unsigned size);
private:
  LoadBalancer *load_balancer;
};

#endif