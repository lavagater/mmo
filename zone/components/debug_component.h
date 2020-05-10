#ifndef DEBUG_COMPONENT_H
#define DEBUG_COMPONENT_H

#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>
#include "shapes.h"

class DebugComponent : public Component
{
public:
  void Init();
  void OnUpdate(double dt);
  void SendDebugInfo();
  void SetRate(float rate);

  //how often to send debug packet
  float send_rate = 1;//every second
  float timer;
  Connection update_connection;
};
#endif