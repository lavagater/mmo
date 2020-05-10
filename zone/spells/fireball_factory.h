/**
 * @brief Factory used to make fireballs
 */

#ifndef FIREBALL_SPELL_FACTORY_H
#define FIREBALL_SPELL_FACTORY_H

#include <spell_factory.h>

class FireBallFactory : public SpellFactory
{
  public:
  //creates a random spell
  Spell *CreateSpell(SpellData *data);
  //recreates spell from data
  Spell *CreateSpell(SpellData *data, Stream &stream);
};

#endif