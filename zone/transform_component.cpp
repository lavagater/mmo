#include "transform_component.h"

void TransformComponent::Load(std::istream &stream)
{
  stream >> position.x();
  stream >> position.y();
}

void TransformComponent::Write(std::ostream &stream)
{
  stream << position.x();
  stream << position.y();
}