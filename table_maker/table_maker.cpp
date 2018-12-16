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
    std::cout << argv[0] << " table_name value1(num_bytes) type(0=char,1=int,2=Unsigned int, 3=short,4=float,5=double,6=string) is_sorted(0,1) value2 type is_sorted value3 type is_sorted" << std::endl;
    return 1;
  }
  std::string tbl = argv[1] + std::string(".tbl");
  std::cout << "tabel name = " << tbl << std::endl;
  std::vector<unsigned> rows;
  std::vector<unsigned> types;
  std::vector<unsigned> sorted;
  for (int i = 2; i < argc; i += 3)
  {
    rows.push_back(atoi(argv[i]));
    types.push_back(atoi(argv[i+1]));
    sorted.push_back(atoi(argv[i+2]));
  }
  std::cout << "make databse file" << std::endl;
  //create the file
  Database db(tbl.c_str(), rows, types, sorted);
  return 0;
}