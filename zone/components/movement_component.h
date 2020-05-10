#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

/**
 * @brief compoinent that allows for an object to move.
 */
class MovementComponent : public Component
{
public:
  MovementComponent();
  void Init();
  void onUpdate(double dt);
  void MoveTo(Eigen::Vector2d pos);

  double speed;

  //signal is fired when the object is moved
  Signals<Eigen::Vector2d> moved_signal;
  Signals<> arrived_signal;
  Eigen::Vector2d destination;
  Eigen::Vector2d last_pos;
  Connection update_connection;
  bool updating;
};