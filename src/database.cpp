

#include "database.h"

Database::Database(char *table_name) : file(table_name, std::ios_base::binary | std::ios_base::in | std::ios_base::out), size(0), rows()
{
  //set the rows
  unsigned num_rows;
  file.read(reinterpret_cast<char*>(&num_rows), sizeof(unsigned));
  for (unsigned i = 0; i < num_rows; ++i)
  {
    unsigned row_size;
    file.read(reinterpret_cast<char*>(&row_size), sizeof(unsigned));
    rows.push_back(row_size);
  }
  //set the size
  file.read(reinterpret_cast<char*>(&size), sizeof(unsigned));
  //save the object size
  for (unsigned i = 0; i < rows.size(); ++i)
  {
    object_size += rows[i];
  }
}

void Database::UpdateSize()
{
  //seek file to where the size goes
  file.seekg((1+rows.size()) * sizeof(unsigned));
  file.write(reinterpret_cast<char*>(&size), sizeof(unsigned));
}

Database::Database(char *table_name, std::vector<unsigned> rows) :file(table_name, std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc), size(0), rows(rows)
{
  //write the rows to the file
  unsigned num_rows = rows.size();
  file.write(reinterpret_cast<char*>(&num_rows), sizeof(unsigned));
  for (unsigned i = 0; i < num_rows; ++i)
  {
    unsigned row_size = rows[i];
    file.write(reinterpret_cast<char*>(&row_size), sizeof(unsigned));
  }
  //write the size
  UpdateSize();
  //save the object size
  for (unsigned i = 0; i < rows.size(); ++i)
  {
    object_size += rows[i];
  }
}

char *Database::Get(unsigned id)
{
  //allocate memory to return
  char *res = new char[object_size];
  //go to the spot in the file, (2 + rows.size()) * sizeof(unsigned) is the size of the header
  file.seekg((2 + rows.size()) * sizeof(unsigned) + id * object_size);
  //read the entire object
  file.read(res, object_size);
  return res;
}

char *Database::Get(unsigned id, unsigned row)
{
  //find out how far into the object to get to the row we want
  unsigned split_size = 0;
  for (unsigned i = 0; i < row; ++i)
  {
    split_size += rows[i];
  }
  //allocate memory to return
  char *res = new char[rows[row]];
  //go to the spot in the file, (2 + rows.size()) * sizeof(unsigned) is the size of the header
  file.seekg((2 + rows.size()) * sizeof(unsigned) + id * object_size + split_size);
  //read just the specified row
  file.read(res, rows[row]);
  return res;
}

void Database::Set(unsigned id, unsigned row, const void *data)
{
  //find out how far into the object to get to the row we want
  unsigned split_size = 0;
  for (unsigned i = 0; i < row; ++i)
  {
    split_size += rows[i];
  }
  //go to the spot in the file, (2 + rows.size()) * sizeof(unsigned) is the size of the header
  file.seekg((2 + rows.size()) * sizeof(unsigned) + id * object_size + split_size);
  //write the data
  file.write(reinterpret_cast<const char*>(data), rows[row]);
  //if we are edting an object that is past size we need to update size
  if (size < id)
  {
    size = id;
    UpdateSize();
  }
}