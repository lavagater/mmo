/*!
  \brief
    Each table is a seperate file, each table has a header which describes what data is in the table, 
    the header is first thing is an unsigned which represents the number of fields/rows 
    then for each row there is an unsigned representing the size in bytes that the row element is.
    then there is an unsigned which represents the number or objects/column.

    The header size is 2 * sizeof(unsigned) + num_rows * sizeof(unsigned)
    and example header could look like
    3 4 4 8 74
    which would mean there are 3 rows first row is 4 bytes second row is 4 bytes and third row is 8 bytes and there is 74 columns
    the files are stored in bianary.
  \author
    Wyatt Lavigueure
  \date
    6/20/2017
*/
#include <vector>
#include <fstream>
/*!
  \brief
    Each databse class just handles one table, this is to try and increase scaling of the database
*/
class Database
{
public:
  /*!
    \brief
      Opens the file for table_name table and loads in the data, if the table is unable to be opened nothing happens
    \param table_name
      The name of the table for the database to open
  */
  Database(const char *table_name);
  /*!
    \brief
      Creates a new table called table_name with rows given by rows,if there was already a table with the given name it is replaced
      and all previous data is lost
    \param table_name
      The name of the table for the database to open
    \param rows
      the size in bytes of each row
  */
  Database(const char *table_name, std::vector<unsigned> rows);
  /*!
    \brief
      Gets object/column corisponding to id from the table, returns the data in a tightly packet format(no pad bytes)
    \param id
      which object/column in the database to get data from
    \param data
      a char pointer, will be pointed at new memory allocated with new and has to be deleted
    \return
      the size in bytes put into data
  */
  unsigned Get(unsigned id, char *data);
  /*!
    \brief
      Gets a speciic row from an object/column from the table, returns the data in a tightly packet format(no pad bytes)
    \param id
      which object/column in the database to get data from
    \param row
      the row in the table
    \param data
      a char pointer, will be pointed at new memory allocated with new and has to be deleted
    \return
      the size of bytes put into data
  */
  unsigned Get(unsigned id, unsigned row, char *data);
  /*!
    \brief
      Sets the value of a row for a specific column
    \param id
      which object/column in the database
    \param row
      the row in the table
    \param data
      raw data to copy into the database
  */
  void Set(unsigned id, unsigned row, const void *data);
  /*!
    \brief
      Creates a new object in the data base and increases the size, default value is garbage 
    \return
      The id of the object just created
  */
  int Create();
  /*!
    \brief
      Gets all the id's of the items with the given value
    \param row
      The row to search
    \param value
      The value to search for
    \return
      an array of all the id's that have the given value, sorted in ascending order
  */
  std::vector<unsigned> Find(unsigned row, char *value);
  /*!
    \brief
      number of objects in file, cannot call Get with an id greater than size, if Set is called with an id greater
  */
  unsigned size;
private:
  //the file on harddrive
  std::fstream file;
  //the rows
  std::vector<unsigned> rows;
  //the size of the object(sum of every entry in rows)
  unsigned object_size;
  //writes the size to the file
  void UpdateSize();
};