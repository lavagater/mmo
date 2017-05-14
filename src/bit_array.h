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

//because its templated
#include "bit_array.cpp"
#endif