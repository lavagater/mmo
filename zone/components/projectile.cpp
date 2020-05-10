#include "projectile.h"
#include "game_object.h"
#include "zone.h"
#include "transform_component.h"

void ProjectileComponent::Init()
{
  update_connection = game_object->zone->update_signal.Connect(std::bind(&ProjectileComponent::OnUpdate, this, std::placeholders::_1));
}
void ProjectileComponent::OnUpdate(double dt)
{
  //move the object
  GETCOMP(game_object, TransformComponent)->position += velocity * dt;
}