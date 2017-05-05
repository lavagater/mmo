#include <fstream>
#include <iostream>

#include "config.h"

namespace config
{
	std::unordered_map<std::string, VoidWrapper> properties;

	void Init(const char *name)
	{
		std::ifstream file(name);
		while(!file.eof())
		{
			std::string line;
			std::getline(file, line);
			int index = line.find_first_of(":");
			if (index < 0)
				continue;
			std::string name = line.substr(0, index);
			//remove trailing spaces from name
			while (name[name.length()-1] == ' ')
			{
				name = name.substr(0, name.length()-1);
			}
			std::string value = line.substr(index+1);
			//remove the leading spaces
			while(value[0] == ' ')
			{
				value = value.substr(1);
			}
			properties[name] = StringToValue(value.c_str());
		}
	}
}