#include "logger.h"
#include "database_protocol.h"
#include "load_balancer_protocol.h"
#include "query.h"
#include "utils.h"
#include "load_balancer.h"
#include "inventory_manager.h"

void InventoryManager::SetUp(LoadBalancer *load_balancer)
{
  this->load_balancer = load_balancer;
  //the life time of the account manager is the life time of the program
  load_balancer->network_signals.signals[load_balancer->protocol.LookUp("Spells")].Connect(std::bind(&InventoryManager::Spells, this, std::placeholders::_3));
}

void InventoryManager::Spells(sockaddr_in *addr)
{
  LOG("SPELLS!");
  if (load_balancer->clients.find(*addr) == load_balancer->clients.end() || load_balancer->clients[*addr] == -1)
  {
    LOG("Client does not exist");
    return;
  }
  //todo need a way to send new spell stuff to client
}
void InventoryManager::SpellsResponse(sockaddr_in client_addr, char *buffer, unsigned size)
{
  LOG("Got spell response size = " << size);
  char new_buf[MAXPACKETSIZE];
  int n = 0;
  //make the packet
  *reinterpret_cast<MessageType*>(new_buf) = load_balancer->protocol.LookUp("Spells");
  n += sizeof(MessageType);
  //104 is currently how big the items in spell db is
  if (size %104 != 0)
  {
    LOGW("Size is wrong!! size is probably " << 104 + size%104);
  }
  memcpy(&new_buf[0] + n, buffer, size);
  n += size;
  //send back to the client
  load_balancer->stack.Send(&new_buf[0], n, &client_addr, load_balancer->flags[client_addr]);
}