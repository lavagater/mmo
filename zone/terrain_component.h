#include "component.h"
#include "signals.h"
#include <Eigen/Dense>

class TerrainComponent : public Component
{
public:
  void Init();
  void onUpdate(double dt);
  void OnPlayerJoined(GameObject *new_player);
  Connection player_joined_connection;
  Connection update_connection;
};