/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  Used to read in config properties from a config file.
  Format of the config file is property name followed by any number of spces
  followed by a colon followed by any number of spaces then the value followed by
  any number of newlines
  example
  name : "bob"
  age:12

  is cool  :  true
  id : 0x7F7F

  For more examples see test_conf.txt


*/
#ifndef CONFIG_H
#define CONFIG_H
#include <unordered_map>
#include <string>

#include "meta.h"
namespace config
{
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
		    Global map that contains properties from config file
		*/
		extern std::unordered_map<std::string, VoidWrapper> properties;
}
#endif