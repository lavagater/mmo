#include "bit_array.h"

template<unsigned size>
BitArray<size>::BitArray()
{
  for (unsigned i = 0; i < sizeof(buffer); ++i)
  {
    buffer[i] = 0;
  }
}

template<unsigned size>
int BitArray<size>::GetSize() const
{
  return size/8==size/8.0?size/8:size/8+1;
}
template<unsigned size>
void BitArray<size>::SetBit(unsigned index, bool value)
{
  if (value)
  {
    switch(index%8)
    {
      case 0:
      buffer[index/8] |= 1;
      case 1:
      buffer[index/8] |= 2;
      case 2:
      buffer[index/8] |= 4;
      case 3:
      buffer[index/8] |= 8;
      case 4:
      buffer[index/8] |= 16;
      case 5:
      buffer[index/8] |= 32;
      case 6:
      buffer[index/8] |= 64;
      case 7:
      buffer[index/8] |= 128;
    }
  }
  else
  {
    switch(index%8)
    {
      case 0:
      buffer[index/8] &= 255 - 1;
      case 1:
      buffer[index/8] &= 255 - 2;
      case 2:
      buffer[index/8] &= 255 - 4;
      case 3:
      buffer[index/8] &= 255 - 8;
      case 4:
      buffer[index/8] &= 255 - 16;
      case 5:
      buffer[index/8] &= 255 - 32;
      case 6:
      buffer[index/8] &= 255 - 64;
      case 7:
      buffer[index/8] &= 255 - 128;
    }
  }
}
template<unsigned size>
bool BitArray<size>::operator[](unsigned index) const
{
  switch(index%8)
  {
    case 0:
    return buffer[index/8] & 1;
    case 1:
    return buffer[index/8] & 2;
    case 2:
    return buffer[index/8] & 4;
    case 3:
    return buffer[index/8] & 8;
    case 4:
    return buffer[index/8] & 16;
    case 5:
    return buffer[index/8] & 32;
    case 6:
    return buffer[index/8] & 64;
    case 7:
    return buffer[index/8] & 128;
  }
  return false;
}