/**
 * @brief This component should be on an object with no other components except a transform, this object will tell players
 *         about the terrain that wont change.
 * @remarks There could be other terrain that can move or be created/destroyed that information will be sent in a different way.
 */

#ifndef TERRAIN_COMP_H
#define TERRAIN_COMP_H

#include "component.h"
#include "signals.h"
#include <Eigen/Dense>

class TerrainComponent : public Component
{
public:
  void Init();
  void onUpdate(double dt);
  void OnPlayerJoined(GameObject *new_player);
  void Load(std::istream &stream);
  void Write(std::ostream &stream);
  Connection player_joined_connection;
  Connection update_connection;
  unsigned terrain_id;
};

#endif