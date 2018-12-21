/*!
  \author
    Wyatt Lavigueure
  \date
    9/26/2017
  \brief
    A proram to test the database. The program expects there two be databases running, one is the player database each object
    has HP which is a float, a level which is an int and an equided item which is a reference to an item. The second table is a table of items
    each item has a name which is a string and a damage which is a float.
*/
#include <vector>
#include <iostream>
#include "database.h" 
#include "query.h"

int main()
{
  std::cout << "Test cout" << std::endl;
  //load table to test on
  Database db("resources/accounts.tbl");
  std::cout << "made db" << std::endl;
  Query query(db);
  std::cout << "made query" << std::endl;
  //copy the script
  std::string arg = STRINGIZE(
    main(string username, string password)
    {
      print("Login\n");
      print(username, "\n");
      print(password, "\n");
      vector res = find(0, username);
      print(res, "\n");
      if (Size(res) == 0)
      {
        print("Not found");
        return int(-1);
      }
      string saved_pass = get(res[0], 1);
      print("saved password = ", saved_pass, "\n");
      if (saved_pass != password)
      {
        print("passwords dont match");
        return int(-1);
      }
      print("Script finished account id = ", res[0]);
      return int(res[0]);
    }
  );

  //add the arguments
  std::vector<Value> arguments;
  Value username;
  username.type = Types::String;
  username.m_string = "πτδδΣß";
  arguments.push_back(username);
  Value password;
  password.type = Types::String;
  password.m_string = "F44217A81173869E08671753C52553646FF5D95B";
  arguments.push_back(password);

  std::cout << "compile" << std::endl;
  Value returnValue;
  if (query.Compile(arg, arguments, returnValue))
  {
    std::cout << "returned = " << returnValue.m_int << std::endl;
    return 0;
  }
}