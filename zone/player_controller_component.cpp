#include "player_controller_component.h"
#include "transform_component.h"
#include "game_object.h"
#include "zone.h"
#include <math.h>
#include "database_protocol.h"
#include "load_balancer_protocol.h"
#include "query.h"
#include "utils.h"

void PlayerControllerComponent::Init()
{
  update_connection = game_object->zone->update_signal.Connect(std::bind(&PlayerControllerComponent::onUpdate, this, std::placeholders::_1));
  player_disconnected = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("Disconnected")].Connect(std::bind(&PlayerControllerComponent::OnDisconnected, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  player_move_connection = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("PlayerMove")].Connect(std::bind(&PlayerControllerComponent::OnMove, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void PlayerControllerComponent::OnDisconnected(char *buffer, unsigned n, sockaddr_in *addr)
{
  LOG("Player Disconnected");
  (void)(n);
  (void)(addr);
  //set the new destination
  buffer += sizeof(MessageType);
  sockaddr_in d_addr = *reinterpret_cast<sockaddr_in*>(buffer);
  if (client_addr == d_addr)
  {
    //send a query to the database setting my position
    char query[MAXPACKETSIZE];
    int len = CreateQueryMessage(game_object->zone->protocol, -1, query, STRINGIZE(
    main(unsigned id, double x_pos, double y_pos)
    {
      vector res = find(0, id);
      print("player ", res[0], "account ", id, " x pos = ", x_pos, " y pos = ", y_pos, "\n");
      if (Size(res) == 0)
      {
        print("player not found... account ", id);
        return;
      }
      set(res[0], 2, x_pos);
      set(res[0], 3, y_pos);
      return;
    }
    ), id, GETCOMP(game_object, TransformComponent)->position.x(), GETCOMP(game_object, TransformComponent)->position.y());
    //send query to the players dataabse
    game_object->zone->stack.Send(query, len, &game_object->zone->players_database, game_object->zone->flags[game_object->zone->players_database]);
    
    //send a message to the other players that the player was removed
    *reinterpret_cast<MessageType*>(&query[0]) = game_object->zone->protocol.LookUp("DeleteObject");
    *reinterpret_cast<unsigned*>(&query[sizeof(MessageType)]) = id;
    unsigned size = sizeof(MessageType) + sizeof(id);
    for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
    {
      //dont send to the player that just disconnected
      if (it->second == game_object)
      {
        continue;
      }
      char msg[500];
      unsigned message_size = size;
      CreateForwardMessage(game_object->zone->protocol, query, message_size, 0, it->first, msg);
      game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr]);
    }

    //dispatch that this object should be removed
    game_object->zone->dispatcher.Dispatch(std::bind(&Zone::RemoveGameObject, game_object->zone, game_object));
    //remove the player from the player map
    game_object->zone->players.erase(game_object->zone->players.find(id));
  }
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
  //make sure the sender is the player moving
  if (!(*reinterpret_cast<sockaddr_in*>(buffer+n-sizeof(sockaddr_in)) == client_addr))
  {
    return;
  }
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