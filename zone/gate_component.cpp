#include "gate_component.h"
#include "logger.h"

void GateComponent::Load(std::istream &stream)
{
  stream >> destination.x();
  stream >> destination.y();
  //read the quoted text for the destination zone
  int count = 0;
  while(count < 2 && !stream.eof())
  {
    char c = stream.get();
    if (c == '"')
    {
      count += 1;
    }
    else if (count == 1)
    {
      zone += c;
    }
  }
  LOGW("Zone = " << zone);
  //hard coded 16 is how big the zone name can be in the database
  if (count != 2 || zone.length() > 16)
  {
    LOGW("number of quotes = " << count << " zone = " << zone);
  }
}
void GateComponent::Write(std::ostream &stream)
{
  stream << destination.x();
  stream << "\n";
  stream << destination.y();
  stream << "\n";
  stream << zone;
  stream << "\n";
}