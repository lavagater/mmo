#ifndef STAT_SCALE_FACTORY
#define STAT_SCALE_FACTORY

#include <spell_value_factory.h>

class StatScaleFactory : public SpellValueFactory
{
public:
  SpellValue *Create(SpellData *data);
  SpellValue *Create(SpellData *data, Stream &stream);
};

#endif