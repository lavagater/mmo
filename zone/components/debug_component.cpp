#include "debug_component.h"
#include "game_object.h"
#include "collider_component.h"
#include "transform_component.h"
#include "zone.h"
#include "player_controller_component.h"
#include "logger.h"
#include <functional>

void DebugComponent::Init()
{
  update_connection = game_object->zone->update_signal.Connect(std::bind(&DebugComponent::OnUpdate, this, std::placeholders::_1));
  timer = send_rate;
}
void DebugComponent::SetRate(float rate)
{
  send_rate = rate;
  timer = send_rate;
}
void DebugComponent::OnUpdate(double dt)
{
  timer -= dt;
  if (timer < 0)
  {
    SendDebugInfo();
    timer = send_rate;
  }
}

void DebugComponent::SendDebugInfo()
{
  char saved[256];
  char *buffer = &saved[0];
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("Debug");
  buffer += sizeof(MessageType);
  *reinterpret_cast<unsigned*>(buffer) = game_object->id;
  buffer += sizeof(unsigned);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.y();
  buffer += sizeof(double);
  unsigned size = buffer - &saved[0];
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    LOG("Sending debug update to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    CreateForwardMessage(game_object->zone->protocol, saved, message_size, it->first, msg);
    //make the message non reliable
    BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr];
    flags.SetBit(ReliableFlag, false);
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, flags);
  }
}