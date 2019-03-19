#include "component.h"
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
};