#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include "meta.h"

int InferType(const char *str)
{
	//save origanal pointer
	const char *start = str;
	//check for signifagant features of the string
	int digits = 0;
	int decimals = 0;
	int single_quotes = 0;
	int double_quotes = 0;
	int negative_signs = 0;
	int hex = 0;
	int length = 0;
	while(*str)
	{
		if (*str == '.')
		{
			decimals += 1;
		}
		else if (*str == '\'')
		{
			single_quotes += 1;
		}
		else if (*str == '"')
		{
			double_quotes += 1;
		}
		else if (*str >= '0' && *str <= '9')
		{
			digits += 1;
		}
		else if (*str >= 'A' && *str <= 'F')
		{
			hex += 1;
		}
		else if (*str == '-')
		{
			negative_signs += 1;
		}
		length += 1;
		str += 1;
	}
	if (length == 0)
	{
		return type_null;
	}
	//check for string
	if (length >= 2 && start[0] == '"' && start[length-1] == '"')
	{
		return type_string;
	}
	//check for a char
	if (length == 3 && start[0] == '\'' && start[2] == '\'')
	{
		return type_char;
	}
	//if it only has numbers and decimals and minuses it could be a number
	if (digits + decimals + negative_signs == length)
	{
		//make sure the negative sign is at the begining it if exists
		if ((negative_signs && *start == '-') || !negative_signs)
		{
			//floating point there if one decimal
			if (decimals == 1)
			{
				if (digits >= DOUBLE_LENGTH)
				{
					return type_double;
				}
				else
				{
					return type_float;
				}
			}
			//non floating point if only digits and or negative signs
			if (digits + negative_signs == length)
			{
				//if its negative it cant be unsigned
				if (negative_signs)
				{
					//at 10 digits a normal int could over flow, so safer to stick it into an unsigned
					if (digits >= 10)
					{
						return type_long;
					}
					else
					{
						return type_int;
					}
				}
				else
				{
					//at 10 digits a normal int could over flow, so safer to stick it into an unsigned
					if (digits >= 10)
					{
						return type_long;
					}
					else if (digits > 6)
					{
						return type_unsigned;
					}
					else
					{
						return type_int;
					}
				}
			}
		}
	}
	//check for hexadecimal
	if (hex)
	{
		//hex could be in to formats 1) FF or 2) 0xFF
		//for format 2 the first zero is counted in the digits so we need add one 
		if (start[0] == '0' && (start[1] == 'x' || start[1] == 'X') && hex + digits + 1 == length)
		{
			return type_hex;
		}
		if (hex+digits == length)
		{
			return type_hex;
		}
	}
	//check for boolean
	if (strcmp(start, "false") == 0 || strcmp(start, "true")==0||strcmp(start, "TRUE")==0||strcmp(start, "FALSE")==0)
	{
		return type_bool;
	}
	return type_unknown;
}

VoidWrapper StringToValue(const char *str)
{
	int type = InferType(str);
	VoidWrapper res;
	switch (type)
	{
		case type_null:
		case type_int:
		res.set<int>(atoi(str), type);
		break;
		case type_long:
		res.set<long>(atol(str), type);
		break;
		case type_unsigned:
		res.set<unsigned>((unsigned)atol(str), type);
		break;
		case type_hex:
		res.set<unsigned>((unsigned)std::stoi(str, 0, 16), type);
		break;
		case type_float:
		res.set<float>((float)atof(str), type);
		break;
		case type_double:
		res.set<double>(atof(str), type);
		break;
		case type_bool:
		res.set<bool>(*str != 0 ? true : false, type);
		break;
		case type_char:
		res.set<char>(str[1], type);
		break;
		case type_string:
		{
			unsigned len = strlen(str);
			char *temp = new char[len];
			strcpy(temp, str+1);
			temp[len-2] = 0;
			res.set_string(temp, type);
		  break;
		}
	}
	return res;
}

VoidWrapper::VoidWrapper()
{
	data = 0;
	type = type_unknown;
}

VoidWrapper::operator int()
{
	return *reinterpret_cast<int*>(data);
}
VoidWrapper::operator unsigned()
{
	return *reinterpret_cast<unsigned*>(data);
}
VoidWrapper::operator float()
{
	return *reinterpret_cast<float*>(data);
}
VoidWrapper::operator double()
{
	return *reinterpret_cast<double*>(data);
}
VoidWrapper::operator char()
{
	return *reinterpret_cast<char*>(data);
}
VoidWrapper::operator bool()
{
	return *reinterpret_cast<bool*>(data);
}
VoidWrapper::operator long()
{
	return *reinterpret_cast<long*>(data);
}
char *VoidWrapper::str()
{
	return (char*)data;
}