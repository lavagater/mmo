#include "burn_factory.h"
#include "burn_effect.h"
#include "spell_factory.h"
#include "spell_value_factory.h"

Effect *BurnFactory::Create(SpellData *data)
{
  BurnEffect *ret = new BurnEffect();
  ret->dispatcher = &data->zone->dispatcher;
  ret->dps = data->spell_value_factories[rand()%data->spell_value_factories.size()]->Create(data);
  //todo add a scale thing to spell data
  ret->duration = rand()%60+15;
  ret->type = type;
  return ret;
}

Effect *BurnFactory::Create(SpellData *data, Stream &stream)
{
  return BurnEffect::Create(stream, data);
}