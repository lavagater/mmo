#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

class PlayerControllerComponent : public Component
{
public:
  void Init();
  void onUpdate(double dt);
  void OnDisconnected(char *buffer, unsigned n, sockaddr_in *addr);
  void OnMove(char *buffer, unsigned n, sockaddr_in *addr);
  void SendMoveMessage();

  Eigen::Vector2d destination;
  unsigned id;
  sockaddr_in lb_addr;
  //dont send stuff to this address its just used to validate messages came from the correct client
  sockaddr_in client_addr;
  Connection update_connection;
  Connection player_move_connection;
  Connection player_disconnected;
};