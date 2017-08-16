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
  std::vector<unsigned> rows = { 4, 1};
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
  //put an element in each database
  db_small.Create();
  db_medium.Create();
  db_large.Create();
  //a buffer for the database functions
  char buffer[1000];
  //randomize buffer
  for (int i = 0; i < 1000;++i)
    buffer[i] = rand();
  //variables for timing
  std::chrono::high_resolution_clock::time_point timer;
  double time_spent;

  //test speed of small database nearly empty
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 100000; ++i)
  {
    db_small.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "small databse one item " << time_spent / 100000.0 << " seconds" << std::endl;

  //test speed of medium database nearly empty
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 100000; ++i)
  {
    db_medium.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "medium databse one item " << time_spent / 100000.0 << " seconds" << std::endl;

  //test speed of large database nearly empty
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 100000; ++i)
  {
    db_large.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "large databse one item " << time_spent / 100000.0 << " seconds" << std::endl;

  //add more items
  for (int i = 0; i < 100; ++i)
  {
    db_small.Create();
    db_medium.Create();
    db_large.Create();
  }

  //test speed of small database with 101 items
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 10000; ++i)
  {
    db_small.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "small databse 101 items " << time_spent / 10000.0 << " seconds" << std::endl;

 //test speed of medium database with 101 items
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 10000; ++i)
  {
    db_medium.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "medium databse 101 items " << time_spent / 10000.0 << " seconds" << std::endl;

  //test speed of large database with 101 items
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 10000; ++i)
  {
    db_large.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "large databse 101 items " << time_spent / 10000.0 << " seconds" << std::endl;

  //add a lot more items
  for (int i = 0; i < 10000; ++i)
  {
    db_small.Create();
    db_medium.Create();
    db_large.Create();
  }

  //test speed of small database with 10101 items
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; ++i)
  {
    db_small.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "small databse 10101 items " << time_spent / 1000.0 << " seconds" << std::endl;

  //test speed of medium database with 10101 items
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; ++i)
  {
    db_medium.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "medium databse 10101 items " << time_spent / 1000.0 << " seconds" << std::endl;

  //test speed of large database with 10101 items
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; ++i)
  {
    db_large.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "large databse 10101 items " << time_spent / 1000.0 << " seconds" << std::endl;

  //just doing large databasefrom now on
  //add a lot lot more items, test how log it takes to create a million items
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000000; ++i)
  {
    db_large.Create();
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "created a million items " << time_spent << " seconds" << std::endl;

  //test speed of large database with 1010101 items
  timer = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 100; ++i)
  {
    db_large.Find(0, buffer);
  }
  time_spent = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timer).count();
  std::cout << "large databse 1010101 items " << time_spent / 100.0 << " seconds" << std::endl;
}

int main()
{
  DatabaseFindTest();
}