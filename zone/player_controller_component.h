#include "component.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

class PlayerControllerComponent : public Component
{
public:
  void Init();
  void onUpdate(double dt);
  void OnMove(char *buffer, unsigned n, sockaddr_in *addr);

  Eigen::Vector2d destination;
  unsigned id;
  sockaddr_in lb_addr;
};