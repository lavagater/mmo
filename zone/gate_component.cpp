#include "gate_component.h"
#include "logger.h"
#include "collider_component.h"
#include "player_controller_component.h"

void GateComponent::Load(std::istream &stream)
{
  stream >> destination.x();
  stream >> destination.y();
  //read the quoted text for the destination zone
  int count = 0;
  while(count < 2 && !stream.eof())
  {
    char c = stream.get();
    if (c == '"')
    {
      count += 1;
    }
    else if (count == 1)
    {
      zone += c;
    }
  }
  //hard coded 16 is how big the zone name can be in the database
  if (count != 2 || zone.length() > 16)
  {
    LOGW("number of quotes = " << count << " zone = " << zone);
  }
  stream >> teleport_on_collision;
  if (teleport_on_collision)
  {
    collision_connection = GETCOMP(game_object, ColliderComponent)->collision_started_signal.Connect(std::bind(&GateComponent::OnCollision, this, std::placeholders::_1));
  }
}
void GateComponent::Write(std::ostream &stream)
{
  stream << destination.x();
  stream << "\n";
  stream << destination.y();
  stream << "\n";
  stream << zone;
  stream << "\n";
  stream << teleport_on_collision;
  stream << "\n";
}
void GateComponent::Init()
{
}
void GateComponent::OnCollision(GameObject *other)
{
  PlayerControllerComponent *player = GETCOMP(other, PlayerControllerComponent);
  if (player)
  {
    player->Teleport(destination, zone);
  }
}