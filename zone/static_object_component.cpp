#include "static_object_component.h"
#include "transform_component.h"
#include "player_controller_component.h"
#include "game_object.h"
#include "zone.h"

void StaticObjectComponent::Init()
{
  player_joined_connection = game_object->zone->player_joined_signal.Connect(std::bind(&StaticObjectComponent::OnPlayerJoined, this, std::placeholders::_1));
}

void StaticObjectComponent::OnPlayerJoined(GameObject *new_player)
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
  CreateForwardMessage(game_object->zone->protocol, msg, size, 0, GETCOMP(new_player, PlayerControllerComponent)->id, msg);
  game_object->zone->stack.Send(msg, size, &GETCOMP(new_player, PlayerControllerComponent)->lb_addr, game_object->zone->flags[GETCOMP(new_player, PlayerControllerComponent)->lb_addr]);
}