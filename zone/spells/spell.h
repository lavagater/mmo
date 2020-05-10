/**
 * @brief The effect is the result of when a skill is used on an entity
 */

#ifndef SPELL_H
#define SPELL_H

#include <string>
#include "stream.h"
#include "effect.h"

class Spell : public Effect
{
  public:
  //spell values
  float range;
  float mana_cost;
  float cool_down;
  float cast_time;
};

#endif