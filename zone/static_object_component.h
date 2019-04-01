#include "component.h"
#include "signals.h"
#include <Eigen/Dense>

//component for objects that wont move
class StaticObjectComponent : public Component
{
public:
  void Init();
  void OnPlayerJoined(GameObject *new_player);
  Connection player_joined_connection;
};