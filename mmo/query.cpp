
#include "query.h"

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
	}
	if (buffer == 0)
	{
		LOGW("trying to set wrong type");
		return ret;
	}
	db.Set(id, row, buffer);
	return ret;
}

//return and parameters in c-style
//Value create()
Value Query::CreateDatabase(std::vector<Value> args)
{
	Value ret;
	ret.type = Unsigned;
	ret.m_unsigned = db.Create();
	if (args.size() != 0)
	{
		LOGW("Dataabse create should not take arguments");
	}
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

Value Query::FindDatabase(std::vector<Value> args)
{
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
	}
	if (buffer == 0)
	{
		LOGW("trying to find wrong type");
		return ret;
	}
	std::vector<unsigned> results = db.Find(row, buffer);
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
	}
	return ret;
}

Value print(std::vector<Value> args)
{
	for (unsigned i = 0; i < args.size(); ++i)
	{
		if (args[i].type == Blob)
		{
			std::cout << "Object size " << args[i].size;
		}
		else if (args[i].type == Vector)
		{
			for (unsigned j = 0; j < args[i].m_vector->size(); ++j)
			{
				std::vector<Value> temp;
				temp.push_back((*args[i].m_vector)[j]);
				std::cout << " " << j << ":";
				print(temp);
			}
		}
		else if (args[i].type == String)
		{
			std::cout << args[i].m_string;
		}
		else
		{
			std::cout << Interpreter::ToDouble(Interpreter::ToVector(args[i])).m_double;
		}
	}
	return Value();
}

bool Query::Compile(std::string code, char *data)
{
	std::vector<Token> tokens;

	//make the tokens!
	const char *str = code.c_str();
	DfaState *dfa = CreateLanguageDfa();
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
		return false;
	}
	if (node == 0)
	{
		FreeAbstractNodes();
		return false;
	}

	SetupLiterals pass1;
	pass1.Visit(node);

	//run the code!!!
	interpreter.functions["print"] = &print;
	interpreter.functions["get"] = std::bind(&Query::GetDatabase, this, std::placeholders::_1);
	interpreter.functions["set"] = std::bind(&Query::SetDatabase, this, std::placeholders::_1);
	interpreter.functions["create"] = std::bind(&Query::CreateDatabase, this, std::placeholders::_1);
	interpreter.functions["delete"] = std::bind(&Query::DeleteDatabase, this, std::placeholders::_1);
	interpreter.functions["find"] = std::bind(&Query::FindDatabase, this, std::placeholders::_1);
	interpreter.Visit(node);

	FreeAbstractNodes();
	FreeData();
	return true;
}