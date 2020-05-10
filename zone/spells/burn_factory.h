/**
 * @brief Factory used to make burn effect
 */

#ifndef BURN_FACTORY_H
#define BURN_FACTORY_H

#include <effect_factory.h>

class BurnFactory : public EffectFactory
{
  public:
  //creates a random spell
  Effect *Create(SpellData *data);
  //recreates spell from data
  Effect *Create(SpellData *data, Stream &stream);
};

#endif