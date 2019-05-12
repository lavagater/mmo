#include "bounce_component.h"
#include "game_object.h"
#include "collider_component.h"
#include "transform_component.h"
#include "zone.h"
#include "player_controller_component.h"
#include "logger.h"
#include <functional>

void BounceComponent::Init()
{
  update_connection = game_object->zone->update_signal.Connect(std::bind(&BounceComponent::OnUpdate, this, std::placeholders::_1));
  collision_connection = GETCOMP(game_object, ColliderComponent)->collision_started_signal.Connect(std::bind(&BounceComponent::OnCollision, this, std::placeholders::_1));
  bounces = 0;
}
void BounceComponent::OnUpdate(double dt)
{
  //move the object
  GETCOMP(game_object, TransformComponent)->position += velocity * dt;
}
void BounceComponent::OnCollision(GameObject *other)
{
  BounceComponent *bounce = GETCOMP(other, BounceComponent);
  if (bounce && 0)
  {
    //bouncing off another bouncy thing
  }
  else
  {
    if (bounces >= 2)
    {
      game_object->SendDeleteMessage();
      //dispatch that this object should be removed
      game_object->zone->dispatcher.Dispatch(std::bind(&Zone::RemoveGameObject, game_object->zone, game_object));
      return;
    }
    bounces += 1;
    //bouncing off a non bouncy thing 
    //try to find a normal ish direction
    Eigen::Vector2d normal = (GETCOMP(game_object, TransformComponent)->position - GETCOMP(other, TransformComponent)->position).normalized();
    //mirror the velocity about the normal
    velocity = 2*normal*-velocity.dot(normal)+velocity;
    //the velocity has changed so tell the clients
    SendMoveMessage();
  }
}

void BounceComponent::SendMoveMessage()
{
  char saved[256];
  char *buffer = &saved[0];
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("Movement");
  buffer += sizeof(MessageType);
  *reinterpret_cast<unsigned*>(buffer) = game_object->id;
  buffer += sizeof(unsigned);
  *reinterpret_cast<double*>(buffer) = velocity.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = velocity.y();
  buffer += sizeof(double);
  //tell each player about the new destination
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.y();
  buffer += sizeof(double);
  unsigned size = buffer - &saved[0];
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    LOG("Sending movement update to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    CreateForwardMessage(game_object->zone->protocol, saved, message_size, 0, it->first, msg);
    LOG("new message size = " << message_size << " sending to " << &GETCOMP(it->second, PlayerControllerComponent)->lb_addr);
    //make the message non reliable
    BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr];
    flags.SetBit(ReliableFlag, false);
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, flags);
    LOG("sent");
  }
}