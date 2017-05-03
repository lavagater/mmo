/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  When reading info from a file its all in strings this file will help make the strings
  into variables and types
*/

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