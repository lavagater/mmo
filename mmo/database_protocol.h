/*!
  \author
    Wyatt Lavigueure
  \date
    8/6/2017
  \brief
    Functions for the database messages
*/
#ifndef DATABASE_PROTOCOL_H
#define DATABASE_PROTOCOL_H

#include "protocol.h"
#include "logger.h"
#include "parser.h"
#include "types.h"

#include <cstring>

//returns the amount written into the buffer
template<typename T>
int FillValues(char *buffer, T value)
{
  //this actually give compiling warnings if its used which is nice

	//if no specialization then ignore argument, and log a warning
	LOGW("Creating query with a unexpected type of size  = " << sizeof(T));
	return 0;
}

//specializations
template<>
inline int FillValues<BlobStruct>(char *buffer, BlobStruct next)
{
	//add integer to the buffer
	buffer[0] = Types::Blob;
  *reinterpret_cast<unsigned*>(buffer+1) = next.size;
  memcpy(buffer + 1 + sizeof(unsigned), next.data, next.size);
  std::cout << "Fill value blob " << ToHexString(next.data, next.size) << std::endl;
  return 1 + sizeof(unsigned) + next.size;
}
template<>
inline int FillValues<int>(char *buffer, int next)
{
	//add integer to the buffer
	buffer[0] = Types::Integer;
  *reinterpret_cast<int*>(buffer+1) = next;
  return 1 + sizeof(next);
}
template<>
inline int FillValues<unsigned>(char *buffer, unsigned next)
{
	//add integer to the buffer
	buffer[0] = Types::Unsigned;
  *reinterpret_cast<unsigned*>(buffer+1) = next;
  return 1 + sizeof(next);
}
template<>
inline int FillValues<short>(char *buffer, short next)
{
	//add integer to the buffer
	buffer[0] = Types::Short;
  *reinterpret_cast<short*>(buffer+1) = next;
  return 1 + sizeof(next);
}
template<>
inline int FillValues<char>(char *buffer, char next)
{
	//add integer to the buffer
	buffer[0] = Types::Char;
  buffer[1] = next;
  return 1 + sizeof(next);
}
template<>
inline int FillValues<double>(char *buffer, double next)
{
	//add integer to the buffer
	buffer[0] = Types::Double;
  *reinterpret_cast<double*>(buffer+1) = next;
  return 1 + sizeof(next);
}
template<>
inline int FillValues<float>(char *buffer, float next)
{
	//add integer to the buffer
	buffer[0] = Types::Float;
  *reinterpret_cast<float*>(buffer+1) = next;
  return 1 + sizeof(next);
}
template<>
inline int FillValues<std::string>(char *buffer, std::string next)
{
	//the type is string add the string to the buffer
	buffer[0] = Types::String;
  buffer[1] = next.length();
  for (unsigned i = 0; i < next.length(); ++i)
  {
    buffer[i + 2] = next[i];
  }
  return 2+next.length();
}

template<typename T, typename ...Args>
int FillValues(char *buffer, T next, Args ...rest)
{
	//add the next paramater to the buffer
	int ret = FillValues(buffer, next);
	//add the rest to the buffer
	return FillValues(buffer+ret, rest...) + ret;
}

//returns length of message
template<typename ...Args>
int CreateQueryMessage(ProtocolLoader &protocol, unsigned id, char *buffer, std::string script, Args ...values)
{
  MessageType type = protocol.LookUp("Query");
  int ret = 0;
  //set type
  *reinterpret_cast<MessageType*>(buffer+ret) = type;
  ret += sizeof(MessageType);
  //add id
  *reinterpret_cast<unsigned*>(buffer+ret) = id;
  ret += sizeof(unsigned);
  //add the script
  *reinterpret_cast<short*>(buffer+ret) = script.length();
  ret += sizeof(short);
  for (unsigned i = 0; i < script.length(); ++i)
  {
    buffer[i+ret] = script[i];
  }
  ret += script.length();
	ret += FillValues(buffer+ret, values...);
	return ret;
}

//helper for the parse query message that reads a value out of the buffer
int ReadValue(char *buffer, int size, std::vector<Value> &parameters, std::vector<char*> memory);
//returns false if the packet was made wrong(which should not happen)
bool ParseQueryMessage(char *buffer, unsigned size, unsigned &id, std::string &script, std::vector<Value> &parameters, std::vector<char*> memory);
//create a query response mesage
int CreateQueryResponse(ProtocolLoader &protocol, unsigned id, char *buffer, Value returnValue);
bool ParseQueryResponse(char *buffer, int length, unsigned &id, char *&data, unsigned &size);
#endif