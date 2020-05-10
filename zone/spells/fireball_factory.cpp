#include <fireball_factory.h>
#include "fireball_spell.h"
#include "spell_value_factory.h"

Spell *FireBallFactory::CreateSpell(SpellData *data)
{
  FireBall *ret = new FireBall();
  ret->type = type;
  //need a scaling value in spell data
  ret->cast_time = 0.5 +rand() % 100 / 20.0;
  ret->cool_down = rand()%15;
  ret->range = rand()%15;
  ret->mana_cost = rand()%100;
  ret->zone = data->zone;
  ret->caster = GETCOMP(data->owner, EntityComponent);
  //get a random value
  ret->damage_amount = data->spell_value_factories[rand()%data->spell_value_factories.size()]->Create(data);
  ret->burn = (OverTimeEffect*)data->over_time_factories[rand()%data->over_time_factories.size()]->Create(data);
  return ret;
}
Spell *FireBallFactory::CreateSpell(SpellData *data, Stream &stream)
{
  return FireBall::Create(stream, data);
}