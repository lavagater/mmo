#include <string.h>

#include "database_protocol.h"
#include "query.h"

int ReadValue(char *buffer, int size, std::vector<Value> &parameters)
{
  LOG("Read value");
  (void)size; //fuck safety
  Types type = (Types)buffer[0];
  LOG("Type = " << type);
  buffer += 1;
  parameters.emplace_back(Value());
  Value &ret = parameters.back();
  ret.type = type;
  LOG("Start switch");
  switch(type)
  {
		case Char:
      ret.m_char = *reinterpret_cast<char*>(buffer);
			return 1 + sizeof(char);
		case Integer:
			ret.m_int = *reinterpret_cast<int*>(buffer);
			return 1 + sizeof(int);
		case Unsigned:
			ret.m_unsigned = *reinterpret_cast<unsigned*>(buffer);
			return 1 + sizeof(unsigned);
		case Short:
			ret.m_short = *reinterpret_cast<short*>(buffer);
			return 1 + sizeof(short);
		case Float:
			ret.m_float = *reinterpret_cast<float*>(buffer);
			return 1 + sizeof(float);
		case Double:
			ret.m_double = *reinterpret_cast<double*>(buffer);
			return 1 + sizeof(double);
		case String:
		{
      LOG("String switch");
      //read the length
      int length = buffer[0];
      LOG("length of buffer = " << length);
      buffer += 1;
      ret.m_string = std::string(buffer, length);
      LOG("string = " << ret.m_string);
      return 2 + length;
		}
		default:
    {
      return -1;
    }
		}
    return -1;
}


bool ParseQueryMessage(char *buffer, unsigned size, unsigned &id, std::string &script, std::vector<Value> &parameters)
{
    if (size < sizeof(MessageType) + sizeof(short))
    {
      LOGW("ParseQueryMessage bad packet");
      return false;
    }
    //skip the message type
    buffer += sizeof(MessageType);
    //read in the id
    id = *reinterpret_cast<unsigned*>(buffer);
    buffer += sizeof(unsigned);
    //read in the script
    short length = *reinterpret_cast<short*>(buffer);
    LOG("Script length = " << length);
    buffer += sizeof(short);
    if (size < sizeof(MessageType) + sizeof(short) + length)
    {
      LOGW("ParseQueryMessage bad packet");
      return false;
    }
    script = std::string(buffer, length);
    LOG("Script = " << script);
    buffer += length;
    //now read in each parameter
    int remaining = (int)size - (sizeof(MessageType) + sizeof(short) + sizeof(unsigned) + length);
    while (remaining > 0)
    {
      int ret = ReadValue(buffer, remaining, parameters);
      LOG("Read paramater ret = " << ret);
      if (ret<0)
      {
        LOGW("ParseQueryMessage bad packet");
        return false;
      }
      buffer += ret;
      remaining -= ret;
    }
    return true;
}

int CreateQueryResponse(ProtocolLoader &protocol, unsigned id, char *buffer, Value returnValue)
{
  int ret = 0;
  *reinterpret_cast<MessageType*>(buffer) = protocol.LookUp("Query");
  ret += sizeof(MessageType);
  *reinterpret_cast<unsigned*>(buffer+ret) = id;
  ret += sizeof(unsigned);
  ret += Query::PackValue(buffer+ret, returnValue);
  return ret;
}
bool ParseQueryResponse(char *buffer, int length, unsigned &id, char *&data, unsigned &size)
{
  if (length < (int)(sizeof(MessageType) + sizeof(unsigned)))
  {
    return false;
  }
  //skip message type
  buffer += sizeof(MessageType);
  //read id
  id =*reinterpret_cast<unsigned*>(buffer);
  buffer += sizeof(unsigned);
  //get the data at the end
  size = length - (sizeof(MessageType) + sizeof(unsigned));
  data = buffer;
  return true;
}