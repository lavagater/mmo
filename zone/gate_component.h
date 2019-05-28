#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

class GateComponent : public Component
{
public:
  void Load(std::istream &stream);
  void Write(std::ostream &stream);

  Eigen::Vector2d destination;
  std::string zone;
};