/*!
\brief
Each table is a seperate file, each table has a header which describes what data is in the table,
the header is first thing is an unsigned which represents the number of fields/rows
then for each row there is an unsigned representing the size in bytes that the row element is.
then there is an unsigned which represents the number or objects/columns, then there is an unsigned for the number
of deleted empty objects/columns.

The header size is 3 * sizeof(unsigned) + num_rows * sizeof(unsigned)
and example header could look like
3 4 4 8 74 2
which would mean there are 3 rows first row is 4 bytes second row is 4 bytes and third row is 8 bytes and there is 74 columns
and 2 of the columns/objects are deleted and will be replaced by the next create call
the files are stored in bianary.

Note id 0 is reserved for null.
\author
Wyatt Lavigueure
\date
6/20/2017
*/
#include <vector>
#include <unordered_map>
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
  \param types
  the types of the rows
  \param sorted
  if the property is sorted or not
  */
  Database(const char *table_name, std::vector<unsigned> rows, std::vector<unsigned> types, std::vector<unsigned> sorted);
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
  unsigned Get(unsigned id, char *&data);
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
  unsigned Get(unsigned id, unsigned row, char *&data);
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
  Reuse deleted object memory if exists otherwise will create a new spot in the database and increase the size.
  The size dose not increase if memory is reused.
  \return
  The id of the object just created
  */
  int Create();
  /*!
  \brief
  Puts a object/column's memory up for grabs. Does change the size of the database just increases num_ids, and adds the given id
  to the list of reuseable ids. If the same object is deleted twice the second delete has no effect
  \param id
  The id of the object/colum to remove
  */
  void Delete(unsigned id);
  /*!
  \brief
  Gets all the id's of the items with the given value, wont return objects that have been deleted, can only find sorted properties
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
  Gets the id's of the items with the given value range, wont return objects that have been deleted, can only find sorted properties
  \param row
  The row to search
  \param min
  The minimum value to search for inclusive
  \param max
  The maximum value to search for inclusive
  \param num_results
  The number of hits to return, -1 is all
  \param is_smallest
  The direction to search from, if true we look for the top num_results smallest values
  if false then we look for the largest num_result values. i.e first ten values is true, last ten values is false
  \return
  an array of num_results the id's that have the given value. the returned id's are sorted by the value searched for
  so the first index will be the smallest value if is_smallest is true, the first value will be the largest is false
  */
  std::vector<unsigned> Find(unsigned row, char *min, char *max, int num_results, bool is_smallest);
  /*!
  \brief
  Finds the object with the largest value in the given row
  \param row
  The property to search for
  \return
  The id of the object
  */
  unsigned FindLargest(unsigned row);
  /*!
  \brief
  Finds the object with the smallest value in the given row
  \param row
  The property to search for
  \return
  The id of the object
  */
  unsigned FindSmallest(unsigned row);
  /*!
  \brief
  makes graphviz script out of the skip list to be visualized with grah viz
  \param row
  Which properties skip list to look at, must be a sorted property
  \return
  The string with the graph viz script
  */
  std::string ToGraphViz(unsigned row);
  /*!
  \brief
  total number of objects in file including deleted objects. To get the total number of active objects do
  size - num_ids. cannot call Get with an id greater than size
  */
  unsigned size;
  /*!
  \brief
  This is the number of ids in the list of reusable ids. When an object is deleted from the database, that object is still there in memory
  and its id is put at the end of the list to be reused. num_ids is the number of objects in the database that are not being used
  */
  int num_ids;
  /*!
  \brief
  The layout of the object. Each element in the vector corrisponds to an attribute of the object, the value of the element is the size
  in bytes of the attribute
  */
  std::vector<unsigned> rows;
  /*!
  \brief
  each element in rows there is a corrisponding element in types which describes what type the data is(int, float, double, data) from the type enum
  */
  std::vector<unsigned> types;
  /*!
  \brief
  each element in rows there is a corrisponding element in is_sorted which is wther there is a skip list made for this property
  */
  std::vector<unsigned> sorted;
  /*!
  \brief
  Flush stream. forcing the internal buffers to be written to the file
  */
  void flush();
  /*!
  \brief
  The different types of data that can be in the database
  */
  enum Types
  {
    Char,
    Integer,
    Unsigned,
    Short,
    Float,
    Double,
    String,
    num
  };
private:
  unsigned CreateNode(unsigned row);
  void RemoveNode(unsigned node, unsigned row);
  void DeleteNode(unsigned node, unsigned row);
  bool InsertNode(unsigned node, unsigned row, const char *data, unsigned id);
  //will look for the exact node, if not found will return the node one smaller
  unsigned FindNode(unsigned row, const char *data, unsigned id);
  unsigned AddNode(unsigned row, const char *data, unsigned id);
  int Compare(unsigned row, const char *lhs, const char*rhs);
  //puts the largest value the type can be into data
  //sizes are hard coded
  void MakeLargest(unsigned row, char *data);
  //update the skip list header in file
  void UpdateSkipSize(unsigned row);
  void UpdateSkipFreeList(unsigned row);
  void UpdateSkipMaxLevel(unsigned row);
  //the main file on harddrive
  std::fstream file;
  //the skip_lists for the sorted properties
  //note, the head node is at position sizeof(unsigned)*3
  //the tail is at position sizeof(unsigned)*3+(sizeof(unsigned)*(2 + MAXLVL) + sizeof(int) + rows[i])
  std::vector<std::fstream> skip_lists;
  //the id of the end of the skip_list file
  std::vector<unsigned> skip_size;
  //the free list for each skip_list
  std::vector<unsigned> free_list;
  //the highest value max_level can get to, the number of levels on head and tail. actual level will never get this high.
  #define MAXLVL 64
  //the highest level in skip_list
  std::vector<int> max_level;
  //a local copy of the reusable ids on the database
  std::vector<unsigned> reusable_ids;
  //the size of the object(sum of every entry in rows)
  unsigned object_size;
  //writes the size to the file
  void UpdateSize();
  //writes the num_ids to the file
  void UpdateIds();
  //maps a file to its file name
  std::unordered_map<std::fstream*, std::string> file_names;
};