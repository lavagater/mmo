#include "terrain_component.h"
#include "transform_component.h"
#include "player_controller_component.h"
#include "game_object.h"
#include "zone.h"

void TerrainComponent::Init()
{
    game_object->zone->update_signal.Connect(std::bind(&TerrainComponent::onUpdate, this, std::placeholders::_1));
    game_object->zone->player_joined_signal.Connect(std::bind(&TerrainComponent::OnPlayerJoined, this, std::placeholders::_1));
}

void TerrainComponent::onUpdate(double dt)
{
  (void)dt;
  //check if the terrain has moved(why the fuck is terrain moving???)
  std::pair<int, int> current_pos = std::make_pair((int)GETCOMP(game_object, TransformComponent)->position.x(), (int)GETCOMP(game_object, TransformComponent)->position.y());
  if (current_pos != prev_pos)
  {
      //remove prev pos from terrains
      game_object->zone->terrain[prev_pos].erase(game_object->zone->terrain[prev_pos].find(game_object));
      //add the new pos
      game_object->zone->terrain[current_pos].insert(game_object);
  }
}

void TerrainComponent::OnPlayerJoined(GameObject *new_player)
{
  LOG("Sending the new player the terrain position " << prev_pos.first << " " << prev_pos.second);
  char msg[256];
  char *buffer = &msg[0];
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("TerrainPosition");
  buffer += sizeof(MessageType);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.y();
  buffer += sizeof(double);

  //send the message to the new player
  unsigned size = buffer - &msg[0];
  CreateForwardMessage(game_object->zone->protocol, msg, size, 0, GETCOMP(new_player, PlayerControllerComponent)->id, msg);
  game_object->zone->stack.Send(msg, size, &GETCOMP(new_player, PlayerControllerComponent)->lb_addr, game_object->zone->flags[GETCOMP(new_player, PlayerControllerComponent)->lb_addr]);
}