/*!
  \author
    Wyatt Lavigueure
  \date
    5/14/2017
  \brief
    A wrapper around a char array, but indexing into the array gets a single bit. The class is templated
    the template argument is the size of the array of bits rounded up to the nearest divider of 8.
    ex BitArray<9> would have 16 bits
*/
#ifndef BIT_H
#define BIT_H

template<unsigned size>
class BitArray
{
public:
  /*!
    \brief
      Initailizes the bits to 0/false
  */
  BitArray();
  /*!
    \brief
      Gets the actuall number of bits we can use, wont always be the same as size
    \return
      the actaul size
  */
  int GetSize() const;
  /*!
    \brief
      Sets a bit in the bit array to a certain value(one or zero).
    \param index
      The index into the bit array 0 is the first bit 1 is the second bit
    \param value
      if true the bit is set to 1, if false bit is set to 0
  */
  void SetBit(unsigned index, bool value = true);
  /*!
    \brief
      Checks if the bit at the index is set
    \param index
      The index into the bit array 0 is the first bit 1 is the second bit
    \return
      True if the bit is 1 and false if the bit is 0
  */
  bool operator[](unsigned index) const;
  /*!
    \brief
      The internal buffer, the size if ceil(size/8)
  */
  char buffer[size/8==size/8.0?size/8:size/8+1];
};

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
      break;
      case 1:
      buffer[index/8] |= 2;
      break;
      case 2:
      buffer[index/8] |= 4;
      break;
      case 3:
      buffer[index/8] |= 8;
      break;
      case 4:
      buffer[index/8] |= 16;
      break;
      case 5:
      buffer[index/8] |= 32;
      break;
      case 6:
      buffer[index/8] |= 64;
      break;
      case 7:
      buffer[index/8] |= 128;
      break;
    }
  }
  else
  {
    switch(index%8)
    {
      case 0:
      buffer[index/8] &= 255 - 1;
      break;
      case 1:
      buffer[index/8] &= 255 - 2;
      break;
      case 2:
      buffer[index/8] &= 255 - 4;
      break;
      case 3:
      buffer[index/8] &= 255 - 8;
      break;
      case 4:
      buffer[index/8] &= 255 - 16;
      break;
      case 5:
      buffer[index/8] &= 255 - 32;
      break;
      case 6:
      buffer[index/8] &= 255 - 64;
      break;
      case 7:
      buffer[index/8] &= 255 - 128;
      break;
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
#endif