#ifndef FLAT_FACTORY
#define FLAT_FACTORY

#include <spell_value_factory.h>
#include "flat_value.h"

class FlatFactory : public SpellValueFactory
{
public:
  SpellValue *Create(SpellData *data)
  {
    (void)(data);
    FlatValue *ret = new FlatValue();
    ret->type = type;
    //toto should have a spell strength value in spelldata to scale the value generated
    ret->value = rand()/(double)RAND_MAX * (rand()%100);
    return ret;
  }
  SpellValue *Create(SpellData *data, Stream &stream)
  {
    (void)(data);
    FlatValue *ret = new FlatValue();
    stream >> ret->value;
    return ret;
  }
};

#endif