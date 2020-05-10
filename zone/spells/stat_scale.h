/**
 * @brief A spell value that is a precentage of of the casters stat
 */

#ifndef STAT_SCALE_H
#define STAT_SCALE_H

#include <string>
#include <sstream>
#include "spell_value.h"
#include "entity.h"
#include "stream.h"

class StatScale : public SpellValue
{
  public:
  float get()
  {
    return caster->getStat(stat) * percentage;
  }
  static SpellValue *Create(Stream &stream, SpellData *spell_data)
  {
    (void)spell_data;
    StatScale *ret = new StatScale();
    stream >> ret->percentage;
    stream >> ret->stat;
    return ret;
  }
  std::string getDescription()
  {
    std::stringstream ss;
    ss << percentage * 100;
    //todo need a way to convert the stat into a string
    ss << "% of your " << stat;
    return ss.str();
  }
  Stream &ToData(Stream &stream)
  {
    stream << percentage;
    stream << stat;
    return stream;
  }
  SpellValue *copy()
  {
    StatScale *ret = new StatScale(*this);
    return ret;
  }
  Entity *caster;
  float percentage;
  Entity::Stats stat;
};

#endif