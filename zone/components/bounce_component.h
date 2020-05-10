#ifndef BOUNCE_COMPONENT_H
#define BOUNCE_COMPONENT_H

#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>
#include "shapes.h"

class BounceComponent : public Component
{
public:
  void Init();
  void OnUpdate(double dt);
  void OnCollision(GameObject *other);
  void SendMoveMessage();

  int bounces;
  Eigen::Vector2d velocity;
  Connection update_connection;
  Connection collision_connection;
};
#endif