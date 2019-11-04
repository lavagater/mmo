
/*!
  \author
    Wyatt Lavigueure
  \date
    3/06/2019
  \brief
    Does all the inventory related work for the load balancer, getting players spells and items from the database.
*/
#ifndef INVENTORY_MANAGER_H
#define INVENTORY_MANAGER_H

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
class InventoryManager
{
public:
  void SetUp(LoadBalancer *load_balancer);
  void Spells(sockaddr_in *addr);
  void SpellsResponse(sockaddr_in client_addr, char *buffer, unsigned size);
private:
  LoadBalancer *load_balancer;
};

#endif