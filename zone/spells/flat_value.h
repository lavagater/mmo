/**
 * @brief A value that is just a number
 */

#ifndef FLAT_VALUE_H
#define FLAT_VALUE_H

#include "spell_value.h"
#include "entity.h"

class FlatValue : public SpellValue
{
  public:
  float get()
  {
    return value;
  }
  std::string getDescription()
  {
    std::stringstream ss;
    ss << value;
    return ss.str();
  }
  Stream &ToData(Stream &stream)
  {
    stream << value;
    return stream;
  }
  SpellValue *copy()
  {
    FlatValue *ret = new FlatValue(*this);
    return ret;
  }
  float value;
};

#endif