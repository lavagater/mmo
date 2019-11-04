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
  //the life time of the account manager is assumed to be the life time of the program
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
  char buffer[MAXPACKETSIZE];
  //need to figure out which zone the client is in
  load_balancer->query_id += 1;
  load_balancer->query_callbacks[load_balancer->query_id] = std::bind(&InventoryManager::SpellsResponse, this, *addr, std::placeholders::_1, std::placeholders::_2);
  int len = CreateQueryMessage(load_balancer->protocol, load_balancer->query_id, &buffer[0], STRINGIZE(
    main(unsigned id)
    {
      print("Getting Spells for player ", id, "\n");
      vec res = find(22, id);
      print("player has ", Size(res), " Spells\n");
      vec ret = vector();
      int i = 0;
      while(i < Size(res))
      {
        vector_push(ret, getRange(res[i], 0, 25));
        i += 1;
      }
      print("Returning vector with ", Size(ret), " elements\n");
      return ret;
    }
  ), (unsigned)load_balancer->clients[*addr]);
  load_balancer->stack.Send(&buffer[0], len, &load_balancer->spells_database, load_balancer->flags[load_balancer->spells_database]);
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