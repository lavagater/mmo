#include "player_controller_component.h"
#include "transform_component.h"
#include "game_object.h"
#include "zone.h"
#include <math.h>

void PlayerControllerComponent::Init()
{
    update_connection = game_object->zone->update_signal.Connect(std::bind(&PlayerControllerComponent::onUpdate, this, std::placeholders::_1));
    player_move_connection = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("PlayerMove")].Connect(std::bind(&PlayerControllerComponent::OnMove, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void PlayerControllerComponent::onUpdate(double dt)
{
  //get the new player position
  Eigen::Vector2d new_pos = GETCOMP(game_object, TransformComponent)->position + (destination - GETCOMP(game_object, TransformComponent)->position).normalized() * dt;
  
  //check for collisions nearby
  int x_coord[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
  int y_coord[] = {1, 1, 1, 0, 0, 0, -1, -1, -1};
  for (int i = 0; i < 9; ++i)
  {
    std::pair<int, int> terrain_index = std::make_pair(int(new_pos.x()) + x_coord[i], int(new_pos.y()) + y_coord[i]);
    if (game_object->zone->terrain.find(terrain_index) != game_object->zone->terrain.end())
    {
      for (auto it = game_object->zone->terrain[terrain_index].begin(); it != game_object->zone->terrain[terrain_index].end(); ++it)
      {
        if ((new_pos-GETCOMP(*it, TransformComponent)->position).norm() < 1)
        {
          //stop moving
          destination = GETCOMP(game_object, TransformComponent)->position;
          //send message with new destination
          SendMoveMessage();
          return;
        }
      }
    }
  }

  GETCOMP(game_object, TransformComponent)->position = new_pos;
}

void PlayerControllerComponent::OnMove(char *buffer, unsigned n, sockaddr_in *addr)
{
  if (n < sizeof(MessageType) + sizeof(unsigned) + sizeof(double) + sizeof(double))
  {
    LOG("Player controller update invalid size");
    return;
  }
  (void)addr;
  (void)n;
  //set the new destination
  buffer += sizeof(MessageType);
  unsigned id = *reinterpret_cast<unsigned*>(buffer);
  if (id != this->id)
  {
      return;
  }
  buffer += sizeof(unsigned);
  destination.x() = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  destination.y() = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  LOG("Got move message from player " << id);
  SendMoveMessage();
}

void PlayerControllerComponent::SendMoveMessage()
{
  char saved[256];
  char *buffer = &saved[0];
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("PlayerMove");
  buffer += sizeof(MessageType);
  *reinterpret_cast<unsigned*>(buffer) = id;
  buffer += sizeof(unsigned);
  *reinterpret_cast<double*>(buffer) = destination.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = destination.y();
  buffer += sizeof(double);
  //tell each player about the new destination
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.y();
  buffer += sizeof(double);
  unsigned size = buffer - &saved[0];
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    LOG("Sending to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    //TODO: maybe forward mesasage can send to multiple places???
    CreateForwardMessage(game_object->zone->protocol, saved, message_size, 0, it->first, msg);
    LOG("new message size = " << message_size << " sending to " << &GETCOMP(it->second, PlayerControllerComponent)->lb_addr);
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr]);
    LOG("sent");
  }
}