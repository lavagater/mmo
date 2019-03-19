#include "component.h"
#include <Eigen/Dense>

class TransformComponent : public Component
{
public:
  Eigen::Vector2d position;
};