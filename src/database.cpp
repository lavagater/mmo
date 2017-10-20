#include <string.h>

#include "database.h"

Database::Database(const char *table_name) : size(0), num_ids(0), rows(), 
                                             file(table_name, std::ios_base::binary | std::ios_base::in | std::ios_base::out),
                                             reusable_ids(), object_size(0)
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
  //set the num_ids
  file.read(reinterpret_cast<char*>(&num_ids), sizeof(int));

  //save the object size
  for (unsigned i = 0; i < rows.size(); ++i)
  {
    object_size += rows[i];
  }

  //set the local copy of the ids
  file.seekg((3 + rows.size()) * sizeof(unsigned) + size * object_size + object_size);
  for (int i = 0; i < num_ids; ++i)
  {
    unsigned temp;
    file.read(reinterpret_cast<char*>(&temp), sizeof(temp));
    reusable_ids.push_back(temp);
  }
}

void Database::UpdateSize()
{
  //seek file to where the size goes
  file.seekg((1+rows.size()) * sizeof(unsigned));
  file.write(reinterpret_cast<char*>(&size), sizeof(size));
}
void Database::UpdateIds()
{
  //seek file to where the size goes
  file.seekg((2+rows.size()) * sizeof(unsigned));
  file.write(reinterpret_cast<char*>(&num_ids), sizeof(num_ids));
}

Database::Database(const char *table_name, std::vector<unsigned> rows) : size(0), num_ids(0), rows(rows),
                                                                         file(table_name, std::ios_base::binary | 
                                                                         std::ios_base::in | std::ios_base::out | 
                                                                         std::ios_base::trunc), reusable_ids(), object_size(0)
{
  //write the rows to the file
  unsigned num_rows = rows.size();
  file.write(reinterpret_cast<char*>(&num_rows), sizeof(num_rows));
  for (unsigned i = 0; i < num_rows; ++i)
  {
    unsigned row_size = rows[i];
    file.write(reinterpret_cast<char*>(&row_size), sizeof(row_size));
  }
  //write the size
  UpdateSize();
  //write the number of ids
  UpdateIds();
  for (unsigned i = 0; i < rows.size(); ++i)
  {
    object_size += rows[i];
  }
}

unsigned Database::Get(unsigned id, char *&data)
{
  if (num_ids > 1000)
  {
    abort();
  }
  data = new char[object_size];
  //go to the spot in the file, (3 + rows.size()) * sizeof(unsigned) is the size of the header
  file.seekg((3 + rows.size()) * sizeof(unsigned) + id * object_size);
  //read the entire object
  file.read(data, object_size);
  return object_size;
}

unsigned Database::Get(unsigned id, unsigned row, char *&data)
{
  //find out how far into the object to get to the row we want
  unsigned split_size = 0;
  for (unsigned i = 0; i < row; ++i)
  {
    split_size += rows[i];
  }
  //allocate memory to return
  data = new char[rows[row]];
  //go to the spot in the file, (3 + rows.size()) * sizeof(unsigned) is the size of the header
  file.seekg((3 + rows.size()) * sizeof(unsigned) + id * object_size + split_size);
  //read just the specified row
  file.read(data, rows[row]);
  return rows[row];
}

void Database::Set(unsigned id, unsigned row, const void *data)
{
  //find out how far into the object to get to the row we want
  unsigned split_size = 0;
  for (unsigned i = 0; i < row; ++i)
  {
    split_size += rows[i];
  }
  //go to the spot in the file, (3 + rows.size()) * sizeof(unsigned) is the size of the header
  file.seekg((3 + rows.size()) * sizeof(unsigned) + id * object_size + split_size);
  //write the data
  file.write(reinterpret_cast<const char*>(data), rows[row]);
}
int Database::Create()
{
  if (num_ids > 0)
  {
    //update the number of ids in the list
    num_ids -= 1;
    unsigned id = reusable_ids[num_ids];
    reusable_ids.pop_back();
    UpdateIds();
    return id;
  }
  else
  {
    unsigned id = size;
    //seek past the spot in the file so that it can be read from
    file.seekg((3 + rows.size()) * sizeof(unsigned) + id * object_size + object_size);
    file.write("c", 1);
    //update the size
    size += 1;
    UpdateSize();
    //return the id
    return id;
  }
}
void Database::Delete(unsigned id)
{
  //check for double delete
  for (int i = 0; i < num_ids; ++i)
  {
    if (reusable_ids[i] == id)
    {
      return;
    }
  }
  //add the id to the end of the list of id's
  file.seekg((3 + rows.size()) * sizeof(unsigned) + size * object_size + object_size + num_ids * sizeof(unsigned));
  file.write(reinterpret_cast<char*>(&id), sizeof(id));
  //update the local list
  reusable_ids.push_back(id);
  num_ids += 1;
  UpdateIds();
}
std::vector<unsigned> Database::Find(unsigned row, char *value)
{
  std::vector<unsigned> res;
  //find out how far into the object to get to the row we want
  unsigned split_size = 0;
  for (unsigned i = 0; i < row; ++i)
  {
    split_size += rows[i];
  }
  char *buffer = new char[rows[row]];
  for (unsigned i = 0; i < size; ++i)
  {
    file.seekg((3 + rows.size()) * sizeof(unsigned) + i * object_size + split_size);
    file.read(buffer, rows[row]);
    if (memcmp(buffer, value, rows[row]) == 0)
    {
      res.push_back(i);
    }
  }
  delete [] buffer;
  //make sure none of the results are deleted
  for (int i = 0; i < num_ids; ++i)
  {
    for (unsigned j = 0; j < res.size(); ++j)
    {
      if (reusable_ids[i] == res[j])
      {
        res.erase(res.begin()+j);
        break;
      }
    }
  }
  return res;
}
void Database::flush()
{
  file.flush();
}