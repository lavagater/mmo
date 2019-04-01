#include "terrain_component.h"
#include "transform_component.h"
#include "player_controller_component.h"
#include "game_object.h"
#include "zone.h"

void TerrainComponent::Init()
{
  update_connection = game_object->zone->update_signal.Connect(std::bind(&TerrainComponent::onUpdate, this, std::placeholders::_1));
  player_joined_connection = game_object->zone->player_joined_signal.Connect(std::bind(&TerrainComponent::OnPlayerJoined, this, std::placeholders::_1));
  std::pair<int, int> current_pos = std::make_pair((int)GETCOMP(game_object, TransformComponent)->position.x(), (int)GETCOMP(game_object, TransformComponent)->position.y());
  game_object->zone->terrain[current_pos].insert(game_object);
  prev_pos = current_pos;
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
      prev_pos = current_pos;
  }
}

void TerrainComponent::OnPlayerJoined(GameObject *new_player)
{
  (void)new_player;
}