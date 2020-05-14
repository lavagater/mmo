#include "terrain_component.h"
#include "transform_component.h"
#include "player_controller_component.h"
#include "game_object.h"
#include "zone.h"
#include "dispatcher.h"

void TerrainComponent::Load(std::istream &stream)
{
  stream >> terrain_id;
}

void TerrainComponent::Write(std::ostream &stream)
{
  stream << terrain_id;
}

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
  //dispatch it so that it comes after the login message, this is pretty much a hack
  game_object->zone->dispatcher.Dispatch([this, new_player](double)
  {
    char buffer[100];
    Stream stream(buffer, 100);
    stream << game_object->zone->protocol.LookUp("Terrain") << terrain_id;
    game_object->zone->SendTo(stream, GETCOMP(new_player, PlayerControllerComponent)->id);
  }, 0.1);
}