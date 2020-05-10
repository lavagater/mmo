#ifndef TRANSFORM_COMPONENT
#define TRANSFORM_COMPONENT

#include "component.h"
#include <Eigen/Dense>

class TransformComponent : public Component
{
public:
  void Load(std::istream &stream);
  void Write(std::ostream &stream);
  Eigen::Vector2d position;
};

#endif