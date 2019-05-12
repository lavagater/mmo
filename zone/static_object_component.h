#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

//component for objects that wont move
class StaticObjectComponent : public Component
{
public:
  void Init();
  void OnPositionUpdateRequest(char *buffer, unsigned n, sockaddr_in *addr);
  void OnPlayerJoined(GameObject *new_player);
  void SendPositionUpdate(GameObject *player);
  Connection player_joined_connection;
};