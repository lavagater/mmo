#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

class GateComponent : public Component
{
public:
  void Init();
  void OnTeleport(char *buffer, unsigned n, sockaddr_in *addr);
  void OnCollision(GameObject *other);
  void OnPlayerMoved();

  Eigen::Vector2d destination;
  Connection player_move_connection;
  Connection teleport_connection;
};