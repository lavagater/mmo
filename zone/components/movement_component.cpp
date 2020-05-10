#include "game_object.h"
#include "zone.h"
#include <math.h>
#include "query.h"
#include "utils.h"
#include "movement_component.h"
#include "transform_component.h"

MovementComponent::MovementComponent()
{
  speed = 1;
}

void MovementComponent::MoveTo(Eigen::Vector2d pos)
{
  destination = pos;
  updating = true;
  moved_signal(pos);
}

void MovementComponent::Init()
{
  update_connection = game_object->zone->update_signal.Connect(std::bind(&MovementComponent::onUpdate, this, std::placeholders::_1));
}

void MovementComponent::onUpdate(double dt)
{
  if (!updating)
    return;
  if ((destination - GETCOMP(game_object, TransformComponent)->position).squaredNorm() <= speed * speed * dt * dt)
  {
    updating = false;
    //save last position in case of a collision
    last_pos = GETCOMP(game_object, TransformComponent)->position;
    GETCOMP(game_object, TransformComponent)->position = destination;
    arrived_signal();
    return;
  }
  //get the new player position
  Eigen::Vector2d new_pos = GETCOMP(game_object, TransformComponent)->position + (destination - GETCOMP(game_object, TransformComponent)->position).normalized() * speed *dt;
  //save last position in case of a collision
  last_pos = GETCOMP(game_object, TransformComponent)->position;
  GETCOMP(game_object, TransformComponent)->position = new_pos;
}