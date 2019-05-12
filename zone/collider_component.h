#ifndef COLLIDER_COMPONENT_H
#define COLLIDER_COMPONENT_H

#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include "shapes.h"
#include <unordered_set>
#include <Eigen/Dense>

class ColliderComponent : public Component
{
public:
  ~ColliderComponent();
  void Load(std::istream &stream);
  void Write(std::ostream &stream);
  void Init();
  void OnBaseUpdate();
  //shape is used to actually check collision, and to determine which grid squares are within
  Shape *shape = 0;
  //the colliders collision group
  unsigned collision_group;
  //vector of the grids the collider is in
  std::vector<std::pair<int, int>> prev_grid;
  //signal that is fired when and object starts colliding with another object
  Signals<GameObject*> collision_started_signal;
  //signal for an object is still colliding with an object
  Signals<GameObject*> collision_persisted_signal;
  //signal for when an object has stopped colliding with an object
  Signals<GameObject*> collision_ended_signal;
  //collisions from the previous frame
  std::unordered_set<GameObject*> previous_collisions;
  Connection update_connection;
};
#endif