/**
 * @brief Small wrapper around a char pointer to keep track of the position in the stream. 
 *        features are kept to a minimal, its it expected for the user to know whats happening and to access the data/size/pos members
 *        for anything they need.
 */
#ifndef STREAM_H
#define STREAM_H

#include <cstdint>
#include <string.h>

class Stream
{
public:
	Stream(char *data, uint32_t size):data(data), size(size), pos(0)
	{

	}
	template<typename T>
	Stream& operator<<(const T&val)
	{
		if (pos + sizeof(T) <= size)
		{
			*reinterpret_cast<T*>(data+pos) = val;
			pos += sizeof(T);
		}
		return *this;
	}
  template<typename T>
	Stream& operator>>(T& val)
	{
		if (pos < sizeof(T))
		{
			return *this;
		}
		pos -= sizeof(T);
		val = *reinterpret_cast<T*>(data + pos);
		return *this;
	}
	bool write(const char *new_data, uint32_t n)
	{
		if (pos+n > size)
		{
			return false;
		}
		memcpy(data+pos, new_data, n);
		pos += n;
		return true;
	}
	char* data;
	uint32_t size;
	uint32_t pos;
};

#endif