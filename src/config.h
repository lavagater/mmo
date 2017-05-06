/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  Used to read in config properties from a config file.
  Format of the config file is property name followed by any number of spces
  followed by a colon followed by any number of spaces then the value followed by
  any number of newlines
  example\n
  name : "bob"\n
  age:12\n
  \n
  is cool  :  true\n
  id : 0x7F7F\n
  \n
  For more examples see test_conf.txt


*/
#ifndef CONFIG_H
#define CONFIG_H
#include <unordered_map>
#include <string>

#include "meta.h"
/*!
  \brief
    Config class reads in properties from a file
*/
class Config
{
public:
    /*!
      \brief
        Reads in the config file and sets parameters and stores them in properties. If called twice it will
        keep old porperties, properties of the same name will be over written
      
      \param name
        The name of the config file to load
    */
		void Init(const char *name = "config.txt");
		/*!
		  \brief
		    The properties
		*/
		std::unordered_map<std::string, VoidWrapper> properties;
};
#endif