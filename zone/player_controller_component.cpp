#include "player_controller_component.h"
#include "transform_component.h"
#include "game_object.h"
#include "zone.h"
#include <math.h>

void PlayerControllerComponent::Init()
{
    game_object->zone->update_signal.Connect(std::bind(&PlayerControllerComponent::onUpdate, this, std::placeholders::_1));
    game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("PlayerMove")].Connect(std::bind(&PlayerControllerComponent::OnMove, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void PlayerControllerComponent::onUpdate(double dt)
{
  GETCOMP(game_object, TransformComponent)->position += (destination - GETCOMP(game_object, TransformComponent)->position).normalized() * dt;
}

void PlayerControllerComponent::OnMove(char *buffer, unsigned n, sockaddr_in *addr)
{
  if (n < sizeof(MessageType) + sizeof(unsigned) + sizeof(double) + sizeof(double))
  {
    LOG("Player controller update invalid size");
    return;
  }
  (void)addr;
  (void)n;
  char *saved = buffer;
  //set the new destination
  buffer += sizeof(MessageType);
  unsigned id = *reinterpret_cast<unsigned*>(buffer);
  if (id != this->id)
  {
      return;
  }
  LOG("Got message from player " << id);
  buffer += sizeof(unsigned);
  destination.x() = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  destination.y() = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  //tell each player about the new destination
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.y();
  buffer += sizeof(double);
  unsigned size = buffer - saved;
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    LOG("Sending to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    //TODO: maybe forward mesasage can send to multiple places???
    CreateForwardMessage(game_object->zone->protocol, saved, message_size, 0, it->first, msg);
    LOG("new message size = " << message_size << " sending to " << &GETCOMP(it->second, PlayerControllerComponent)->lb_addr);
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr]);
    LOG("sent");
  }
}