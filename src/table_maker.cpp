/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   6/24/2017
\brief  
  Helper executable to make table.

*/
/*****************************************************************************/
#include <iostream>
#include <vector>

#include "database.h"

int main(int argc, char **argv)
{
  if (argc == 1)
  {
    std::cout << "usage:" << std::endl;
    std::cout << argv[0] << " table_name value1 value2 value3" << std::endl;
    return 1;
  }
  std::string tbl = argv[1] + std::string(".tbl"); 
  std::vector<unsigned> rows;
  for (int i = 2; i < argc; ++i)
  {
    rows.push_back(atoi(argv[i]));
  }
  //create the file
  Database db(tbl.c_str(), rows);
  return 0;
}