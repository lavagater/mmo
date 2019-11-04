#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

class PlayerControllerComponent : public Component
{
public:
  void Init();
  /**
   * @brief When a player joins, send the new players information to all the other players.
   */
  void OnPlayerJoined(GameObject *new_player);
  /**
   * @brief When the new player does a position update request, we send him the information of the other players
   */
  void OnPositionUpdate(char *buffer, unsigned n, sockaddr_in *addr);
  void OnCollision(GameObject *other);
  void OnDisconnected(char *buffer, unsigned n, sockaddr_in *addr);
  void OnMove(char *buffer, unsigned n, sockaddr_in *addr);
  void OnShoot(char *buffer, unsigned n, sockaddr_in *addr);
  void OnSpell(char *buffer, unsigned n, sockaddr_in *addr);
  void Teleport(Eigen::Vector2d destination, std::string zone_name);
  void OnTeleport(char *buffer, unsigned n, sockaddr_in *addr);
  void SendMoveMessage();
  void SendPlayerInfo(GameObject *reciever);
  void Shoot(Eigen::Vector2d velocity);

  unsigned id;
  sockaddr_in lb_addr;
  //dont send stuff to this address its just used to validate messages came from the correct client
  sockaddr_in client_addr;
  Connection move_connection;
  Connection player_move_connection;
  Connection shoot_connection;
  Connection player_disconnected;
  Connection collision_started_connection;
  Connection collision_persisted_connection;
  Connection position_update_connection;
  Connection player_joined_connection;
  Connection teleport_connection;
  Connection spell_connection;
};