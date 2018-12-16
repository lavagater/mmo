
#include "query.h"
#include <string.h>

Query::Query(Database &db) : db(db)
{
}

void Query::FreeData()
{
	for (unsigned i = 0; i < data.size(); ++i)
	{
		delete data[i];
	}
	data.clear();
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
	default:
	break;
	}
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
	int ret = 0;

	switch(value.type)
	{
		case Types::Blob:
		//blobs are easy just copy data
		memcpy(buffer, value.data,value.size);
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
		FreeAbstractNodes();
		LOG("Tree bad");
		return false;
	}
	if (node == 0)
	{
		FreeAbstractNodes();
		LOG("Tree bad");
		return false;
	}

	LOG("Made tree");
	SetupLiterals pass1;
	pass1.Visit(node);
	LOG("did pass 1");

	//run the code!!!
	Interpreter interpreter;
	interpreter.arguments = parameters;
	interpreter.functions["print"] = &print;
	interpreter.functions["get"] = std::bind(&Query::GetDatabase, this, std::placeholders::_1);
	interpreter.functions["set"] = std::bind(&Query::SetDatabase, this, std::placeholders::_1);
	interpreter.functions["create"] = std::bind(&Query::CreateDatabase, this, std::placeholders::_1);
	interpreter.functions["delete"] = std::bind(&Query::DeleteDatabase, this, std::placeholders::_1);
	interpreter.functions["find"] = std::bind(&Query::FindDatabase, this, std::placeholders::_1, std::ref(interpreter));
	interpreter.Visit(node);
	//set the return value
	returnValue = interpreter.returnValue;

	FreeAbstractNodes();
	FreeData();
	return true;
}