#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

class PlayerControllerComponent : public Component
{
public:
  void Init();
  void onUpdate(double dt);
  void OnMove(char *buffer, unsigned n, sockaddr_in *addr);
  void SendMoveMessage();

  Eigen::Vector2d destination;
  unsigned id;
  sockaddr_in lb_addr;
  Connection update_connection;
  Connection player_move_connection;
};