
#include "blowfish.h"

BlowFish::BlowFish(unsigned *key, unsigned size)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			s_box[i][j] = 0;
		}
	}
	for (int i = 0; i < 18; ++i)
	{
		keys[i] = key[i % size];
	}
	unsigned lhs = 0, rhs = 0;
	for (int i = 0; i < 18; ++i)
	{
		encrypt(lhs, rhs);
		keys[i] = lhs;
		keys[i+1] = rhs;
	}
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			encrypt(lhs, rhs);
			s_box[i][j] = lhs;
			s_box[i][j] = rhs;
		}
	}
}

unsigned BlowFish::round_function(unsigned x)
{
  //copied from wikipidea
  unsigned h = s_box[0][x >> 24] + s_box[1][x >> 16 & 0xff];
  return ( h ^ s_box[2][x >> 8 & 0xff] ) + s_box[3][x & 0xff];
}

void BlowFish::encrypt(unsigned &lhs, unsigned &rhs)
{
  for (int i = 0; i < 16; i += 2)
  {
  	lhs ^= keys[i];
  	rhs ^= round_function(lhs);
  	rhs ^= keys[i+1];
  	lhs ^= round_function(rhs);
  }
  lhs ^= keys[16];
  rhs ^= keys[17];
  //swap left and right
  unsigned temp = rhs;
  rhs = lhs;
  lhs = temp;
}

void BlowFish::decrypt(unsigned &lhs, unsigned &rhs)
{
  for (int i = 16; i > 0; i -= 2)
  {
  	lhs ^= keys[i+1];
  	rhs ^= round_function(lhs);
  	rhs ^= keys[i];
  	lhs ^= round_function(rhs);
  }
  lhs ^= keys[1];
  rhs ^= keys[0];
  //swap left and right
  unsigned temp = rhs;
  rhs = lhs;
  lhs = temp;
}
