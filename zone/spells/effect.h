/**
 * @brief Base class for buffs and effects that last for a certain amount of time.
 */

#ifndef EFFECT_H
#define EFFECT_H

#include <string>
#include <spell.h>

class EntityComponent;
class SpellData;

class Effect
{
  public:
  virtual ~Effect(){}
  virtual Effect *copy() = 0;
  virtual void cast(EntityComponent *target) = 0;
  //get description is used for the player that has the skill
  virtual std::string getDescription() = 0;
  virtual Stream &ToData(Stream &stream) = 0;
  //the index into the factory
  char type;
};

#endif