/**
 * @brief Base class for all the spell factories, used to create spells
 */

#ifndef SPELL_FACTORY_H
#define SPELL_FACTORY_H

#include <string>
#include <spell.h>
#include <stream.h>
#include <game_object.h>

class Zone;
class SpellValueFactory;
class EffectFactory;
class SpellFactory;

// contains all info that any spell could need
struct SpellData
{
  GameObject *owner;
  Zone *zone;
  //contains the factories to make spell values
  std::vector<SpellValueFactory*> spell_value_factories;
  //contains the factories to make overtime effects
  std::vector<EffectFactory*> over_time_factories;
  //contains the factories to make instant effects
  std::vector<EffectFactory*> instant_effect_factories;
  //not needed by other factories, but i put this here because so all the
  //factories are in the same place
  std::vector<SpellFactory*> spell_factories;
};

class SpellFactory
{
  public:
  //creates a random spell
  virtual Spell *CreateSpell(SpellData *data) = 0;
  //recreates spell from data
  virtual Spell *CreateSpell(SpellData *data, Stream &stream) = 0;
  char type;
};

#endif