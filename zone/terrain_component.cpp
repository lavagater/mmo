#include "terrain_component.h"
#include "transform_component.h"
#include "player_controller_component.h"
#include "game_object.h"
#include "zone.h"

void TerrainComponent::Init()
{
  update_connection = game_object->zone->update_signal.Connect(std::bind(&TerrainComponent::onUpdate, this, std::placeholders::_1));
  player_joined_connection = game_object->zone->player_joined_signal.Connect(std::bind(&TerrainComponent::OnPlayerJoined, this, std::placeholders::_1));
}

void TerrainComponent::onUpdate(double dt)
{
  (void)dt;
}

void TerrainComponent::OnPlayerJoined(GameObject *new_player)
{
  (void)new_player;
}