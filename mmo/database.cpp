#include <cmath>
#include <string.h>
#include <string>
#include <sstream>
#include <float.h>
#include <algorithm>

#include "logger.h"
#include "database.h"
#include "types.h"

Database::Database(const char *table_name) : size(0), num_ids(0), rows(),
file(table_name, std::ios_base::binary | std::ios_base::in | std::ios_base::out),
reusable_ids(), object_size(0)
{
	if (file.peek() == std::ifstream::traits_type::eof())
	{
		LOGW("Database file empty");
	}
	file_names[&file] = table_name;
	//set the rows
	unsigned num_rows;
	file.read(reinterpret_cast<char*>(&num_rows), sizeof(unsigned));
  LOG("Database has " << num_rows << " rows");

	for (unsigned i = 0; i < num_rows; ++i)
	{
		//size
		unsigned row_size;
		file.read(reinterpret_cast<char*>(&row_size), sizeof(unsigned));
    LOG("row[" << i << "] = " << row_size);

		rows.push_back(row_size);
		//type
		unsigned row_type;
		file.read(reinterpret_cast<char*>(&row_type), sizeof(unsigned));
    LOG("row[" << i << "] type = " << row_type);


		types.push_back(row_type);
		//is_sorted
		unsigned row_is_sorted;
		file.read(reinterpret_cast<char*>(&row_is_sorted), sizeof(unsigned));
    LOG("row[" << i << "] sorted = " << row_type);


		sorted.push_back(row_is_sorted);

		//skip_list
		//add default values
		skip_lists.push_back(std::fstream());
		skip_size.push_back(0);
		free_list.push_back(0);
		max_level.push_back(0);

		//if its sorted then use actual values
		if (row_is_sorted)
		{
			std::string temp_name = table_name;
			temp_name += std::to_string(i);
			skip_lists[i].open(temp_name, std::ios_base::binary | std::ios_base::in | std::ios_base::out);
			skip_lists[i].read(reinterpret_cast<char*>(&skip_size[i]), sizeof(unsigned));


			skip_lists[i].read(reinterpret_cast<char*>(&free_list[i]), sizeof(unsigned));


			skip_lists[i].read(reinterpret_cast<char*>(&max_level[i]), sizeof(unsigned));

			//save the file name
			file_names[&skip_lists[i]] = temp_name;
		}
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
	file.seekg((3 + rows.size() * 3) * sizeof(unsigned) + size * object_size + object_size);
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
	file.seekp((1 + rows.size() * 3) * sizeof(unsigned));
	file.write(reinterpret_cast<char*>(&size), sizeof(size));
}
void Database::UpdateIds()
{
	//seek file to where the size goes
	file.seekp((2 + rows.size() * 3) * sizeof(unsigned));
	file.write(reinterpret_cast<char*>(&num_ids), sizeof(num_ids));
}

Database::Database(const char *table_name, std::vector<unsigned> rows, std::vector<unsigned> types, std::vector<unsigned> sorted) :
	size(1), num_ids(0), rows(rows), types(types), sorted(sorted),
	file(table_name, std::ios_base::binary |
		std::ios_base::in | std::ios_base::out |
		std::ios_base::trunc), reusable_ids(), object_size(0)
{
	file_names[&file] = table_name;
	//write the rows to the file
	unsigned num_rows = rows.size();
	file.write(reinterpret_cast<char*>(&num_rows), sizeof(num_rows));
	for (unsigned i = 0; i < num_rows; ++i)
	{
		//rows
		unsigned row_size = rows[i];
		file.write(reinterpret_cast<char*>(&row_size), sizeof(row_size));
		//types
		unsigned row_type = types[i];
		file.write(reinterpret_cast<char*>(&row_type), sizeof(row_type));
		//sorted
		unsigned row_sorted = sorted[i];
		file.write(reinterpret_cast<char*>(&row_sorted), sizeof(row_sorted));

		//skip_list
		//add default values
		skip_lists.push_back(std::fstream());
		skip_size.push_back(0);
		free_list.push_back(0);
		max_level.push_back(0);
		if (row_sorted)
		{
			std::string temp_name = table_name;
			temp_name += std::to_string(i);
			//save the file name
			file_names[&skip_lists[i]] = temp_name;
			//make the skip list file
			skip_lists[i].open(temp_name, std::ios_base::binary |
				std::ios_base::in | std::ios_base::out |
				std::ios_base::trunc);

			//head node and tail node are special
			skip_lists[i].seekp(sizeof(unsigned) * 3 + sizeof(unsigned) + rows[i]);
			int level = MAXLVL;
			skip_lists[i].write(reinterpret_cast<char *>(&level), sizeof(level));
			unsigned tail = sizeof(unsigned) * 3 + (sizeof(unsigned)*(2 + MAXLVL) + sizeof(int) + rows[i]);
			//set previous to null
			unsigned null_id = 0;
			skip_lists[i].write(reinterpret_cast<char *>(&null_id), sizeof(null_id));
			for (int j = 0; j < MAXLVL; ++j)
			{
				skip_lists[i].write(reinterpret_cast<char *>(&tail), sizeof(tail));
			}

			//set tail data so its the largest possible node so no node can be inserted after it
			skip_lists[i].seekp(sizeof(unsigned) * 3 + (sizeof(unsigned)*(2 + MAXLVL) + sizeof(int) + rows[i]));
			//tail id is max_unsigned
			unsigned id = unsigned(-1);
			//tail data is largest
			char *data = new char[rows[i]];
			MakeLargest(i, data);
			skip_lists[i].write(reinterpret_cast<char *>(&id), sizeof(id));
			skip_lists[i].write(data, rows[i]);
			skip_lists[i].write(reinterpret_cast<char *>(&level), sizeof(level));
			//set tail previous to head
			unsigned head = sizeof(unsigned) * 3;
			skip_lists[i].write(reinterpret_cast<char *>(&head), sizeof(head));
			for (int j = 0; j < MAXLVL; ++j)
			{
				skip_lists[i].write(reinterpret_cast<char *>(&null_id), sizeof(null_id));
			}
			delete[] data;

			// 3 unsigneds for header, 2 max level nodes for head and tail
			skip_size[i] = sizeof(unsigned) * 3 + (sizeof(unsigned)*(2 + MAXLVL) + sizeof(int) + rows[i]) * 2;
			//write the header to file
			UpdateSkipSize(i);
			UpdateSkipFreeList(i);
			UpdateSkipMaxLevel(i);
		}
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
	data = new char[object_size];
	//go to the spot in the file, (3 + rows.size()*3) * sizeof(unsigned) is the size of the header
	file.seekg((3 + rows.size() * 3) * sizeof(unsigned) + id * object_size);
	//read the entire object
	file.read(data, object_size);


	return object_size;
}

unsigned Database::Get(unsigned id, unsigned row, char *data, unsigned size)
{
	if (size < rows[row])
	{
		LOGW("Size not big enough");
		return 0;
	}
	//find out how far into the object to get to the row we want
	unsigned split_size = 0;
	for (unsigned i = 0; i < row; ++i)
	{
		split_size += rows[i];
	}
	//go to the spot in the file, (3 + rows.size()*3) * sizeof(unsigned) is the size of the header
	file.seekg((3 + rows.size() * 3) * sizeof(unsigned) + id * object_size + split_size);
	//read just the specified row
	file.read(data, rows[row]);


	return rows[row];
}

unsigned Database::Get(unsigned id, unsigned row, char *&data)
{
	//allocate memory to return
	data = new char[rows[row]];
	return Get(id, row, data, rows[row]);
}

unsigned Database::Get(unsigned id, unsigned start_row, unsigned end_row, char *data, unsigned size)
{
	unsigned offset = 0;
	for (unsigned i = start_row; i <= end_row; ++i)
	{
		Get(id, i, data + offset, size - offset);
		offset += rows[i];
	}
	LOG("Database get faggot " << ToHexString(data, size) << std::endl);
	return offset;
}
unsigned Database::Get(unsigned id, unsigned start_row, unsigned end_row, char *&data)
{
	unsigned size = 0;
	for (unsigned i = start_row; i <= end_row; ++i)
	{
		size += rows[i];
	}
	data = new char[size];
	return Get(id, start_row, end_row, data, size);
}

void Database::Set(unsigned id, unsigned row, const void *data)
{
	//find out how far into the object to get to the row we want
	unsigned split_size = 0;
	for (unsigned i = 0; i < row; ++i)
	{
		LOG(i);
		split_size += rows[i];
	}
	//go to the spot in the file, (3 + rows.size()*3) * sizeof(unsigned) is the size of the header
	file.seekg((3 + rows.size() * 3) * sizeof(unsigned) + id * object_size + split_size);
	//get what the data was
	char *old_data = new char[rows[row]];
	file.read(old_data, rows[row]);
	file.seekp((3 + rows.size() * 3) * sizeof(unsigned) + id * object_size + split_size);
	//write the data
	file.write(reinterpret_cast<const char*>(data), rows[row]);

	//update the skip list
	if (sorted[row])
	{
		LOG("got here 1");
		unsigned node = FindNode(row, old_data, id);
		LOG("got here 2");
		RemoveNode(node, row);
		LOG("got here 3");
		skip_lists[row].seekp(node + sizeof(unsigned));
		skip_lists[row].write(reinterpret_cast<const char*>(data), rows[row]);
		InsertNode(node, row, reinterpret_cast<const char*>(data), id);
		LOG("got here 4");
	}
	delete[] old_data;
}
void Database::Set(unsigned id, unsigned start_row, unsigned end_row, const void *data)
{
	unsigned offset = 0;
	for (unsigned i = start_row; i <= end_row; ++i)
	{
	  LOG(i);
		Set(id, i, (char*)data + offset);
		offset += rows[i];
	}
	LOG("Database set faggot " << ToHexString((char*)data, offset) << std::endl);
}
unsigned Database::Create()
{
	unsigned id;
	if (num_ids > 0)
	{
		//update the number of ids in the list
		num_ids -= 1;
		id = reusable_ids[num_ids];
		reusable_ids.pop_back();
		UpdateIds();
	}
	else
	{
		id = size;
		//seek past the spot in the file so that it can be read from
		file.seekp((3 + rows.size() * 3) * sizeof(unsigned) + id * object_size + object_size);
		file.write("c", 1);
		//update the size
		size += 1;
		UpdateSize();
	}
	LOG("New database entry id = " << id);
	//for each sorted property set the value to random
	unsigned split = 0;
	for (unsigned i = 0; i < rows.size(); ++i)
	{
		if (sorted[i])
		{
			char *data = new char[rows[i]];
			//set to random
			for (unsigned j = 0; j < rows[i]; ++j)
			{
				data[j] = rand();
			}
			//write the random value
			file.seekp((3 + rows.size() * 3) * sizeof(unsigned) + id * object_size + split);
			file.write(data, rows[i]);
			//create a node in skip list for the value
			AddNode(i, data, id);
			delete[] data;
		}
		split += rows[i];
	}
	return id;
}
void Database::Delete(unsigned id)
{
	//cant delete null
	if (id == 0)
	{
		return;
	}
	//check for double delete
	for (int i = 0; i < num_ids; ++i)
	{
		if (reusable_ids[i] == id)
		{
			return;
		}
	}
	//delete the nodes in the skip lists
	unsigned split = 0;
	for (unsigned i = 0; i < rows.size(); ++i)
	{
		if (sorted[i])
		{
			char *data = new char[rows[i]];
			file.seekg((3 + rows.size() * 3) * sizeof(unsigned) + id * object_size + split);
			file.read(data, rows[i]);


			unsigned node = FindNode(i, data, id);
			RemoveNode(node, i);
			DeleteNode(node, i);
		}
		split += rows[i];
	}
	//add the id to the end of the list of id's
	file.seekp((3 + rows.size() * 3) * sizeof(unsigned) + size * object_size + object_size + num_ids * sizeof(unsigned));
	file.write(reinterpret_cast<char*>(&id), sizeof(id));
	//update the local list
	reusable_ids.push_back(id);
	num_ids += 1;
	UpdateIds();
}
std::vector<unsigned> Database::Find(unsigned row, char *value)
{
	return Find(row, value, value, -1, true);
}
std::vector<unsigned> Database::Find(unsigned row, char *min, char *max, int num_results, bool is_smallest)
{
	std::vector<unsigned> res;
	if (sorted[row] == 0)
	{
		LOGW("Trying to find a non sorted value");
		return res;
	}
	char *buffer = new char[rows[row]];
	//getting the smallest we start at the smallest element and walk forward through the list
	if (is_smallest)
	{
		//find the first node
		unsigned start = FindNode(row, min, 0);
		skip_lists[row].seekg(start + sizeof(unsigned) * 2 + sizeof(int) + rows[row]);
		skip_lists[row].read(reinterpret_cast<char*>(&start), sizeof(start));
		//walk list until the value is bigger than max or reach num_results
		unsigned id;
		unsigned next;
		skip_lists[row].seekg(start);
		skip_lists[row].read(reinterpret_cast<char*>(&id), sizeof(id));
		skip_lists[row].read(buffer, rows[row]);
		skip_lists[row].seekg(start + sizeof(unsigned) * 2 + sizeof(int) + rows[row]);
		skip_lists[row].read(reinterpret_cast<char*>(&next), sizeof(next));
		while (Compare(row, buffer, max) <= 0 && next)
		{
			res.push_back(id);
			start = next;
			skip_lists[row].seekg(start);
			skip_lists[row].read(reinterpret_cast<char*>(&id), sizeof(id));
			skip_lists[row].read(buffer, rows[row]);
			skip_lists[row].seekg(start + sizeof(unsigned) * 2 + sizeof(int) + rows[row]);
			skip_lists[row].read(reinterpret_cast<char*>(&next), sizeof(next));
			unsigned temp;
			skip_lists[row].seekg(start + sizeof(unsigned) + sizeof(int) + rows[row]);
			skip_lists[row].read(reinterpret_cast<char*>(&temp), sizeof(temp));
			if (num_results > 0 && --num_results == 0)
			{
				break;
			}
		}
	}
	//is_smallest is false then we start at the biggest value and walk backwards
	else
	{
		//find the last node
		unsigned end = FindNode(row, max, unsigned(-2));
		//walk list backwards until the value is smaller than min or reach num_results
		unsigned id;
		unsigned prev;
		skip_lists[row].seekg(end);
		skip_lists[row].read(reinterpret_cast<char*>(&id), sizeof(id));
		skip_lists[row].read(buffer, rows[row]);
		skip_lists[row].seekg(end + sizeof(unsigned) + sizeof(int) + rows[row]);
		skip_lists[row].read(reinterpret_cast<char*>(&prev), sizeof(prev));
		while (Compare(row, buffer, min) >= 0 && prev)
		{
			res.push_back(id);
			end = prev;
			skip_lists[row].seekg(end);
			skip_lists[row].read(reinterpret_cast<char*>(&id), sizeof(id));
			skip_lists[row].read(buffer, rows[row]);
			skip_lists[row].seekg(end + sizeof(unsigned) + sizeof(int) + rows[row]);
			skip_lists[row].read(reinterpret_cast<char*>(&prev), sizeof(prev));
			if (num_results > 0 && --num_results == 0)
			{
				break;
			}
		}
	}
	delete [] buffer;
	return res;
}
std::vector<unsigned> Database::Find(std::vector<unsigned> row, std::vector<char*> min, std::vector<char*> max, int num_results, bool is_smallest)
{
	LOG("Finding range");
	std::vector<unsigned> res;
	if (row.size() == 0 || row.size() != min.size() || min.size() != max.size())
	{
		LOGW("Trying to find with incorrect row amounts");
		return res;
	}
	if (sorted[row[0]] == 0)
	{
		LOGW("Trying to find a non sorted value");
		return res;
	}
	//create a buffer to store the row[0] data
	char *buffer = new char[rows[row[0]]];
	//getting the smallest we start at the smallest element and walk forward through the list
	if (is_smallest)
	{
		//find the first node
		unsigned start = FindNode(row[0], min[0], 0);
		LOG("start node = " << start);
		skip_lists[row[0]].seekg(start + sizeof(unsigned) * 2 + sizeof(int) + rows[row[0]]);
		skip_lists[row[0]].read(reinterpret_cast<char*>(&start), sizeof(start));
		//walk list until the value is bigger than max or reach num_results
		unsigned id;
		unsigned next;
		skip_lists[row[0]].seekg(start);
		skip_lists[row[0]].read(reinterpret_cast<char*>(&id), sizeof(id));
		skip_lists[row[0]].read(buffer, rows[row[0]]);
		skip_lists[row[0]].seekg(start + sizeof(unsigned) * 2 + sizeof(int) + rows[row[0]]);
		skip_lists[row[0]].read(reinterpret_cast<char*>(&next), sizeof(next));
		while (Compare(row[0], buffer, max[0]) <= 0 && next)
		{
			//need to make sure all the other rows are matched as well
			bool isMatch = true;
			for (unsigned i = 1; i < row.size(); ++i)
			{
				char *temp;
				Get(id, row[i], temp);
				if (Compare(row[i], temp, min[i]) < 0 || Compare(row[i], temp, max[i]) > 0)
				{
					delete [] temp;
					LOG("match failed id = " << id << " row " << row[i] << " did not match");
					isMatch = false;
					break;
				}
				else
				{
					delete [] temp;
				}
			}
			if (isMatch)
			{
				res.push_back(id);
			}
			start = next;
			skip_lists[row[0]].seekg(start);
			skip_lists[row[0]].read(reinterpret_cast<char*>(&id), sizeof(id));
			skip_lists[row[0]].read(buffer, rows[row[0]]);
			skip_lists[row[0]].seekg(start + sizeof(unsigned) * 2 + sizeof(int) + rows[row[0]]);
			skip_lists[row[0]].read(reinterpret_cast<char*>(&next), sizeof(next));
			unsigned temp;
			skip_lists[row[0]].seekg(start + sizeof(unsigned) + sizeof(int) + rows[row[0]]);
			skip_lists[row[0]].read(reinterpret_cast<char*>(&temp), sizeof(temp));
			if (num_results > 0 && --num_results == 0)
			{
				break;
			}
		}
	}
	//is_smallest is false then we start at the biggest value and walk backwards
	else
	{
		//find the last node
		unsigned end = FindNode(row[0], max[0], unsigned(-2));
		//walk list backwards until the value is smaller than min or reach num_results
		unsigned id;
		unsigned prev;
		skip_lists[row[0]].seekg(end);
		skip_lists[row[0]].read(reinterpret_cast<char*>(&id), sizeof(id));
		skip_lists[row[0]].read(buffer, rows[row[0]]);
		skip_lists[row[0]].seekg(end + sizeof(unsigned) + sizeof(int) + rows[row[0]]);
		skip_lists[row[0]].read(reinterpret_cast<char*>(&prev), sizeof(prev));
		while (Compare(row[0], buffer, min[0]) >= 0 && prev)
		{
			//need to make sure all the other rows are matched as well
			bool isMatch = true;
			for (unsigned i = 1; i < row.size(); ++i)
			{
				char *temp;
				Get(id, row[i], temp);
				if (Compare(row[i], temp, min[i]) < 0 || Compare(row[i], temp, max[i]) > 0)
				{
					delete [] temp;
					isMatch = false;
					break;
				}
				else
				{
					delete [] temp;
				}
			}
			if (isMatch)
			{
				res.push_back(id);
			}
			end = prev;
			skip_lists[row[0]].seekg(end);
			skip_lists[row[0]].read(reinterpret_cast<char*>(&id), sizeof(id));
			skip_lists[row[0]].read(buffer, rows[row[0]]);
			skip_lists[row[0]].seekg(end + sizeof(unsigned) + sizeof(int) + rows[row[0]]);
			skip_lists[row[0]].read(reinterpret_cast<char*>(&prev), sizeof(prev));
			if (num_results > 0 && --num_results == 0)
			{
				break;
			}
		}
	}
	delete [] buffer;
	return res;
}
unsigned Database::FindLargest(unsigned row)
{
	unsigned tail = sizeof(unsigned) * 3 + (sizeof(unsigned)*(2 + MAXLVL) + sizeof(int) + rows[row]);
	//goto the tails previous pointer
	skip_lists[row].seekg(tail + sizeof(unsigned) + rows[row] + sizeof(int));
	unsigned prev;
	skip_lists[row].read(reinterpret_cast<char*>(&prev), sizeof(prev));
	skip_lists[row].seekg(prev);
	unsigned id;
	skip_lists[row].read(reinterpret_cast<char*>(&id), sizeof(id));
	return id;
}
unsigned Database::FindSmallest(unsigned row)
{
	unsigned head = sizeof(unsigned) * 3;
	//goto the tails previous pointer
	skip_lists[row].seekg(head + sizeof(unsigned) * 2 + rows[row] + sizeof(int));
	unsigned next;
	skip_lists[row].read(reinterpret_cast<char*>(&next), sizeof(next));
	skip_lists[row].seekg(next);
	unsigned id;
	skip_lists[row].read(reinterpret_cast<char*>(&id), sizeof(id));
	return id;
}
void Database::flush()
{
	file.flush();
}
void Database::UpdateSkipSize(unsigned row)
{
	skip_lists[row].seekp(0);
	skip_lists[row].write(reinterpret_cast<char*>(&skip_size[row]), sizeof(skip_size[row]));
}
void Database::UpdateSkipFreeList(unsigned row)
{
	skip_lists[row].seekp(sizeof(unsigned));
	skip_lists[row].write(reinterpret_cast<char*>(&free_list[row]), sizeof(free_list[row]));
}
void Database::UpdateSkipMaxLevel(unsigned row)
{
	skip_lists[row].seekp(sizeof(unsigned) * 2);
	skip_lists[row].write(reinterpret_cast<char*>(&max_level[row]), sizeof(max_level[row]));
}
int Database::Compare(unsigned row, const char *lhs, const char*rhs)
{
	switch (types[row])
	{
	case Char:
	{
		if (*lhs < *rhs)
		{
			return -1;
		}
		else if (*rhs < *lhs)
		{
			return 1;
		}
		else
		{
			return 0;
		}
		break;
	}
	case Short:
	{
		if (*reinterpret_cast<const short*>(lhs) < *reinterpret_cast<const short*>(rhs))
		{
			return -1;
		}
		else if (*reinterpret_cast<const short*>(rhs) < *reinterpret_cast<const short*>(lhs))
		{
			return 1;
		}
		else
		{
			return 0;
		}
		break;
	}
	case Integer:
	{
		if (*reinterpret_cast<const int*>(lhs) < *reinterpret_cast<const int*>(rhs))
		{
			return -1;
		}
		else if (*reinterpret_cast<const int*>(rhs) < *reinterpret_cast<const int*>(lhs))
		{
			return 1;
		}
		else
		{
			return 0;
		}
		break;
	}
	case Unsigned:
	{
		if (*reinterpret_cast<const unsigned*>(lhs) < *reinterpret_cast<const unsigned*>(rhs))
		{
			return -1;
		}
		else if (*reinterpret_cast<const unsigned*>(rhs) < *reinterpret_cast<const unsigned*>(lhs))
		{
			return 1;
		}
		else
		{
			return 0;
		}
		break;
	}
	case Float:
	{
		float left = *reinterpret_cast<const float*>(lhs);
		float right = *reinterpret_cast<const float*>(rhs);
		//nan values are considreeded the smallest
		if (std::isnan(left) || std::isnan(right))
		{
			if (std::isnan(left) && std::isnan(right))
				return memcmp(lhs, rhs, rows[row]);
			if (std::isnan(left))
				return -1;
			if (std::isnan(right))
				return 1;
		}
		if (left < right)
		{
			return -1;
		}
		else if (right < left)
		{
			return 1;
		}
		else
		{
			return 0;
		}
		break;
	}
	case Double:
	{
		double left = *reinterpret_cast<const double*>(lhs);
		double right = *reinterpret_cast<const double*>(rhs);
		//nan values are considreeded the smallest
		if (std::isnan(left) || std::isnan(right))
		{
			if (std::isnan(left) && std::isnan(right))
				return memcmp(lhs, rhs, rows[row]);
			if (std::isnan(left))
				return -1;
			if (std::isnan(right))
				return 1;
		}
		if (left < right)
		{
			return -1;
		}
		else if (right < left)
		{
			return 1;
		}
		else
		{
			return 0;
		}
		break;
	}
	case String:
	case Blob:
	{
		return memcmp(lhs, rhs, rows[row]);
		break;
	}
	}
	return 0;
}
void Database::MakeLargest(unsigned row, char *data)
{
	switch (types[row])
	{
	case Char:
	{
		*data = 127;
		break;
	}
	case Short:
	{
		*reinterpret_cast<short*>(data) = 32767;
		break;
	}
	case Integer:
	{
		*reinterpret_cast<int*>(data) = 2147483647;
		break;
	}
	case Unsigned:
	{
		*reinterpret_cast<int*>(data) = -1;
		break;
	}
	case Float:
	{
		*reinterpret_cast<float*>(data) = FLT_MAX;
		break;
	}
	case Double:
	{
		*reinterpret_cast<double*>(data) = DBL_MAX;
		break;
	}
	case String:
	case Blob:
	{
		memset(data, (unsigned char)(-1), rows[row]);
		break;
	}
	}
}
unsigned Database::CreateNode(unsigned row)
{
	if (free_list[row] == 0)
	{
		//figure out what level to make the new node
		int level = 1;
		double d = rand() / (double)RAND_MAX;

		while (level < MAXLVL && level <= max_level[row] && d < 0.5)
		{
			level += 1;
			d = rand() / (double)RAND_MAX;
		}
		if (level > max_level[row])
		{
			max_level[row] = level;
			UpdateSkipMaxLevel(row);
		}

		//add node at the very end of file
		skip_lists[row].seekp(skip_size[row] + sizeof(unsigned) + rows[row]);
		skip_lists[row].write(reinterpret_cast<char*>(&level), sizeof(level));
		unsigned ret = skip_size[row];
		//increase the size of the skip list
		skip_size[row] += sizeof(unsigned) * (2 + level) + sizeof(int) + rows[row];
		UpdateSkipSize(row);
		return ret;
	}
	else
	{
		//snag the first thing on the free list
		unsigned ret = free_list[row];
		// free_list = freelist->next
		skip_lists[row].seekg(free_list[row] + sizeof(unsigned) * 2 + sizeof(int) + rows[row]);
		skip_lists[row].read(reinterpret_cast<char*>(&free_list[row]), sizeof(free_list[row]));
		return ret;
	}
}
void Database::DeleteNode(unsigned node, unsigned row)
{
	//add it to the free list
	skip_lists[row].seekp(node + sizeof(unsigned) * 2 + sizeof(int) + rows[row]);
	skip_lists[row].write(reinterpret_cast<char*>(&free_list[row]), sizeof(free_list[row]));
	free_list[row] = node;
}
void Database::RemoveNode(unsigned node, unsigned row)
{
	//get node info
	unsigned id;
	char *data = new char[rows[row]];
	skip_lists[row].seekg(node);
	skip_lists[row].read(reinterpret_cast<char*>(&id), sizeof(id));
	skip_lists[row].read(data, rows[row]);
	int level = max_level[row];
	//set walker to the head
	unsigned walker = sizeof(unsigned) * 3;
	int my_level;
	skip_lists[row].seekg(node + sizeof(unsigned) * 2 + rows[row]);
	skip_lists[row].read(reinterpret_cast<char*>(&my_level), sizeof(my_level));
	//bufffer for later
	char *data_n = new char[rows[row]];
	while (level >= 0)
	{
		unsigned next;
		skip_lists[row].seekg(walker + rows[row] + sizeof(unsigned) * (2 + level) + sizeof(int));
		skip_lists[row].read(reinterpret_cast<char*>(&next), sizeof(next));
		skip_lists[row].seekg(next);
		unsigned id_n;
		skip_lists[row].read(reinterpret_cast<char*>(&id_n), sizeof(id_n));
		skip_lists[row].read(data_n, rows[row]);
		//use compare function that looks at the type of data
		int cmp = Compare(row, data_n, data);
		if (cmp == 0)
		{
			//compare ids
			if (id_n < id)
			{
				cmp = -1;
			}
			else if (id < id_n)
			{
				cmp = 1;
			}
		}
		if (cmp < 0)
		{
			walker = next;
		}
		else if (cmp > 0)
		{
			level -= 1;
		}
		else
		{
			//get node next pointer
			skip_lists[row].seekg(node + rows[row] + sizeof(unsigned)*(2 + level) + sizeof(int));
			unsigned my_next;
			skip_lists[row].read(reinterpret_cast<char*>(&my_next), sizeof(my_next));
			//set walkers next to be node next
			skip_lists[row].seekp(walker + rows[row] + sizeof(unsigned)*(2 + level) + sizeof(int));
			skip_lists[row].write(reinterpret_cast<char*>(&my_next), sizeof(my_next));
			//if this is level 0 then adjust the previous pointer as well
			if (level == 0)
			{
				//next points back to walker instead of node
				skip_lists[row].seekp(my_next + rows[row] + sizeof(unsigned) + sizeof(int));
				skip_lists[row].write(reinterpret_cast<char*>(&walker), sizeof(walker));
			}
			level -= 1;
		}

	}
	//delete the buffers
	delete[] data_n;
	delete[] data;
}

bool Database::InsertNode(unsigned node, unsigned row, const char *data, unsigned id)
{
	int level = max_level[row];
	//set walker to the head
	unsigned walker = sizeof(unsigned) * 3;
	int my_level;
	skip_lists[row].seekg(node + sizeof(unsigned) + rows[row]);
	skip_lists[row].read(reinterpret_cast<char*>(&my_level), sizeof(my_level));
	//subtract one to get the higest index
	my_level -= 1;

	//bufffer for later
	char *data_n = new char[rows[row]];
	while (level >= 0)
	{
		unsigned next;
		skip_lists[row].seekg(walker + rows[row] + sizeof(unsigned) * (2 + level) + sizeof(int));
		skip_lists[row].read(reinterpret_cast<char*>(&next), sizeof(next));
		skip_lists[row].seekg(next);
		unsigned id_n;
		skip_lists[row].read(reinterpret_cast<char*>(&id_n), sizeof(id_n));
		skip_lists[row].read(data_n, rows[row]);
		//use compare function that looks at the type of data
		int cmp = Compare(row, data_n, data);
		if (cmp == 0)
		{
			//compare ids
			if (id_n < id)
			{
				cmp = -1;
			}
			else if (id < id_n)
			{
				cmp = 1;
			}
		}
		if (cmp < 0)
		{
			walker = next;
			if (walker == sizeof(unsigned) * 3 + (sizeof(unsigned)*(2 + MAXLVL) + sizeof(int) + rows[row]))
			{
				//if we try to insert past the tail
				delete[] data_n;
				return false;
			}
		}
		else if (cmp > 0)
		{
			if (level == my_level)
			{
				//set the node to point at what walker is pointing at
				skip_lists[row].seekp(node + rows[row] + sizeof(unsigned)*(2 + level) + sizeof(int));
				skip_lists[row].write(reinterpret_cast<char*>(&next), sizeof(next));
				//set walker to point at node
				skip_lists[row].seekp(walker + rows[row] + sizeof(unsigned)*(2 + level) + sizeof(int));
				skip_lists[row].write(reinterpret_cast<char*>(&node), sizeof(node));
				//if were at the base level also hook up the previous pointers
				if (level == 0)
				{
					//next points back to node
					skip_lists[row].seekp(next + rows[row] + sizeof(unsigned) + sizeof(int));
					skip_lists[row].write(reinterpret_cast<char*>(&node), sizeof(node));
					//node points back to walker
					skip_lists[row].seekp(node + rows[row] + sizeof(unsigned) + sizeof(int));
					skip_lists[row].write(reinterpret_cast<char*>(&walker), sizeof(walker));
				}
				my_level -= 1;
			}
			//goto next level
			level -= 1;
		}
		else
		{
			//the thing that is being inserted already exists
			delete[] data_n;
			return false;
		}

	}
	//delete the buffer
	delete[] data_n;
	return true;
}
unsigned Database::FindNode(unsigned row, const char *data, unsigned id)
{
	LOG("FindNode row = " << row << " data = " << ToHexString(data, rows[row]));
	int level = max_level[row];
	//set walker to the head
	unsigned walker = sizeof(unsigned) * 3;

	//bufffer for later
	char *data_n = new char[rows[row]];
	while (level >= 0)
	{
		unsigned next;
		skip_lists[row].seekg(walker + rows[row] + sizeof(unsigned) * (2 + level) + sizeof(int));
		skip_lists[row].read(reinterpret_cast<char*>(&next), sizeof(next));
		skip_lists[row].seekg(next);
		unsigned id_n;
		skip_lists[row].read(reinterpret_cast<char*>(&id_n), sizeof(id_n));
		skip_lists[row].read(data_n, rows[row]);
		LOG("new id = " << id_n << " new data = " << ToHexString(data_n, rows[row]));
		//use compare function that looks at the type of data
		int cmp = Compare(row, data_n, data);
		if (cmp == 0)
		{
			//compare ids
			if (id_n < id)
			{
				cmp = -1;
			}
			else if (id < id_n)
			{
				cmp = 1;
			}
		}
		if (cmp < 0)
		{
			walker = next;
		}
		else if (cmp > 0)
		{
			//goto next level
			level -= 1;
		}
		else
		{
			//found it
			walker = next;
			break;
		}

	}
	//delete the buffer
	delete[] data_n;
	return walker;
}
unsigned Database::AddNode(unsigned row, const  char *data, unsigned id)
{
	unsigned new_node = CreateNode(row);

	skip_lists[row].seekp(new_node);
	skip_lists[row].write(reinterpret_cast<char*>(&id), sizeof(id));
	skip_lists[row].write(data, rows[row]);
	InsertNode(new_node, row, data, id);
	return new_node;
}

//helper for printing node names
static std::string name_node(unsigned id, unsigned file_id, unsigned level, char *data, unsigned row, Database &db)
{
	std::stringstream ss;
	ss << "\"id" + std::to_string(id) + "_" + std::to_string(file_id) + "_" + std::to_string(level) + "\\n";
	if (db.types[row] == Char)
		ss << *reinterpret_cast<char*>(data);
	if (db.types[row] == Short)
		ss << *reinterpret_cast<short*>(data);
	if (db.types[row] == Integer)
		ss << *reinterpret_cast<int*>(data);
	if (db.types[row] == Float)
		ss << *reinterpret_cast<float*>(data);
	if (db.types[row] == Double)
		ss << *reinterpret_cast<double*>(data);
	if (db.types[row] == Unsigned)
		ss << *reinterpret_cast<int*>(data);
	if (db.types[row] == String)
		ss.write("String", 6);
	if (db.types[row] == Blob)
		ss << ToHexString(data, db.rows[row]);
	ss << "\"";
	return ss.str();
}

int my_min(int lhs, int rhs)
{
	return (((lhs) < (rhs)) ? (lhs) : (rhs));
}

std::string Database::ToGraphViz(unsigned row)
{
	std::string graph = "digraph {rankdir=LR;";
	unsigned walker = sizeof(unsigned) * 3;

	//bufffer for later
	char *data_n = new char[rows[row]];
	char *data = new char[rows[row]];
	while (walker != 0)
	{
		//read the nodes level
		int level;
		skip_lists[row].seekg(walker + sizeof(unsigned) + rows[row]);
		skip_lists[row].read(reinterpret_cast<char*>(&level), sizeof(level));
		unsigned id;
		skip_lists[row].seekg(walker);
		skip_lists[row].read(reinterpret_cast<char*>(&id), sizeof(id));
		skip_lists[row].read(data, rows[row]);
		//add a connection for each level
		for (unsigned i = 0; i < (unsigned)my_min(level, max_level[row]); ++i)
		{
			unsigned next;
			skip_lists[row].seekg(walker + rows[row] + sizeof(unsigned) * (2 + i) + sizeof(int));
			skip_lists[row].read(reinterpret_cast<char*>(&next), sizeof(next));
			unsigned nextid;
			skip_lists[row].seekg(next);
			skip_lists[row].read(reinterpret_cast<char*>(&nextid), sizeof(nextid));
			skip_lists[row].read(data_n, rows[row]);
			graph += name_node(id, walker, i, data, row, *this) + " -> " + name_node(nextid, next, i, data_n, row, *this) + ";\n";
			if (i != 0)
			{
				graph += "{rank = same; " + name_node(id, walker, i - 1, data, row, *this) + " " + name_node(id, walker, i, data, row, *this) + "}";
				graph += name_node(id, walker, i - 1, data, row, *this) + " -> " + name_node(id, walker, i, data, row, *this) + ";\n";
			}
		}
		//seek to the first next pointer
		skip_lists[row].seekg(walker + sizeof(unsigned) + rows[row] + sizeof(int) + sizeof(unsigned));
		//set walker to the next pointer
		skip_lists[row].read(reinterpret_cast<char*>(&walker), sizeof(walker));
	}

	graph += "}";
	return graph;
}