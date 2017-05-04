/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  Test cases

*/
/*****************************************************************************/
#include <iostream>
#include <sstream>
#include <string.h>

#include "wyatt_sock.h"
#include "meta.h"

#define PRINT_ERROR std::cout << "line " << __LINE__ << std::endl; 

bool TestInferType();
bool TestStringToValue();

bool (*tests[])() = { 
    TestInferType, TestStringToValue
}; 

int main()
{
	int num_failed = 0;
	for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i)
	{
		if (!tests[i]())
		{
			num_failed += 1;
		}
	}
	if (num_failed)
	{
		getchar();
	}
  return 1337;
}

//test cases
bool TestInferType()
{
	//check int
	for (int i = 0; i < 999999; i += 13)
	{
		std::stringstream s;
		s << i;
		int type = InferType(s.str().c_str());
		if (type != type_int)
		{
			PRINT_ERROR
			std::cout << "str = " << s.str() << std::endl;
			std::cout << "type is " << type << std::endl;
			return false;
		}
	}
	//check negative
	for (long i = -9999999999; i < -9999998999; i += 753)
	{
		std::stringstream s;
		s << i;
		int type = InferType(s.str().c_str());
		if (type != type_int && type != type_long)
		{
			PRINT_ERROR
			std::cout << "str = " << s.str() << std::endl;
			std::cout << "type is " << type << std::endl;
			return false;
		}
	}
	//check bool
	int type = InferType("TRUE");
	if (type != type_bool)
	{
		PRINT_ERROR
		std::cout << "type is " << type << std::endl;
		return false;
	}
	type = InferType("false");
	if (type != type_bool)
	{
		PRINT_ERROR
		std::cout << "type is " << type << std::endl;
		return false;
	}
	type = InferType("FALSE");
	if (type != type_bool)
	{
		PRINT_ERROR
		std::cout << "type is " << type << std::endl;
		return false;
	}
	type = InferType("true");
	if (type != type_bool)
	{
		PRINT_ERROR
		std::cout << "type is " << type << std::endl;
		return false;
	}
	//check floating point
	for (int j = 0; j < 100; ++j)
	{
		for (int i = -50; i < 50; ++i)
		{
			if (i == 0)
				continue;
		  double test = double(j) / double(i);
		  //make sure its not an int
		  if (test == int(test))
		  {
		  	continue;
		  }
			std::stringstream s;
			s << test;
			int type = InferType(s.str().c_str());
			if (type != type_float && type != type_double)
			{
				PRINT_ERROR
				std::cout << "str = " << s.str() << std::endl;
				std::cout << "type is " << type << std::endl;
				return false;
			}
		}
	}
	//check char
	{//start a scope so i can copy pasta
		char temp[] = "\'a\'";
		temp[1] = 1;
		for (int i = 0; i < 127; ++i)
		{
			type = InferType(temp);
			if (type != type_char)
			{
				PRINT_ERROR
				std::cout << "str is "<< temp << std::endl;
				std::cout << "type is " << type << std::endl;
			}
			temp[1] += 1;
		}
	}//end scope
	//check string
	{//start a scope so i can copy pasta
		char temp[] = "\"baby sitter\"";
		for (int i = 0; i < 1337; ++i)
		{
			type = InferType(temp);
			if (type != type_string)
			{
				PRINT_ERROR
				std::cout << "str is "<< temp << std::endl;
				std::cout << "type is " << type << std::endl;
			}
			temp[i%11+1] += i % 77;
			if (temp[i%11+1] == 0)
			{
				temp[i%11+1] = 'a';
			}

		}
	}//endscope

  //test nothing
  type = InferType("");
	if (type != type_null)
	{
		PRINT_ERROR
		std::cout << "type is " << type << std::endl;
		return false;
	}

	return true;
}

bool TestStringToValue()
{
	//test int
	if ((int)StringToValue("14") != 14)
	{
		PRINT_ERROR
		std::cout << "14 gives " << (int)StringToValue("14") << std::endl; 
		return false;
	}
	//test negative
	if ((int)StringToValue("-2") != -2)
	{
		PRINT_ERROR
		std::cout << "-2 gives " << (int)StringToValue("-2") << std::endl; 
		return false;
	}
	//test big number
	if ((long)StringToValue("1234567890") != 1234567890)
	{
		PRINT_ERROR
		std::cout << "1234567890 gives " << (long)StringToValue("1234567890") << std::endl; 
		return false;
	}
	//test floating points
	if ((double)StringToValue("1234.567890") != 1234.567890)
	{
		PRINT_ERROR
		std::cout << "1234.567890 gives " << (double)StringToValue("1234.567890") << std::endl; 
		return false;
	}
	//test empty string
	if ((int)StringToValue("") != 0)
	{
		PRINT_ERROR
		std::cout << "\"\" = " << (int)StringToValue("") << std::endl; 
		return false;
	}
	//test boolean
	if ((bool)StringToValue("true") != true)
	{
		PRINT_ERROR
		std::cout << "true gives " << (bool)StringToValue("true") << std::endl; 
		return false;
	}
	//test boolean
	if ((bool)StringToValue("TRUE") != true)
	{
		PRINT_ERROR
		std::cout << "TRUE gives " << (bool)StringToValue("TRUE") << std::endl; 
		return false;
	}
	//test boolean
	if ((bool)StringToValue("false") != false)
	{
		PRINT_ERROR
		std::cout << "false gives " << (bool)StringToValue("false") << std::endl; 
		return false;
	}
	//test boolean
	if ((bool)StringToValue("FALSE") != false)
	{
		PRINT_ERROR
		std::cout << "FALSE gives " << (bool)StringToValue("FALSE") << std::endl; 
		return false;
	}
	//test char
	if ((char)StringToValue("'f'") != 'f')
	{
		PRINT_ERROR
		std::cout << "'f' gives " << (char)StringToValue("'f'") << std::endl; 
		return false;
	}
	//test string
	char * str = StringToValue("\"huehuehue\"").str();
	if (strcmp(str, "huehuehue") != 0)
	{
		PRINT_ERROR
		std::cout << "huehuehue !=" << str << std::endl; 
		return false;
	}
	delete [] str;
	str = new char[100];
	//test string
	StringToValue("\"abcdefghijklmnopqrstuvwxyzzzz\"").str(str);
	if (strcmp(str, "abcdefghijklmnopqrstuvwxyzzzz") != 0)
	{
		PRINT_ERROR
		std::cout << "abcdefghijklmnopqrstuvwxyzzzz !=" << str << std::endl; 
		return false;
	}
	delete [] str;
	return true;
}