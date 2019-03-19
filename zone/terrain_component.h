#include "component.h"
#include "signals.h"
#include <Eigen/Dense>

class TerrainComponent : public Component
{
public:
  void Init();
  void onUpdate(double dt);
  void OnPlayerJoined(GameObject *new_player);
  //at the moment each terrain can only be in 1 position
  std::pair<int, int> prev_pos;
  Eigen::Vector2d rectangle;
  Connection player_joined_connection;
  Connection update_connection;
};