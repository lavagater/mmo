#include "game_object.h"
#include "network_stack.h"
#include "game_object.h"
#include "zone.h"
#include "database_protocol.h"
#include "load_balancer_protocol.h"
#include "query.h"
#include "utils.h"
#include "player_controller_component.h"
#include <sstream>

GameObject::~GameObject()
{
  destroy_signal(this);
  for (auto it = components.begin(); it != components.end(); ++it)
  {
    if (it->second)
    {
        delete it->second;
    }
  }
  components.clear();
}

void GameObject::SendDeleteMessage()
{
  char buffer[MAXPACKETSIZE];
  
  *reinterpret_cast<MessageType*>(&buffer[0]) = zone->protocol.LookUp("DeleteObject");
  *reinterpret_cast<unsigned*>(&buffer[sizeof(MessageType)]) = id;
  unsigned size = sizeof(MessageType)+sizeof(unsigned);
  for (auto it = zone->players.begin(); it != zone->players.end(); ++it)
  {
    //if im deleting a player dont send that player the delete message
    if (it->second == this)
    {
      continue;
    }
    char msg[500];
    unsigned message_size = size;
    CreateForwardMessage(zone->protocol, buffer, message_size, it->first, msg);
    zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr]);
  }
}