/*!
  \brief
    In this file there is functions to time the speed of functions. 
*/
#include <iostream>
#include <chrono>
#include <vector>

#include "database.h"

/*
  \brief
    Tests the speed of database find
*/
void DatabaseFindTest()
{
  std::vector<int> rows = { 4, 1};
  //table with an integer and a char
  Database db_small("speed_test_small.tbl", rows);
  //add some more ints
  for (int i = 0; i < 30; ++i)
  {
    rows.push_back(4);
  }
  //table with integer a char and 30 integers
  Database db_medium("speed_test_medium.tbl", rows);
  //make a large table same as medium plus 500 bytes and 1000 bytes
  rows.push_back(500);
  rows.push_back(1000);
  Database db_large("speed_test_large.tbl", rows);
  std::chrono::high_resolution_clock::time_point timer = std::chrono::high_resolution_clock::now();

  double time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
}

int main()
{
  DatabaseFindTest();
}