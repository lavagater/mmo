#include "player_controller_component.h"
#include "transform_component.h"
#include "game_object.h"
#include "zone.h"
#include <math.h>
#include "database_protocol.h"
#include "load_balancer_protocol.h"
#include "query.h"
#include "utils.h"
#include "collider_component.h"
#include "bounce_component.h"
#include "static_object_component.h"
#include "terrain_component.h"
#include "gate_component.h"
#include "movement_component.h"
#include "interactive_component.h"
#include "entity_component.h"

void PlayerControllerComponent::Init()
{
  //when the gameobject moves call the function to tell the client
  move_connection = GETCOMP(game_object, MovementComponent)->moved_signal.Connect(std::bind(&PlayerControllerComponent::SendMoveMessage, this));
  player_disconnected = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("Disconnected")].Connect(std::bind(&PlayerControllerComponent::OnDisconnected, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  player_move_connection = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("PlayerMove")].Connect(std::bind(&PlayerControllerComponent::OnMove, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  shoot_connection = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("Shoot")].Connect(std::bind(&PlayerControllerComponent::OnShoot, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  collision_started_connection = GETCOMP(game_object, ColliderComponent)->collision_started_signal.Connect(std::bind(&PlayerControllerComponent::OnCollision, this, std::placeholders::_1));
  collision_persisted_connection = GETCOMP(game_object, ColliderComponent)->collision_persisted_signal.Connect(std::bind(&PlayerControllerComponent::OnCollision, this, std::placeholders::_1));
  position_update_connection = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("UpdatePosition")].Connect(std::bind(&PlayerControllerComponent::OnPositionUpdate, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  player_joined_connection = game_object->zone->player_joined_signal.Connect(std::bind(&PlayerControllerComponent::OnPlayerJoined, this,  std::placeholders::_1));
  teleport_connection = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("Teleport")].Connect(std::bind(&PlayerControllerComponent::OnTeleport, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  spell_connection = game_object->zone->network_signals.signals[game_object->zone->protocol.LookUp("CastSpell")].Connect(std::bind(&PlayerControllerComponent::OnSpell, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void PlayerControllerComponent::Teleport(Eigen::Vector2d destination, std::string zone_name)
{
  char query[MAXPACKETSIZE];
  int len = CreateQueryMessage(game_object->zone->protocol, -1, query, STRINGIZE(
  main(unsigned id, double x_pos, double y_pos, string zone)
  {
    vector res = find(0, id);
    if (Size(res) == 0)
    {
      print("player not found... account ", id);
      return;
    }
    print("player id ", id, " going to zone ", zone);
    set(res[0], 1, zone);
    set(res[0], 2, x_pos);
    set(res[0], 3, y_pos);
    return;
  }
  ), id, destination.x(), destination.y(), zone_name);
  game_object->zone->stack.Send(query, len, &game_object->zone->players_database, game_object->zone->flags[game_object->zone->players_database]);

  //send a message to the other players that the player was removed from this zone
  game_object->SendDeleteMessage();
  //dispatch that this object should be removed
  game_object->zone->dispatcher.Dispatch(std::bind(&Zone::RemoveGameObject, game_object->zone, game_object));
  //remove the player from the player map
  game_object->zone->players.erase(game_object->zone->players.find(id));

  //TODO Move this to its own zone change function
  //send load balancer a message saying this client
  int size = 0;
  *reinterpret_cast<MessageType*>(&query[size]) = game_object->zone->protocol.LookUp("ZoneChange");
  size += sizeof(MessageType);
  *reinterpret_cast<unsigned*>(&query[size]) = id;
  size += sizeof(unsigned);
  memcpy(&query[size], zone_name.data(), zone_name.length()+1);
  //plus one for null terminator
  size += zone_name.length()+1;
  game_object->zone->stack.Send(query, size, &lb_addr, game_object->zone->flags[lb_addr]);
}

void PlayerControllerComponent::OnTeleport(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)(addr);
  if (n < sizeof(MessageType) + sizeof(unsigned) + sizeof(unsigned))
  {
    LOGW("teleport message wrong size n = " << n);
    return;
  }
  buffer += sizeof(MessageType);
  unsigned player_id = *reinterpret_cast<unsigned*>(buffer);
  buffer += sizeof(unsigned);
  unsigned portal_id = *reinterpret_cast<unsigned*>(buffer);
  if (id != player_id)
  {
    return;
  }
  //TODO retrieve the destination from the portal object
  Eigen::Vector2d new_pos = GETCOMP(game_object->zone->object_by_id[portal_id], GateComponent)->destination;
  std::string zone_name = GETCOMP(game_object->zone->object_by_id[portal_id], GateComponent)->zone;

  GETCOMP(game_object, InteractiveComponent)->Interact(game_object->zone->object_by_id[portal_id], std::bind(&PlayerControllerComponent::Teleport, this, new_pos, zone_name), 1);
}

void PlayerControllerComponent::SendPlayerInfo(GameObject *reciever)
{
  LOG("Sending the new player player info for player " << game_object->id);
  char msg[256];
  char *buffer = &msg[0];
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("PlayerJoined");
  buffer += sizeof(MessageType);
  *reinterpret_cast<unsigned*>(buffer) = game_object->id;
  buffer += sizeof(unsigned);
  *reinterpret_cast<unsigned*>(buffer) = id;
  buffer += sizeof(unsigned);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, TransformComponent)->position.y();
  buffer += sizeof(double);

  //send the message to the new player
  unsigned size = buffer - &msg[0];
  CreateForwardMessage(game_object->zone->protocol, msg, size, GETCOMP(reciever, PlayerControllerComponent)->id, msg);
  //this message will be reliable(default) because its the only time when the players id and zone id are sent
  int len = game_object->zone->stack.Send(msg, size, &GETCOMP(reciever, PlayerControllerComponent)->lb_addr, game_object->zone->flags[GETCOMP(reciever, PlayerControllerComponent)->lb_addr]);
  LOG("Sent " << len << " bytes");
}

void PlayerControllerComponent::OnPlayerJoined(GameObject *new_player)
{
  //dont send myself my info
  if (new_player != game_object)
  {
    GETCOMP(new_player, PlayerControllerComponent)->SendPlayerInfo(game_object);
  }
}

void PlayerControllerComponent::OnPositionUpdate(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)addr;
  if (n < sizeof(MessageType) + sizeof(unsigned) + sizeof(sockaddr_in))
  {
    LOG("Player joined malformed message");
    return;
  }
  sockaddr_in client_addr = *reinterpret_cast<sockaddr_in*>(buffer+n-sizeof(sockaddr_in));
  unsigned player_id = *reinterpret_cast<unsigned*>(buffer + sizeof(MessageType));
  if (game_object->zone->players.find(player_id) == game_object->zone->players.end())
  {
    return;
  }
  GameObject *player = game_object->zone->players[player_id];
  if (GETCOMP(player, PlayerControllerComponent)->client_addr == client_addr)
  {
    //send the new player my info
    SendPlayerInfo(player);
  }
}

void PlayerControllerComponent::Shoot(Eigen::Vector2d velocity)
{
  //create a bouncing object and send it in a random direction
  GameObject *bullet = game_object->zone->CreateGameObject();
  bullet->type = 2;
  TransformComponent *trans = ADDCOMP(bullet, TransformComponent);
  ADDCOMP(bullet, ColliderComponent);
  GETCOMP(bullet, ColliderComponent)->shape = new Circle();
  ((Circle*)GETCOMP(bullet, ColliderComponent)->shape)->radius = 0.25;
  GETCOMP(bullet, ColliderComponent)->collision_group = 1;
  GETCOMP(bullet, ColliderComponent)->shape->object = bullet;
  BounceComponent *bounce = ADDCOMP(bullet, BounceComponent);
  bounce->velocity = velocity;
  trans->position = GETCOMP(game_object, TransformComponent)->position + bounce->velocity.normalized();
  StaticObjectComponent *stat = ADDCOMP(bullet, StaticObjectComponent);
  //tell all the players about my new object
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    stat->SendPositionUpdate(it->second);
  }
  bounce->SendMoveMessage();
}

void PlayerControllerComponent::OnDisconnected(char *buffer, unsigned n, sockaddr_in *addr)
{
  LOG("Player Disconnected");
  (void)(n);
  (void)(addr);
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
    game_object->SendDeleteMessage();

    //dispatch that this object should be removed
    game_object->zone->dispatcher.Dispatch(std::bind(&Zone::RemoveGameObject, game_object->zone, game_object));
    //remove the player from the player map
    game_object->zone->players.erase(game_object->zone->players.find(id));
  }
}

void PlayerControllerComponent::OnCollision(GameObject *other)
{
  TerrainComponent *terrain = GETCOMP(other, TerrainComponent);
  if (terrain)
  {
    MovementComponent *move = GETCOMP(game_object, MovementComponent);
    //stop moving
    move->MoveTo(move->last_pos);
    GETCOMP(game_object, TransformComponent)->position = move->last_pos;
  }
}

void PlayerControllerComponent::OnShoot(char *buffer, unsigned n, sockaddr_in *addr)
{
  if (n < sizeof(MessageType) + sizeof(unsigned) + sizeof(double) + sizeof(double))
  {
    LOG("Player controller shoot invalid size");
    return;
  }
  (void)addr;
  (void)n;
  //make sure the sender is the player shooting
  if (!(*reinterpret_cast<sockaddr_in*>(buffer+n-sizeof(sockaddr_in)) == client_addr))
  {
    return;
  }
  buffer += sizeof(MessageType);
  unsigned id = *reinterpret_cast<unsigned*>(buffer);
  if (id != this->id)
  {
      return;
  }
  Eigen::Vector2d vel;
  buffer += sizeof(unsigned);
  vel.x() = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  vel.y() = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  LOG("Got shoot message from player " << id);
  Shoot(vel.normalized() * 10);
}

void PlayerControllerComponent::OnSpell(char *buffer, unsigned n, sockaddr_in *addr)
{
  if (n < sizeof(MessageType) + sizeof(int) + sizeof(int))
  {
    LOG("Player controller cast spell invalid size");
    return;
  }
  (void)addr;
  //make sure the sender is the player casting the spell
  if ((*reinterpret_cast<sockaddr_in*>(buffer+n-sizeof(sockaddr_in)) == client_addr) == false)
  {
    return;
  }
  buffer += sizeof(MessageType);
  int target_id = *reinterpret_cast<int*>(buffer);
  buffer += sizeof(int);
  if (game_object->zone->object_by_id.find(target_id) == game_object->zone->object_by_id.end())
  {
    LOG("Target does not exist target_id = " << target_id);
    return;
  }
  //make sure target is an entity
  if (GETCOMP(game_object->zone->object_by_id[target_id], EntityComponent) == 0)
  {
    LOG("Target is not an entity target_id = " << target_id);
    return;
  }

  //todo at somepoint we will use this to determine which of the players spells to use
  int which_spell = *reinterpret_cast<int*>(buffer);

  LOG("Got spell message from player " << game_object->name << " spell = " << which_spell);
  //cast the spell
  EntityComponent *entity = GETCOMP(game_object, EntityComponent);
  if (entity)
  {
    entity->UseSpell(which_spell, game_object->zone->object_by_id[target_id]);
  }
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
  Eigen::Vector2d destination;
  destination.x() = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  destination.y() = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  LOG("Got move message from player " << id);
  GETCOMP(game_object, MovementComponent)->MoveTo(destination);
}

void PlayerControllerComponent::SendMoveMessage()
{
  char saved[256];
  char *buffer = &saved[0];
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("PlayerMove");
  buffer += sizeof(MessageType);
  *reinterpret_cast<unsigned*>(buffer) = id;
  buffer += sizeof(unsigned);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, MovementComponent)->destination.x();
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = GETCOMP(game_object, MovementComponent)->destination.y();
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
    CreateForwardMessage(game_object->zone->protocol, saved, message_size, it->first, msg);
    LOG("new message size = " << message_size << " sending to " << &GETCOMP(it->second, PlayerControllerComponent)->lb_addr);
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr]);
    LOG("sent");
  }
}