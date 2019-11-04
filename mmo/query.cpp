
#include "query.h"
#include <string.h>

Query::Query(Database &db) : db(db)
{
	interpreter = 0;
}

void Query::FreeData()
{
	for (unsigned i = 0; i < data.size(); ++i)
	{
		delete data[i];
	}
	data.clear();
	delete interpreter;
	interpreter = 0;
}

Query::~Query()
{
	FreeData();
}

//return and parameters in c-style
//void set(unsigned id, unsigned row, Value value)
Value Query::SetDatabase(std::vector<Value> args)
{
	LOG("set database");
	Value ret;
	if (args.size() != 3)
	{
		LOGW("Database get with incorrect number of arguments");
		return ret;
	}
	unsigned id = Interpreter::ToUnsigned(Interpreter::ToVector(args[0])).m_unsigned;
	unsigned row = Interpreter::ToUnsigned(Interpreter::ToVector(args[1])).m_unsigned;

	ret = args[2];
	char *buffer = 0;
	switch ((Types)db.types[row])
	{
	case Char:
		ret = Interpreter::ToChar(Interpreter::ToVector(ret));
		buffer = &ret.m_char;
		break;
	case Integer:
		ret = Interpreter::ToInt(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_int;
		break;
	case Unsigned:
		ret = Interpreter::ToUnsigned(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_unsigned;
		break;
	case Short:
		ret = Interpreter::ToShort(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_short;
		break;
	case Float:
		ret = Interpreter::ToFloat(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_float;
		break;
	case Double:
		ret = Interpreter::ToDouble(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_double;
		break;
	case String:
		ret = Interpreter::ToString(Interpreter::ToVector(ret));
		buffer = new char[db.rows[row]];
		memset(buffer, 0, db.rows[row]);
		data.push_back(buffer);
		memcpy(buffer, ret.m_string.c_str(), ret.m_string.length());
		break;
	case Blob:
	    if (ret.type != Blob)
		{
			LOGW("Passing type " << ret.type << " to function taking blob");
		}
		buffer = new char[db.rows[row]];
		memset(buffer, 0, db.rows[row]);
		data.push_back(buffer);
		memcpy(buffer, ret.data, ret.size);
		break;
	default:
	break;
	}
	if (buffer == 0)
	{
		LOGW("trying to set wrong type");
		return ret;
	}
	LOG("Calling db set");
	db.Set(id, row, buffer);
	LOG("db setted");
	return ret;
}

//return and parameters in c-style
//Value create()
Value Query::CreateDatabase(std::vector<Value> args)
{
	LOG("creat database");
	Value ret;
	ret.type = Unsigned;
	ret.m_unsigned = db.Create();
	if (args.size() != 0)
	{
		LOGW("Dataabse create should not take arguments");
	}
	LOG("creat database done");
	return ret;
}

Value Query::DeleteDatabase(std::vector<Value> args)
{
	Value ret;
	if (args.size() != 1)
	{
		LOGW("delete takes one argument");
		return ret;
	}
	db.Delete(Interpreter::ToUnsigned(Interpreter::ToVector(args[0])).m_unsigned);
	return ret;
}

Value Query::FindDatabase(std::vector<Value> args, Interpreter &interpreter)
{
	LOG("Looking for shit in a database");
	Value ret;
	if (args.size() != 2)
	{
		LOGW("find atatak 2 argutmetgent");
	}
	unsigned row = Interpreter::ToUnsigned(Interpreter::ToVector(args[0])).m_unsigned;

	ret = args[1];
	char *buffer = 0;
	switch ((Types)db.types[row])
	{
	case Char:
		ret = Interpreter::ToChar(Interpreter::ToVector(ret));
		buffer = &ret.m_char;
		break;
	case Integer:
		ret = Interpreter::ToInt(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_int;
		break;
	case Unsigned:
		ret = Interpreter::ToUnsigned(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_unsigned;
		break;
	case Short:
		ret = Interpreter::ToShort(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_short;
		break;
	case Float:
		ret = Interpreter::ToFloat(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_float;
		break;
	case Double:
		ret = Interpreter::ToDouble(Interpreter::ToVector(ret));
		buffer = (char*)&ret.m_double;
		break;
	case String:
		ret = Interpreter::ToString(Interpreter::ToVector(ret));
		buffer = new char[db.rows[row]];
		memset(buffer, 0, db.rows[row]);
		data.push_back(buffer);
		memcpy(buffer, ret.m_string.c_str(), ret.m_string.length());
		break;
	case Blob:
	    if (ret.type != Blob)
		{
			LOGW("Passing type " << ret.type << " to function taking blob");
		}
		buffer = new char[db.rows[row]];
		memset(buffer, 0, db.rows[row]);
		data.push_back(buffer);
		memcpy(buffer, ret.data, ret.size);
		break;
	default:
	break;
	}
	if (buffer == 0)
	{
		LOGW("trying to find wrong type");
		return ret;
	}
	LOG("about to find");
	std::vector<unsigned> results = db.Find(row, buffer);
	LOG("shit found " << results.size());
	ret.type = Vector;
	ret.m_vector = new std::vector<Value>;
	interpreter.vectors.push_back(ret.m_vector);
	for (unsigned i = 0; i < results.size(); ++i)
	{
		Value entry;
		entry.type = Unsigned;
		entry.m_unsigned = results[i];
		ret.m_vector->push_back(entry);
	}
	return ret;
}

//c-style definition
//vector findRange(vector<unsigned> rows, vector<value> min, vector<value> max, int numResults = -1, bool sort_by_smallest = true);
Value Query::FindRangeDatabase(std::vector<Value> args, Interpreter &interpreter)
{
	LOG("Looking for shit in a database");
	Value ret;
	if (args.size() < 3)
	{
		LOGW("findRange with too few arguments");
		return ret;
	}
	if (args[0].type != Types::Vector || args[1].type != Types::Vector || args[2].type != Types::Vector)
	{
		LOGW("findRange arguments not vectors");
		return ret;
	}
	if (args[0].m_vector->size() == 0 || args[0].m_vector->size() != args[1].m_vector->size() || args[1].m_vector->size() != args[2].m_vector->size())
	{
		LOGW("findRange vector size wrong");
		return ret;
	}
	//need to convert vector ov values into a vector of unsigned
	std::vector<unsigned> rows;
	for (unsigned i = 0; i < args[0].m_vector->size(); ++i)
	{
		rows.push_back(Interpreter::ToUnsigned(Interpreter::ToVector((*args[0].m_vector)[i])).m_unsigned);
	}

	std::vector<char *> min;
	std::vector<char *> max;
	//need to keep the temporary values around until the end of the function because we save pointers to their data in min
	std::vector<Value> temp(rows.size());
	//also one for max
	std::vector<Value> temp2(rows.size());
	for (unsigned i = 0; i < rows.size(); ++i)
	{
		temp[i] = (*args[1].m_vector)[i];
		char *buffer = 0;
		switch ((Types)db.types[rows[i]])
		{
		case Char:
			temp[i] = Interpreter::ToChar(Interpreter::ToVector(temp[i]));
			buffer = &temp[i].m_char;
			break;
		case Integer:
			temp[i] = Interpreter::ToInt(Interpreter::ToVector(temp[i]));
			buffer = (char*)&temp[i].m_int;
			break;
		case Unsigned:
			temp[i] = Interpreter::ToUnsigned(Interpreter::ToVector(temp[i]));
			buffer = (char*)&temp[i].m_unsigned;
			break;
		case Short:
			temp[i] = Interpreter::ToShort(Interpreter::ToVector(temp[i]));
			buffer = (char*)&temp[i].m_short;
			break;
		case Float:
			temp[i] = Interpreter::ToFloat(Interpreter::ToVector(temp[i]));
			buffer = (char*)&temp[i].m_float;
			break;
		case Double:
			temp[i] = Interpreter::ToDouble(Interpreter::ToVector(temp[i]));
			buffer = (char*)&temp[i].m_double;
			break;
		case String:
			temp[i] = Interpreter::ToString(Interpreter::ToVector(temp[i]));
			buffer = new char[db.rows[rows[i]]];
			memset(buffer, 0, db.rows[rows[i]]);
			data.push_back(buffer);
			memcpy(buffer, temp[i].m_string.c_str(), temp[i].m_string.length());
			break;
		case Blob:
			if (temp[i].type != Blob)
			{
				LOGW("Passing type " << temp[i].type << " to function taking blob");
			}
			buffer = new char[db.rows[rows[i]]];
			memset(buffer, 0, db.rows[rows[i]]);
			data.push_back(buffer);
			memcpy(buffer, temp[i].data, temp[i].size);
			break;
		default:
			LOGW("trying to find wrong type");
			return temp[i];
		break;
		}
		min.push_back(buffer);
	}
	//now do max values
	for (unsigned i = 0; i < rows.size(); ++i)
	{
		temp2[i] = (*args[2].m_vector)[i];
		char *buffer = 0;
		switch ((Types)db.types[rows[i]])
		{
		case Char:
			temp2[i] = Interpreter::ToChar(Interpreter::ToVector(temp2[i]));
			buffer = &temp2[i].m_char;
			break;
		case Integer:
			temp2[i] = Interpreter::ToInt(Interpreter::ToVector(temp2[i]));
			buffer = (char*)&temp2[i].m_int;
			break;
		case Unsigned:
			temp2[i] = Interpreter::ToUnsigned(Interpreter::ToVector(temp2[i]));
			buffer = (char*)&temp2[i].m_unsigned;
			break;
		case Short:
			temp2[i] = Interpreter::ToShort(Interpreter::ToVector(temp2[i]));
			buffer = (char*)&temp2[i].m_short;
			break;
		case Float:
			temp2[i] = Interpreter::ToFloat(Interpreter::ToVector(temp2[i]));
			buffer = (char*)&temp2[i].m_float;
			break;
		case Double:
			temp2[i] = Interpreter::ToDouble(Interpreter::ToVector(temp2[i]));
			buffer = (char*)&temp2[i].m_double;
			break;
		case String:
			temp2[i] = Interpreter::ToString(Interpreter::ToVector(temp2[i]));
			buffer = new char[db.rows[rows[i]]];
			memset(buffer, 0, db.rows[rows[i]]);
			data.push_back(buffer);
			memcpy(buffer, temp2[i].m_string.c_str(), temp2[i].m_string.length());
			break;
		case Blob:
			if (temp2[i].type != Blob)
			{
				LOGW("Passing type " << temp2[i].type << " to function taking blob");
			}
			buffer = new char[db.rows[rows[i]]];
			memset(buffer, 0, db.rows[rows[i]]);
			data.push_back(buffer);
			memcpy(buffer, temp2[i].data, temp2[i].size);
			break;
		default:
			LOGW("trying to find wrong type");
			return temp2[i];
		break;
		}
		max.push_back(buffer);
	}
	//number of results
	int num_results = -1;
	if (args.size() > 3)
	{
		num_results = Interpreter::ToUnsigned(Interpreter::ToVector(args[3])).m_int;
	}
	bool smallest = true;
	if (args.size() > 4)
	{
		num_results = Interpreter::ToUnsigned(Interpreter::ToVector(args[4])).m_int != 0;
	}
	std::vector<unsigned> results = db.Find(rows, min, max, num_results, smallest);
	LOG("Find Range num results = " << results.size());
	ret.type = Vector;
	ret.m_vector = new std::vector<Value>;
	interpreter.vectors.push_back(ret.m_vector);
	for (unsigned i = 0; i < results.size(); ++i)
	{
		Value entry;
		entry.type = Unsigned;
		entry.m_unsigned = results[i];
		ret.m_vector->push_back(entry);
	}
	return ret;
}

//return and parameters in c-style
//Value get(unsigned id, unsigned row)
Value Query::GetDatabase(std::vector<Value> args)
{
	Value ret;
	if (args.size() != 2)
	{
		LOGW("Database get with incorrect number of arguments");
		return ret;
	}
	unsigned id = Interpreter::ToUnsigned(Interpreter::ToVector(args[0])).m_unsigned;
	unsigned row = Interpreter::ToUnsigned(Interpreter::ToVector(args[1])).m_unsigned;
	ret.type = (Types)db.types[row];
	char *buffer;
	unsigned n = db.Get(id, row, buffer);
	data.push_back(buffer);
	switch (ret.type)
	{
	case Char:
		ret.m_char = *buffer;
		break;
	case Integer:
		ret.m_int = (*(int*)buffer);
		break;
	case Unsigned:
		ret.m_unsigned = (*(unsigned*)buffer);
		break;
	case Short:
		ret.m_short = (*(short*)buffer);
		break;
	case Float:
		ret.m_float = (*(float*)buffer);
		break;
	case Double:
		ret.m_double = (*(double*)buffer);
		break;
	case String:
		ret.m_string = std::string(buffer, n);
		break;
	case Blob:
		ret.data = new char[n];
		memcpy(ret.data, buffer, n);
		ret.size = n;
		data.push_back(ret.data);
	    break;
	default:
	break;
	}
	return ret;
}

//return and parameters in c-style
//void* get(unsigned id, unsigned start_row, unsigned end_row)
Value Query::GetDatabaseRange(std::vector<Value> args)
{
	Value ret;
	if (args.size() != 3)
	{
		LOGW("Database get range with incorrect number of arguments");
		return ret;
	}
	unsigned id = Interpreter::ToUnsigned(Interpreter::ToVector(args[0])).m_unsigned;
	unsigned minrow = Interpreter::ToUnsigned(Interpreter::ToVector(args[1])).m_unsigned;
	unsigned maxrow = Interpreter::ToUnsigned(Interpreter::ToVector(args[2])).m_unsigned;
	ret.type = Types::Blob;
	ret.size = db.Get(id, minrow, maxrow, ret.data);
	data.push_back(ret.data);
	return ret;
}


//return and parameters in c-style
//void set(unsigned id, unsigned start_row, unsigned end_row, void *data)
Value Query::SetDatabaseRange(std::vector<Value> args)
{
	Value ret;
	if (args.size() != 4)
	{
		LOGW("Database set range with incorrect number of arguments");
		return ret;
	}
	if (args[3].type != Types::Blob)
	{
		LOGW("Set range data is not blob");
		return ret;
	}
	unsigned id = Interpreter::ToUnsigned(Interpreter::ToVector(args[0])).m_unsigned;
	unsigned minrow = Interpreter::ToUnsigned(Interpreter::ToVector(args[1])).m_unsigned;
	unsigned maxrow = Interpreter::ToUnsigned(Interpreter::ToVector(args[2])).m_unsigned;
	db.Set(id, minrow, maxrow, args[3].data);
	return ret;
}

Value print(std::vector<Value> args)
{
	LOG("Print called with " << args.size() << " elements");
	for (unsigned i = 0; i < args.size(); ++i)
	{
		if (args[i].type == Blob)
		{
			std::cout << "Object size " << args[i].size << std::flush;
		}
		else if (args[i].type == Vector)
		{
			for (unsigned j = 0; j < args[i].m_vector->size(); ++j)
			{
				std::vector<Value> temp;
				temp.push_back((*args[i].m_vector)[j]);
				std::cout << " " << j << ":" << std::flush;
				print(temp);
			}
		}
		else if (args[i].type == String)
		{
			std::cout << args[i].m_string << std::flush;
		}
		else
		{
			std::cout << Interpreter::ToDouble(Interpreter::ToVector(args[i])).m_double << std::flush;
		}
	}
	return Value();
}

int Query::PackValue(char *buffer, Value value)
{
	LOG("PACK VALUE type = " << value.type);
	int ret = 0;

	switch(value.type)
	{
		case Types::Blob:
		//blobs are easy just copy data
		memcpy(buffer, value.data,value.size);
		std::cout << "Pack blob " << ToHexString(value.data, value.size) << std::endl;
		ret += value.size;
		break;
		case Types::Char:
		//default types are easy just cast buffer
		buffer[0] = value.m_char;
		ret += 1;
		break;
		case Types::Double:
		*reinterpret_cast<double*>(buffer) = value.m_double;
		ret += sizeof(double);
		break;
		case Types::Float:
		*reinterpret_cast<float*>(buffer) = value.m_float;
		ret += sizeof(float);
		break;
		case Types::Integer:
		*reinterpret_cast<int*>(buffer) = value.m_int;
		ret += sizeof(int);
		break;
		case Types::Unsigned:
		*reinterpret_cast<unsigned*>(buffer) = value.m_unsigned;
		ret += sizeof(unsigned);
		break;
		case Types::Short:
		*reinterpret_cast<short*>(buffer) = value.m_short;
		ret += sizeof(short);
		break;
		case Types::String:
		memcpy(buffer, value.m_string.c_str(),value.m_string.length());
		ret += value.m_string.length();
		break;
		case Types::Vector:
		{
			LOG("Pack vector size, " << value.m_vector->size());
			for (unsigned i = 0; i < value.m_vector->size(); ++i)
			{
				ret += PackValue(buffer+ret, (*value.m_vector)[i]);
			}
			break;
		}
		break;
		case Types::Map:
		{
			for (auto it = value.m_map->begin(); it != value.m_map->end(); ++it)
			{
				ret += PackValue(buffer+ret, it->first);
				ret += PackValue(buffer+ret, it->second);
			}
			break;
		}
		break;
		default:
		LOGW("Trying to pack type "<< value.type <<" not setup");
		break;
	}

	return ret;
}

bool Query::Compile(std::string code, std::vector<Value> &parameters, Value &returnValue)
{
	LOG("Running script");
	std::cout << "Running script!!" << std::endl;
	std::vector<Token> tokens;

	//free data from last tree
  FreeAbstractNodes();
	FreeData();

	//make the tokens!
	const char *str = code.c_str();
	DfaState *dfa = CreateLanguageDfa();
	LOG("Created dfa");
	while (1)
	{
		Token token;
		ReadLanguageToken(dfa, str, token);
		if (token.tokenType == Token::Invalid)
		{
			break;
		}
		str += token.length;
		//dont add comments or white space tokens
		if (token.tokenType != Token::Whitespace && token.tokenType != Token::MultiLineComment && token.tokenType != Token::SingleLineComment)
		{
			tokens.push_back(token);
		}
	}
	LOG("Read tokens");

	//make the tree!!
	unsigned index = 0;
	MainNode *node = 0;
	try
	{
		node = MainRule(tokens, index);
	}
	catch (std::exception &)
	{
		LOG("Tree bad");
		return false;
	}
	if (node == 0)
	{
		LOG("Tree bad");
		return false;
	}

	LOG("Made tree");
	SetupLiterals pass1;
	pass1.Visit(node);
	LOG("did pass 1");

	//make the new interpreter
	interpreter = new Interpreter();
	interpreter->arguments = parameters;
	interpreter->functions["print"] = &print;
	interpreter->functions["get"] = std::bind(&Query::GetDatabase, this, std::placeholders::_1);
	interpreter->functions["set"] = std::bind(&Query::SetDatabase, this, std::placeholders::_1);
	interpreter->functions["getRange"] = std::bind(&Query::GetDatabaseRange, this, std::placeholders::_1);
	interpreter->functions["setRange"] = std::bind(&Query::SetDatabaseRange, this, std::placeholders::_1);
	interpreter->functions["create"] = std::bind(&Query::CreateDatabase, this, std::placeholders::_1);
	interpreter->functions["delete"] = std::bind(&Query::DeleteDatabase, this, std::placeholders::_1);
	interpreter->functions["find"] = std::bind(&Query::FindDatabase, this, std::placeholders::_1, std::ref(*interpreter));
	interpreter->functions["findRange"] = std::bind(&Query::FindRangeDatabase, this, std::placeholders::_1, std::ref(*interpreter));
	interpreter->Visit(node);
	LOG("Code Finished");
	//set the return value
	returnValue = interpreter->returnValue;
	LOG("return value set type = " << returnValue.type);
	if (returnValue.type == Types::Vector)
	{
		LOG("Return vector with " << returnValue.m_vector->size() << " elements");
	}
	//dont delete the interpreter so it stays alive in case the return value is pointing to memory inside the interpreter
	return true;
}