#ifndef PROJECTILE_COMPONENT_H
#define PROJECTILE_COMPONENT_H

#include "component.h"
#include "signals.h"
#include <Eigen/Dense>

//literally just a component that makes an object move in a direction atm
class ProjectileComponent : public Component
{
public:
  void Init();
  void OnUpdate(double dt);

  Eigen::Vector2d velocity;
  Connection update_connection;
};

#endif