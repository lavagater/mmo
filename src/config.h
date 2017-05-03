/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  Used to read in config properties from a config file

*/
#include <unordered_map>
#include <string>
namespace config
{
    /*!
      \brief
        Reads in the config file and sets parameters
      
      \param name
        The name of the config file to load
    */
		void Init(const char *name = "config.txt");
		//std::unordered_map<std::string, Param> 
}