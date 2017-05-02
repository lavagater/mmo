#include <fstream>

#include "config.h"

namespace config
{
	void Init(const char *name)
	{
		std::ifstream file(name);
	}
}