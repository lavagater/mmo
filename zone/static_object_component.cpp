#include "static_object_component.h"
#include "transform_component.h"
#include "player_controller_component.h"
#include "game_object.h"
#include "zone.h"

void StaticObjectComponent::Init()
{
  //player_joined_connection = game_object->zone->player_joined_signal.Connect(std::bind(&StaticObjectComponent::OnPlayerJoined, this, std::placeholders::_1));
  player_joined_connection = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("UpdatePosition")].Connect(std::bind(&StaticObjectComponent::OnPositionUpdateRequest, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void StaticObjectComponent::OnPositionUpdateRequest(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)addr;
  if (n < sizeof(MessageType) + sizeof(unsigned) + sizeof(sockaddr_in))
  {
    LOG("Position Update request");
    return;
  }
  sockaddr_in client_addr = *reinterpret_cast<sockaddr_in*>(buffer+n-sizeof(sockaddr_in));
  unsigned player_id = *reinterpret_cast<unsigned*>(buffer + sizeof(MessageType));
  if (game_object->zone->players.find(player_id) == game_object->zone->players.end())
  {
    return;
  }
  GameObject *player = game_object->zone->players[player_id];
  if (GETCOMP(player, PlayerControllerComponent)->client_addr == client_addr)
  {
    //send the position update, TODO: the varification could be done just once and a signal could be fired which sends the position updates
    SendPositionUpdate(player);
  }
}

void StaticObjectComponent::SendPositionUpdate(GameObject *player)
{
  LOG("Sending the new player the objects position " << GETCOMP(game_object, TransformComponent)->position.x() << " " << GETCOMP(game_object, TransformComponent)->position.y());
  LOG("id = " << game_object->id << " type = " << game_object->type);
  char msg[256];
  char *buffer = &msg[0];
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("UpdatePosition");
  buffer += sizeof(MessageType);
  *reinterpret_cast<unsigned*>(buffer) = game_object->id;
  buffer += sizeof(unsigned);
  *reinterpret_cast<unsigned*>(buffer) = game_object->type;
  buffer += sizeof(unsigned);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.y();
  buffer += sizeof(double);

  //send the message to the new player
  unsigned size = buffer - &msg[0];
  CreateForwardMessage(game_object->zone->protocol, msg, size, 0, GETCOMP(player, PlayerControllerComponent)->id, msg);
  game_object->zone->stack.Send(msg, size, &GETCOMP(player, PlayerControllerComponent)->lb_addr, game_object->zone->flags[GETCOMP(player, PlayerControllerComponent)->lb_addr]);
}

void StaticObjectComponent::OnPlayerJoined(GameObject *new_player)
{
  SendPositionUpdate(new_player);
}