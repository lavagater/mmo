#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

class PlayerControllerComponent : public Component
{
public:
  void Init();
  void onUpdate(double dt);
  void OnPlayerJoined(char *buffer, unsigned n, sockaddr_in *addr);
  void OnCollision(GameObject *other);
  void OnDisconnected(char *buffer, unsigned n, sockaddr_in *addr);
  void OnMove(char *buffer, unsigned n, sockaddr_in *addr);
  void OnShoot(char *buffer, unsigned n, sockaddr_in *addr);
  void SendMoveMessage();
  void SendPlayerInfo(GameObject *reciever);
  void Shoot(Eigen::Vector2d velocity);

  Eigen::Vector2d destination;
  unsigned id;
  sockaddr_in lb_addr;
  //dont send stuff to this address its just used to validate messages came from the correct client
  sockaddr_in client_addr;
  Eigen::Vector2d last_pos;
  Connection update_connection;
  Connection player_move_connection;
  Connection shoot_connection;
  Connection player_disconnected;
  Connection collision_started_connection;
  Connection collision_persisted_connection;
  Connection player_joined_connection;
};