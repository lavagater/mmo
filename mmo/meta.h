/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  When reading info from a file its all in strings this file will help make the strings
  into variables and types
*/
#ifndef META_H
#define META_H
#include <stdlib.h>
#include <string>
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
    Guesses the type of a string. Note that hex must be capital and spaces are not ignored
    strings must be inside double quotes. Gibberish will result in type_unknown
  
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
	    Copy constructor
	  \param rhs
	    the object to copy
	*/
	VoidWrapper(const VoidWrapper &rhs);
	/*!
	  \brief
	    deep copy another void wrapper
	  \param rhs
	    the object being copied
	*/
	 VoidWrapper &operator=(const VoidWrapper &rhs);
	 /*!
	  \brief
	    deep copy another void wrapper
	  \param rhs
	    the object being copied
	*/
	 void copy(const VoidWrapper &rhs);
	 /*!
	  \brief
	    frees the memory and resets the type
	*/
	 void clear();
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
		clear();
		type = ntype;
		if (ntype == type_null || ntype == type_unknown)
			return;
		data = new T(value);
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
		clear();
		data = new char[strlen(value)+1];
		strcpy((char*)data, value);
		type = ntype;
	}
	int type; /*!< The variable type that this class is representing*/
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an int
	*/
	explicit operator int() const;
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an unsigned
	*/
	explicit operator unsigned() const;
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an float
	*/
	explicit operator float() const;
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an double
	*/
	explicit operator double() const;
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an char
	*/
	explicit operator char() const;
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an bool
	*/
	explicit operator bool() const;
	/*!
	  \brief
	    turns the data pointed to by the void pointer into an long
	*/
	explicit operator long() const;
	/*!
	  \brief
	    turns the data from a char * to an std::string. Only works if type is type_string
	    otherwise behaviour is undefiened
	*/
	explicit operator std::string() const;
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
#endif