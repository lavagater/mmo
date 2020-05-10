#include <stat_scale_factory.h>
#include <stat_scale.h>
#include "entity_component.h"

SpellValue *StatScaleFactory::Create(SpellData *data)
{
  StatScale *ret = new StatScale();
  ret->type = type;
  ret->caster = &GETCOMP(data->owner, EntityComponent)->entity;
  //todo use a value in spell data to determine apporazxamitly how good the spell should be
  ret->percentage = double(rand()) / RAND_MAX;
  ret->stat = static_cast<Entity::Stats>(rand()%Entity::Stats::num);
  return ret;
}
SpellValue *StatScaleFactory::Create(SpellData *data, Stream &stream)
{
  return StatScale::Create(stream, data);
}