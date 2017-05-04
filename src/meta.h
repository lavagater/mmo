/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  When reading info from a file its all in strings this file will help make the strings
  into variables and types
*/
#include <stdlib.h>
#include <string.h>
#include <cassert>

enum
{
	type_bool,
  type_int,
  type_unsigned,
  //hex is unsigned but needs to be distinguishable
  type_hex,
  type_long,
  type_float,
  type_double,
  type_char,
  type_string,
  type_unknown,
  //nothing
  type_null,
  //the number of types there are
  type_amount
};

//this is the number of digits where floats become doubles for infer type
#define DOUBLE_LENGTH 10

/*!
  \brief
    Guesses the type of a string
  
  \param str
    The string representation of a built in type

  \return
    returns the type that is contained in the string. ex) "5" would return type_int
*/
int InferType(const char *str);

/*!
  \brief
    A class that can store any built in type
*/
class VoidWrapper
{
private:
	void *data;
public:
	/*!
	  \brief
	    Initalize things to zero
	*/
	VoidWrapper();
	/*!
	  \brief
	    Deletes the data
	*/
	~VoidWrapper();
	/*!
	  \brief
	    deletes the previous data and sets the data to a new value. does not work for char *
	    use set_string for char * instead
	  
	  \param value
	    The value of the data to set
	  \param ntype
	    The enum value of what type this wrapper is for
	*/
	template <typename T>
	void set(T value, int ntype)
	{
		//if the type was string then we need to delete pointer differently
		if (type == type_string)
		{
			delete [] (char*)data;
		}
		else
		{
			delete (int*)data;
		}
		data = new T(value);
		type = ntype;
	}
	/*!
	  \brief
	    same as set but for char *
	  
	  \param value
	    The value of the data to set
	  \param ntype
	    The enum value of what type this wrapper is for
	*/
	void set_string(char *value, int ntype = type_string)
	{
		//if the type was string then we need to delete pointer differently
		if (type == type_string)
		{
			delete [] (char*)data;
		}
		else
		{
			delete (int*)data;
		}
		data = new char[strlen(value)+1];
		strcpy((char*)data, value);
		type = ntype;
	}
	int type; /*!< The variable type that this class is representing*/
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an int
	*/
	explicit operator int();
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an unsigned
	*/
	explicit operator unsigned();
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an float
	*/
	explicit operator float();
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an double
	*/
	explicit operator double();
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an char
	*/
	explicit operator char();
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an bool
	*/
	explicit operator bool();
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an long
	*/
	explicit operator long();
	/*!
	  \brief
	    copies the string and returns that. the string will have to be deleted
	*/
	char *str();
	/*!
	  \brief
	    copies the data int the pointer p
	  \param p
	  	The char * to store the string in
	*/
	void str(char *p);
};

/*!
  \brief
    Converts a strng into another type. i.e. "true" would become bool true
  
  \param str
    The string representation of a built in type

  \return
    returns a wrapper around a void pointer that points at the correct type and value
*/
VoidWrapper StringToValue(const char *str);