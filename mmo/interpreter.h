#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include "logger.h"
#include <unordered_map>
#include <functional>

class Adder
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return lhs + rhs;
	}
};
class Subber
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return lhs - rhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("subtract of std::string is undefined");
		return lhs;
	}
};
class Timeser
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return lhs * rhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("multiply of std::string is undefined");
		return lhs;
	}
};
class Divider
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return lhs / rhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("divide of std::string is undefined");
		return lhs;
	}
};
class Modder
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return lhs - rhs * std::floor(lhs / rhs);
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("modulo of std::string is undefined");
		return lhs;
	}
};

class Equalityer
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return T(lhs == rhs);
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		if (lhs == rhs)
		{
			return "1";
		}
		else
		{
			return "0";
		}
	}
};
class Inequalityer
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return T(lhs != rhs);
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		if (lhs != rhs)
		{
			return "1";
		}
		else
		{
			return "0";
		}
	}
};

class LessThaner
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return T(lhs < rhs);
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search dvwev");
		return lhs;
	}
};

class LessThanOrEqualToer
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return T(lhs <= rhs);
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search asdqwv");
		return lhs;
	}
};

class GreaterThaner
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return T(lhs > rhs);
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search awerhg");
		return lhs;
	}
};

class GreaterThanOrEqualToer
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return T(lhs >= rhs);
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search rthr");
		return lhs;
	}
};

class Logicaler
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return rhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search wwbrte");
		return lhs;
	}
};

class Positiver
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return +lhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search sdvwegsdg");
		return lhs;
	}
};

class Negator
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return -lhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search sdvwegsdg");
		return lhs;
	}
	template<>
	unsigned operator()<unsigned>(unsigned lhs, unsigned rhs)
	{
		LOGW("This should not happen search sdvewsvgsxd");
		return lhs;
	}
};

class LogicalNotter
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return !lhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search rdhbr");
		return lhs;
	}
};

class PreIncrementor
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return ++lhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search tndfgdrtrd");
		return lhs;
	}
};

class PreDecrementer
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return --lhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search sdvwegsdg");
		return lhs;
	}
};

class PostIncrementer
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return ++lhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search sdvwegsdg");
		return lhs;
	}
};

class PostDecrementor
{
public:
	template<typename T>
	T operator()(T lhs, T rhs)
	{
		return --lhs;
	}
	template<>
	std::string operator()<std::string>(std::string lhs, std::string rhs)
	{
		LOGW("This should not happen search sdvwegsdg");
		return lhs;
	}
};

class Interpreter : public Visitor
{
public:
	std::unordered_map<std::string, std::function<Value(std::vector<Value>)> > functions;
	std::unordered_map<std::string, Value> variables;
	std::vector<std::unordered_map<Value, Value, ValueHash> *> maps;
	std::vector<std::vector<Value> *> vectors;
	//the arguments to be passed to main, format is [1 byte size of argument][the argument]...
	const char *arguments; //note this can also be left blank and the arguments can be directly added to the variables
						   //size in bytes of arguments char array
	unsigned size;
	Value lastValue;
	//this is set inside a break or continue node, 1 for break, 2 for continue
	//it stops a scope and propegates up to the if/while where it is reset to 0
	//return node uses this as well with a value of 3
	int stopScope;
	Interpreter()
	{
		functions["char"] = &ToChar;
		functions["int"] = &ToInt;
		functions["unsigned"] = &ToUnsigned;
		functions["short"] = &ToShort;
		functions["float"] = &ToFloat;
		functions["double"] = &ToDouble;
		functions["string"] = &ToString;
		functions["vector"] = std::bind(&Interpreter::CreateVector, this, std::placeholders::_1);
		functions["map"] = std::bind(&Interpreter::CreateMap, this, std::placeholders::_1);
		functions["GetKeys"] = std::bind(&Interpreter::GetKeys, this, std::placeholders::_1);;
		functions["Size"] = &VectorSize;
	}
	int Visit(MainNode *node)
	{
		//for now im gonna assume the arguments are already in the variables
		node->scope->Walk(this);
		return 0;
	}
	int Visit(StatementNode *node)
	{
		node->Walk(this, false);
		return 0;
	}
	int Visit(ScopeNode *node)
	{
		for (unsigned i = 0; i < node->statements.size(); ++i)
		{
			node->statements[i]->Walk(this);
			if (stopScope)
			{
				break;
			}
		}
		return 0;
	}
	int Visit(ReturnNode *node)
	{
		node->Walk(this, false);
		stopScope = 3;
		return 0;
	}
	int Visit(BreakNode *node)
	{
		node->Walk(this, false);
		stopScope = 1;
		return 0;
	}
	int Visit(ContinueNode *node)
	{
		node->Walk(this, false);
		stopScope = 2;
		return 0;
	}
	int Visit(ExpressionNode *node)
	{
		node->Walk(this, false);
		return 0;
	}
	int Visit(IfNode *node)
	{
		if (node->condition)
		{
			node->condition->Walk(this);
		}
		else
		{
			//if there was no condition then this is an else
			lastValue.m_int = 1;
		}
		if (lastValue.m_int)
		{
			node->scope->Walk(this);
			//dont propegate any further, unless its a return
			if (stopScope != 3)
			{
				stopScope = 0;
			}
		}
		else if (node->elseIf)
		{
			node->elseIf->Walk(this);
		}
		return 0;
	}
	int Visit(WhileNode *node)
	{
		if (node->condition)
		{
			node->condition->Walk(this);
		}
		else
		{
			return 0;
		}
		while (lastValue.type == Integer && lastValue.m_int)
		{
			node->scope->Walk(this);
			//dont propegate any further, unless its a return
			if (stopScope == 1)
			{
				stopScope = 0;
				break;
			}
			else if (stopScope == 3)
			{
				//propegate the return further
				break;
			}
			else
			{
				//for continue just reset the value
				stopScope = 0;
			}
			node->condition->Walk(this);
		}
		return 0;
	}
	int Visit(LiteralNode *node)
	{
		lastValue = node->value;
		return 0;
	}
	template <typename T>
	Value BinaryNodeHelper(Value lhs, Value rhs, T function)
	{
		Value ret;
		if (lhs.type == Vector || rhs.type == Vector
			|| lhs.type == Map || rhs.type == Map
			|| lhs.type == Blob || rhs.type == Blob)
		{
			LOGW("Vector, map, and blob types cannot be converted");
			return ret;
		}
		switch (lhs.type)
		{
		case Char:
			if (rhs.type == Char)
			{
				ret.type = Char;
				ret.m_char = function(lhs.m_char, rhs.m_char);
			}
			else if (rhs.type == Integer)
			{
				ret.type = Integer;
				ret.m_int = function(int(lhs.m_char), rhs.m_int);
			}
			else if (rhs.type == Unsigned)
			{
				ret.type = Unsigned;
				ret.m_unsigned = function(unsigned(lhs.m_char), rhs.m_unsigned);
			}
			else if (rhs.type == Short)
			{
				ret.type = Short;
				ret.m_short = function(short(lhs.m_char), rhs.m_short);
			}
			else if (rhs.type == Float)
			{
				ret.type = Float;
				ret.m_float = function(float(lhs.m_char), rhs.m_float);
			}
			else if (rhs.type == Double)
			{
				ret.type = Double;
				ret.m_double = function(double(lhs.m_char), rhs.m_double);
			}
			else if (rhs.type == String)
			{
				ret.type = String;
				ret.m_string = function(std::to_string(lhs.m_char), rhs.m_string);
			}
			break;
		case Integer:
			if (rhs.type == Char)
			{
				ret.type = Integer;
				ret.m_int = function(lhs.m_int, int(rhs.m_char));
			}
			else if (rhs.type == Unsigned)
			{
				ret.type = Unsigned;
				ret.m_unsigned = function(unsigned(lhs.m_int), rhs.m_unsigned);
			}
			else if (rhs.type == Short)
			{
				ret.type = Integer;
				ret.m_int = function(lhs.m_int, int(rhs.m_short));
			}
			else if (rhs.type == Float)
			{
				ret.type = Float;
				ret.m_float = function(float(lhs.m_int), rhs.m_float);
			}
			else if (rhs.type == Double)
			{
				ret.type = Double;
				ret.m_double = function(double(lhs.m_int), rhs.m_double);
			}
			else if (rhs.type == String)
			{
				ret.type = String;
				ret.m_string = function(std::to_string(lhs.m_int), rhs.m_string);
			}
			break;
		case Unsigned:
			if (rhs.type == Unsigned)
			{
				ret.type = Unsigned;
				ret.m_unsigned = function(lhs.m_unsigned, rhs.m_unsigned);
			}
			else if (rhs.type == Integer)
			{
				ret.type = Unsigned;
				ret.m_unsigned = function(lhs.m_unsigned, unsigned(rhs.m_int));
			}
			else if (rhs.type == Char)
			{
				ret.type = Unsigned;
				ret.m_unsigned = function(lhs.m_unsigned, unsigned(rhs.m_char));
			}
			else if (rhs.type == Short)
			{
				ret.type = Unsigned;
				ret.m_unsigned = function(lhs.m_unsigned, unsigned(rhs.m_short));
			}
			else if (rhs.type == Float)
			{
				ret.type = Float;
				ret.m_float = function(float(lhs.m_unsigned), rhs.m_float);
			}
			else if (rhs.type == Double)
			{
				ret.type = Double;
				ret.m_double = function(double(lhs.m_unsigned), rhs.m_double);
			}
			else if (rhs.type == String)
			{
				ret.type = String;
				ret.m_string = function(std::to_string(lhs.m_unsigned), rhs.m_string);
			}
			break;
		case Short:
			if (rhs.type == Short)
			{
				ret.type = Short;
				ret.m_short = function(lhs.m_short, rhs.m_short);
			}
			else if (rhs.type == Integer)
			{
				ret.type = Integer;
				ret.m_int = function(int(lhs.m_short), rhs.m_int);
			}
			else if (rhs.type == Unsigned)
			{
				ret.type = Unsigned;
				ret.m_unsigned = function(unsigned(lhs.m_short), rhs.m_unsigned);
			}
			else if (rhs.type == Char)
			{
				ret.type = Short;
				ret.m_short = function(lhs.m_short, short(rhs.m_char));
			}
			else if (rhs.type == Float)
			{
				ret.type = Float;
				ret.m_float = function(float(lhs.m_short), rhs.m_float);
			}
			else if (rhs.type == Double)
			{
				ret.type = Double;
				ret.m_double = function(double(lhs.m_short), rhs.m_double);
			}
			else if (rhs.type == String)
			{
				ret.type = String;
				ret.m_string = function(std::to_string(lhs.m_short), rhs.m_string);
			}
			break;
		case Float:
			if (rhs.type == Float)
			{
				ret.type = Float;
				ret.m_float = function(lhs.m_float, rhs.m_float);
			}
			if (rhs.type == Integer)
			{
				ret.type = Float;
				ret.m_float = function(lhs.m_float, float(rhs.m_int));
			}
			else if (rhs.type == Unsigned)
			{
				ret.type = Float;
				ret.m_float = function(lhs.m_float, float(rhs.m_unsigned));
			}
			else if (rhs.type == Short)
			{
				ret.type = Float;
				ret.m_float = function(lhs.m_float, float(rhs.m_short));
			}
			else if (rhs.type == Char)
			{
				ret.type = Float;
				ret.m_float = function(lhs.m_float, float(rhs.m_char));
			}
			else if (rhs.type == Double)
			{
				ret.type = Double;
				ret.m_double = function(double(lhs.m_float), rhs.m_double);
			}
			else if (rhs.type == String)
			{
				ret.type = String;
				ret.m_string = function(std::to_string(lhs.m_float), rhs.m_string);
			}
			break;
		case Double:
			if (rhs.type == Double)
			{
				ret.type = Double;
				ret.m_double = function(lhs.m_double, rhs.m_double);
			}
			else if (rhs.type == Integer)
			{
				ret.type = Double;
				ret.m_double = function(lhs.m_double, double(rhs.m_int));
			}
			else if (rhs.type == Unsigned)
			{
				ret.type = Double;
				ret.m_double = function(lhs.m_double, double(rhs.m_unsigned));
			}
			else if (rhs.type == Short)
			{
				ret.type = Double;
				ret.m_double = function(lhs.m_double, double(rhs.m_short));
			}
			else if (rhs.type == Float)
			{
				ret.type = Double;
				ret.m_double = function(lhs.m_double, double(rhs.m_float));
			}
			else if (rhs.type == Char)
			{
				ret.type = Double;
				ret.m_double = function(lhs.m_double, double(rhs.m_char));
			}
			else if (rhs.type == String)
			{
				ret.type = String;
				ret.m_string = function(std::to_string(lhs.m_double), rhs.m_string);
			}
			break;
		case String:
			if (rhs.type == String)
			{
				ret.type = String;
				ret.m_string = function(lhs.m_string, rhs.m_string);
			}
			else if (rhs.type == Integer)
			{
				ret.type = String;
				ret.m_string = function(lhs.m_string, std::to_string(rhs.m_int));
			}
			else if (rhs.type == Unsigned)
			{
				ret.type = String;
				ret.m_string = function(lhs.m_string, std::to_string(rhs.m_unsigned));
			}
			else if (rhs.type == Short)
			{
				ret.type = String;
				ret.m_string = function(lhs.m_string, std::to_string(rhs.m_short));
			}
			else if (rhs.type == Float)
			{
				ret.type = String;
				ret.m_string = function(lhs.m_string, std::to_string(rhs.m_float));
			}
			else if (rhs.type == Double)
			{
				ret.type = String;
				ret.m_string = function(lhs.m_string, std::to_string(rhs.m_double));
			}
			else if (rhs.type == Char)
			{
				ret.type = String;
				ret.m_string = function(lhs.m_string, std::to_string(rhs.m_char));
			}
			break;
		}
		return ret;
	}
	static Value ToChar(std::vector<Value> args)
	{
		Value ret;
		ret.type = Char;
		if (args.size() == 0)
		{
			return ret;
		}
		switch (args[0].type)
		{
		case Char:
			ret = args[0];
			break;
		case Integer:
			ret.m_char = args[0].m_int;
			break;
		case Unsigned:
			ret.m_char = args[0].m_unsigned;
			break;
		case Short:
			ret.m_char = args[0].m_short;
			break;
		case Float:
			ret.m_char = args[0].m_float;
			break;
		case Double:
			ret.m_char = args[0].m_double;
			break;
		case String:
			ret.m_char = args[0].m_string[0];
			break;
		}
		return ret;
	}
	static Value ToInt(std::vector<Value> args)
	{
		Value ret;
		ret.type = Integer;
		if (args.size() == 0)
		{
			return ret;
		}
		switch (args[0].type)
		{
		case Char:
			ret.m_int = args[0].m_char;
			break;
		case Integer:
			ret = args[0];
			break;
		case Unsigned:
			ret.m_int = args[0].m_unsigned;
			break;
		case Short:
			ret.m_int = args[0].m_short;
			break;
		case Float:
			ret.m_int = args[0].m_float;
			break;
		case Double:
			ret.m_int = args[0].m_double;
			break;
		case String:
			std::stringstream ss(args[0].m_string);
			ss >> ret.m_int;
			break;
		}
		return ret;
	}
	static Value ToUnsigned(std::vector<Value> args)
	{
		Value ret;
		ret.type = Unsigned;
		if (args.size() == 0)
		{
			return ret;
		}
		switch (args[0].type)
		{
		case Char:
			ret.m_unsigned = args[0].m_char;
			break;
		case Integer:
			ret.m_unsigned = args[0].m_int;
			break;
		case Unsigned:
			ret = args[0];
			break;
		case Short:
			ret.m_unsigned = args[0].m_short;
			break;
		case Float:
			ret.m_unsigned = args[0].m_float;
			break;
		case Double:
			ret.m_unsigned = args[0].m_double;
			break;
		case String:
			std::stringstream ss(args[0].m_string);
			ss >> ret.m_unsigned;
			break;
		}
		return ret;
	}
	static Value ToShort(std::vector<Value> args)
	{
		Value ret;
		ret.type = Short;
		if (args.size() == 0)
		{
			return ret;
		}
		switch (args[0].type)
		{
		case Char:
			ret.m_short = args[0].m_char;
			break;
		case Integer:
			ret.m_short = args[0].m_int;
			break;
		case Unsigned:
			ret.m_short = args[0].m_unsigned;
			break;
		case Short:
			ret = args[0];
			break;
		case Float:
			ret.m_short = args[0].m_float;
			break;
		case Double:
			ret.m_short = args[0].m_double;
			break;
		case String:
			std::stringstream ss(args[0].m_string);
			ss >> ret.m_short;
			break;
		}
		return ret;
	}
	static Value ToFloat(std::vector<Value> args)
	{
		Value ret;
		ret.type = Float;
		if (args.size() == 0)
		{
			return ret;
		}
		switch (args[0].type)
		{
		case Char:
			ret.m_float = args[0].m_char;
			break;
		case Integer:
			ret.m_float = args[0].m_int;
			break;
		case Unsigned:
			ret.m_float = args[0].m_unsigned;
			break;
		case Short:
			ret.m_float = args[0].m_short;
			break;
		case Float:
			ret = args[0];
			break;
		case Double:
			ret.m_float = args[0].m_double;
			break;
		case String:
			std::stringstream ss(args[0].m_string);
			ss >> ret.m_float;
			break;
		}
		return ret;
	}
	static Value ToDouble(std::vector<Value> args)
	{
		Value ret;
		ret.type = Double;
		if (args.size() == 0)
		{
			return ret;
		}
		switch (args[0].type)
		{
		case Char:
			ret.m_double = args[0].m_char;
			break;
		case Integer:
			ret.m_double = args[0].m_int;
			break;
		case Unsigned:
			ret.m_double = args[0].m_unsigned;
			break;
		case Short:
			ret.m_double = args[0].m_short;
			break;
		case Float:
			ret.m_double = args[0].m_float;
			break;
		case Double:
			ret = args[0];
			break;
		case String:
			std::stringstream ss(args[0].m_string);
			ss >> ret.m_double;
			break;
		}
		return ret;
	}
	static Value ToString(std::vector<Value> args)
	{
		Value ret;
		ret.type = String;
		if (args.size() == 0)
		{
			return ret;
		}
		std::stringstream ss;
		switch (args[0].type)
		{
		case Char:
			ss << args[0].m_char;
			ret.m_string = ss.str();
			break;
		case Integer:
			ss << args[0].m_int;
			ret.m_string = ss.str();
			break;
		case Unsigned:
			ss << args[0].m_unsigned;
			ret.m_string = ss.str();
			break;
		case Short:
			ss << args[0].m_short;
			ret.m_string = ss.str();
			break;
		case Float:
			ss << args[0].m_float;
			ret.m_string = ss.str();
			break;
		case Double:
			ss << args[0].m_double;
			ret.m_string = ss.str();
			break;
		case String:
			ret = args[0];
			break;
		}
		return ret;
	}
	Value CreateVector(std::vector<Value> args)
	{
		Value ret;
		ret.type = Vector;
		ret.m_vector = new std::vector<Value>(args);
		vectors.push_back(ret.m_vector);
		return ret;
	}
	static Value VectorSize(std::vector<Value> args)
	{
		Value ret;
		ret.type = Unsigned;
		if (args.size() == 0 || args[0].type != Vector || args[0].m_vector == 0)
		{
			LOGW("Size must take a vector");
			return ret;
		}
		ret.m_unsigned = args[0].m_vector->size();
		return ret;
	}
	Value CreateMap(std::vector<Value> args)
	{
		Value ret;
		ret.type = Map;
		if (args.size() == 0 || args[0].m_map == 0)
		{
			ret.m_map = new std::unordered_map<Value, Value, ValueHash>;
		}
		else
		{
			ret.m_map = new std::unordered_map<Value, Value, ValueHash>(*args[0].m_map);
		}
		maps.push_back(ret.m_map);
		return ret;
	}
	Value GetKeys(std::vector<Value> args)
	{
		Value ret = CreateVector(std::vector<Value>());
		if (args.size() == 0 || args[0].type != Map)
		{
			LOGW("GetKeys must take a map");
			return ret;
		}
		for (auto it = args[0].m_map->begin(); it != args[0].m_map->end(); ++it)
		{
			ret.m_vector->push_back(it->first);
		}
		return ret;
	}
	static std::vector<Value> ToVector(Value rhs)
	{
		std::vector<Value> ret;
		ret.push_back(rhs);
		return ret;
	}
	int Visit(BinaryOperatorNode *node)
	{
		Value lhs;
		Value rhs;
		if (node->left)
		{
			node->left->Walk(this);
			lhs = lastValue;
		}
		else
		{
			LOGW("binary operator node no lhs");
		}
		//short circuit
		if (node->binaryOperator.tokenType == Token::LogicalOr && ToInt(ToVector(lhs)).m_int)
		{
			lastValue = ToInt(ToVector(lhs));
			return 0;
		}
		if (node->binaryOperator.tokenType == Token::LogicalAnd && !ToInt(ToVector(lhs)).m_int)
		{
			lastValue = ToInt(ToVector(lhs));
			return 0;
		}
		if (node->right)
		{
			node->right->Walk(this);
			rhs = lastValue;
		}
		else
		{
			LOGW("binary operator node no rhs");
		}
		switch (node->binaryOperator.tokenType)
		{
		case Token::Assignment:
			if (lhs.pointer)
			{
				*lhs.pointer = rhs;
				//last value is a reference to the left side
				lastValue = *lhs.pointer;
				lastValue.pointer = lhs.pointer;
				return 0;
			}
			else
			{
				LOGW("Token::Assignment lhs is not a pointer");
			}
			break;
		case Token::AssignmentPlus:
			if (lhs.pointer)
			{
				switch (lhs.type)
				{
				case Char:
					rhs = ToChar(ToVector(rhs));
					lhs.pointer->m_char += rhs.m_char;
					break;
				case Integer:
					rhs = ToInt(ToVector(rhs));
					lhs.pointer->m_int += rhs.m_int;
					break;
				case Unsigned:
					rhs = ToUnsigned(ToVector(rhs));
					lhs.pointer->m_unsigned += rhs.m_unsigned;
					break;
				case Short:
					rhs = ToShort(ToVector(rhs));
					lhs.pointer->m_short += rhs.m_short;
					break;
				case Float:
					rhs = ToFloat(ToVector(rhs));
					lhs.pointer->m_float += rhs.m_float;
					break;
				case Double:
					rhs = ToDouble(ToVector(rhs));
					lhs.pointer->m_double += rhs.m_double;
					break;
				case String:
					rhs = ToString(ToVector(rhs));
					lhs.pointer->m_string += rhs.m_string;
					break;
				case Vector:
					lhs.pointer->m_vector->push_back(rhs);
					break;
				default:
					LOGW("Assignment plus with bad type");
					break;
				}
				lastValue = *lhs.pointer;
				lastValue.pointer = lhs.pointer;
				return 0;
			}
			else
			{
				LOGW("Token::AssignmentPlus lhs is not a pointer");
			}
			break;
		case Token::AssignmentMinus:
			if (lhs.pointer)
			{
				switch (lhs.type)
				{
				case Char:
					rhs = ToChar(ToVector(rhs));
					lhs.pointer->m_char -= rhs.m_char;
					break;
				case Integer:
					rhs = ToInt(ToVector(rhs));
					lhs.pointer->m_int -= rhs.m_int;
					break;
				case Unsigned:
					rhs = ToUnsigned(ToVector(rhs));
					lhs.pointer->m_unsigned -= rhs.m_unsigned;
					break;
				case Short:
					rhs = ToShort(ToVector(rhs));
					lhs.pointer->m_short -= rhs.m_short;
					break;
				case Float:
					rhs = ToFloat(ToVector(rhs));
					lhs.pointer->m_float -= rhs.m_float;
					break;
				case Double:
					rhs = ToDouble(ToVector(rhs));
					lhs.pointer->m_double -= rhs.m_double;
					break;
				default:
					LOGW("Assignment minus with bad type");
					break;
				}
				lastValue = *lhs.pointer;
				lastValue.pointer = lhs.pointer;
				return 0;
			}
			else
			{
				LOGW("Token::Assignment\Minus lhs is not a pointer");
			}
			break;
		case Token::AssignmentMultiply:
			if (lhs.pointer)
			{
				switch (lhs.type)
				{
				case Char:
					rhs = ToChar(ToVector(rhs));
					lhs.pointer->m_char *= rhs.m_char;
					break;
				case Integer:
					rhs = ToInt(ToVector(rhs));
					lhs.pointer->m_int *= rhs.m_int;
					break;
				case Unsigned:
					rhs = ToUnsigned(ToVector(rhs));
					lhs.pointer->m_unsigned *= rhs.m_unsigned;
					break;
				case Short:
					rhs = ToShort(ToVector(rhs));
					lhs.pointer->m_short *= rhs.m_short;
					break;
				case Float:
					rhs = ToFloat(ToVector(rhs));
					lhs.pointer->m_float *= rhs.m_float;
					break;
				case Double:
					rhs = ToDouble(ToVector(rhs));
					lhs.pointer->m_double *= rhs.m_double;
					break;
				default:
					LOGW("Assignment multiply with bad type");
					break;
				}
				lastValue = *lhs.pointer;
				lastValue.pointer = lhs.pointer;
				return 0;
			}
			else
			{
				LOGW("Token::AssignmentMultiply lhs is not a pointer");
			}
			break;
		case Token::AssignmentDivide:
			if (lhs.pointer)
			{
				switch (lhs.type)
				{
				case Char:
					rhs = ToChar(ToVector(rhs));
					lhs.pointer->m_char /= rhs.m_char;
					break;
				case Integer:
					rhs = ToInt(ToVector(rhs));
					lhs.pointer->m_int /= rhs.m_int;
					break;
				case Unsigned:
					rhs = ToUnsigned(ToVector(rhs));
					lhs.pointer->m_unsigned /= rhs.m_unsigned;
					break;
				case Short:
					rhs = ToShort(ToVector(rhs));
					lhs.pointer->m_short /= rhs.m_short;
					break;
				case Float:
					rhs = ToFloat(ToVector(rhs));
					lhs.pointer->m_float /= rhs.m_float;
					break;
				case Double:
					rhs = ToDouble(ToVector(rhs));
					lhs.pointer->m_double /= rhs.m_double;
					break;
				default:
					LOGW("Assignment plus with bad type");
					break;
				}
				lastValue = *lhs.pointer;
				lastValue.pointer = lhs.pointer;
				return 0;
			}
			else
			{
				LOGW("Token::AssignmentDevide lhs is not a pointer");
			}
			break;
		case Token::AssignmentModulo:
			if (lhs.pointer)
			{
				switch (lhs.type)
				{
				case Char:
					rhs = ToChar(ToVector(rhs));
					lhs.pointer->m_char %= rhs.m_char;
					break;
				case Integer:
					rhs = ToInt(ToVector(rhs));
					lhs.pointer->m_int %= rhs.m_int;
					break;
				case Unsigned:
					rhs = ToUnsigned(ToVector(rhs));
					lhs.pointer->m_unsigned %= rhs.m_unsigned;
					break;
				case Short:
					rhs = ToShort(ToVector(rhs));
					lhs.pointer->m_short %= rhs.m_short;
					break;
				case Float:
					rhs = ToFloat(ToVector(rhs));
					lhs.pointer->m_float = lhs.m_float - rhs.m_float * std::floor(lhs.m_float / rhs.m_float);
					break;
				case Double:
					rhs = ToDouble(ToVector(rhs));
					lhs.pointer->m_double = lhs.m_double - rhs.m_double * std::floor(lhs.m_double / rhs.m_double);
					break;
				default:
					LOGW("Assignment modulo with bad type");
					break;
				}
				lastValue = *lhs.pointer;
				lastValue.pointer = lhs.pointer;
				return 0;
			}
			else
			{
				LOGW("Token::AssignmentModulo lhs is not a pointer");
			}
			break;
		case Token::Plus:
			lhs = BinaryNodeHelper<Adder>(lhs, rhs, Adder());
			break;
		case Token::Minus:
			lhs = BinaryNodeHelper<Subber>(lhs, rhs, Subber());
			break;
		case Token::Asterisk:
			lhs = BinaryNodeHelper<Timeser>(lhs, rhs, Timeser());
			break;
		case Token::Divide:
			lhs = BinaryNodeHelper<Divider>(lhs, rhs, Divider());
			break;
		case Token::Modulo:
			lhs = BinaryNodeHelper<Modder>(lhs, rhs, Modder());
			break;
		case Token::Equality:
			if (lhs.type == Vector || rhs.type == Vector
				|| lhs.type == Map || rhs.type == Map
				|| lhs.type == Blob || rhs.type == Blob)
			{
				if (lhs.type == rhs.type)
				{
					lhs.type = Integer;
					if (lhs.type == Vector)
					{
						lhs.m_int = lhs.m_vector == rhs.m_vector;
					}
					else if (lhs.type == Map)
					{
						lhs.m_int = lhs.m_map == rhs.m_map;
					}
					else if (lhs.type == Blob)
					{
						if (lhs.size == rhs.size)
						{
							lhs.m_int = !memcmp(lhs.data, rhs.data, lhs.size);
						}
						else
						{
							lhs.m_int = 0;
						}
					}
				}
			}
			else
			{
				lhs = BinaryNodeHelper<Equalityer>(lhs, rhs, Equalityer());
				lhs = ToInt(ToVector(lhs));
			}
			break;
		case Token::Inequality:
			if (lhs.type == Vector || rhs.type == Vector
				|| lhs.type == Map || rhs.type == Map
				|| lhs.type == Blob || rhs.type == Blob)
			{
				if (lhs.type == rhs.type)
				{
					lhs.type = Integer;
					if (lhs.type == Vector)
					{
						lhs.m_int = lhs.m_vector != rhs.m_vector;
					}
					else if (lhs.type == Map)
					{
						lhs.m_int = lhs.m_map != rhs.m_map;
					}
					else if (lhs.type == Blob)
					{
						if (lhs.size == rhs.size)
						{
							lhs.m_int = memcmp(lhs.data, rhs.data, lhs.size);
						}
						else
						{
							lhs.m_int = 1;
						}
					}
				}
			}
			else
			{
				lhs = BinaryNodeHelper<Inequalityer>(lhs, rhs, Inequalityer());
				lhs = ToInt(ToVector(lhs));
			}
			break;
			break;
		case Token::LessThan:
			if (lhs.type == Vector || rhs.type == Vector
				|| lhs.type == Map || rhs.type == Map
				|| lhs.type == Blob || rhs.type == Blob
				|| lhs.type == String || rhs.type == String)
			{
				LOGW("Comparison on non number value");
			}
			else
			{
				lhs = BinaryNodeHelper<LessThaner>(lhs, rhs, LessThaner());
				lhs = ToInt(ToVector(lhs));
			}
			break;
		case Token::LessThanOrEqualTo:
			if (lhs.type == Vector || rhs.type == Vector
				|| lhs.type == Map || rhs.type == Map
				|| lhs.type == Blob || rhs.type == Blob
				|| lhs.type == String || rhs.type == String)
			{
				LOGW("Comparison on non number value");
			}
			else
			{
				lhs = BinaryNodeHelper<LessThanOrEqualToer>(lhs, rhs, LessThanOrEqualToer());
				lhs = ToInt(ToVector(lhs));
			}
			break;
		case Token::GreaterThan:
			if (lhs.type == Vector || rhs.type == Vector
				|| lhs.type == Map || rhs.type == Map
				|| lhs.type == Blob || rhs.type == Blob
				|| lhs.type == String || rhs.type == String)
			{
				LOGW("Comparison on non number value");
			}
			else
			{
				lhs = BinaryNodeHelper<GreaterThaner>(lhs, rhs, GreaterThaner());
				lhs = ToInt(ToVector(lhs));
			}
			break;
		case Token::GreaterThanOrEqualTo:
			if (lhs.type == Vector || rhs.type == Vector
				|| lhs.type == Map || rhs.type == Map
				|| lhs.type == Blob || rhs.type == Blob
				|| lhs.type == String || rhs.type == String)
			{
				LOGW("Comparison on non number value");
			}
			else
			{
				lhs = BinaryNodeHelper<GreaterThanOrEqualToer>(lhs, rhs, GreaterThanOrEqualToer());
				lhs = ToInt(ToVector(lhs));
			}
			break;
		case Token::LogicalAnd:
		case Token::LogicalOr:
			if (lhs.type == Vector || rhs.type == Vector
				|| lhs.type == Map || rhs.type == Map
				|| lhs.type == Blob || rhs.type == Blob
				|| lhs.type == String || rhs.type == String)
			{
				LOGW("Comparison on non number value");
			}
			else
			{
				lhs = BinaryNodeHelper<Logicaler>(lhs, rhs, Logicaler());
				lhs = ToInt(ToVector(lhs));
			}
			break;
			break;
		default:
			LOGW("Invalid Binary operator type = " << node->binaryOperator.tokenType);
			break;
		}
		lastValue = lhs;
		return 0;
	}
	int Visit(UnaryOperatorNode *node)
	{
		if (node->right)
		{
			node->right->Walk(this);
		}
		switch (node->unaryOperator.tokenType)
		{
		case Token::Plus:
		{
			lastValue = BinaryNodeHelper<Positiver>(lastValue, lastValue, Positiver());
			break;
		}
		case Token::Minus:
		{
			lastValue = BinaryNodeHelper<Negator>(lastValue, lastValue, Negator());
			break;
		}
		case Token::LogicalNot:
		{
			lastValue = BinaryNodeHelper<LogicalNotter>(lastValue, lastValue, LogicalNotter());
			break;
		}
		case Token::Increment:
		{
			lastValue = BinaryNodeHelper<PreIncrementor>(lastValue, lastValue, PreIncrementor());
			//update the variable as well
			if (lastValue.pointer)
			{
				*lastValue.pointer = lastValue;
				//make sure the variable does not have a pointer
				lastValue.pointer->pointer = 0;
			}
			else
			{
				LOGW("PreIncrementing a r-value");
			}
			break;
		}
		case Token::Decrement:
		{
			lastValue = BinaryNodeHelper<PreDecrementer>(lastValue, lastValue, PreDecrementer());
			//update the variable as well
			if (lastValue.pointer)
			{
				*lastValue.pointer = lastValue;
				//make sure the variable does not have a pointer
				lastValue.pointer->pointer = 0;
			}
			else
			{
				LOGW("predecrementing a r-value");
			}
			break;
		}
		}
		return 0;
	}
	int Visit(PostExpressionNode *node)
	{
		node->left->Walk(this);
		switch (node->postOperator.tokenType)
		{
		case Token::Increment:
		{
			if (lastValue.pointer)
			{
				Value temp = lastValue;
				temp = BinaryNodeHelper<PostIncrementer>(temp, temp, PostIncrementer());
				*lastValue.pointer = temp;
				//make sure the variable does not have a pointer
				lastValue.pointer->pointer = 0;
				//the value is no longer a reference
				lastValue.pointer = 0;
			}
			break;
		}
		case Token::Decrement:
		{
			if (lastValue.pointer)
			{
				Value temp = lastValue;
				temp = BinaryNodeHelper<PostDecrementor>(temp, temp, PostDecrementor());
				*lastValue.pointer = temp;
				//make sure the variable does not have a pointer
				lastValue.pointer->pointer = 0;
				//the value is no longer a reference
				lastValue.pointer = 0;
			}
			break;
		}
		}
		return 0;
	}
	int Visit(MemberAccessNode *node)
	{
		node->Walk(this, false);
		return 0;
	}
	int Visit(CallNode *node)
	{
		std::vector<Value> args;
		for (unsigned i = 0; i < node->arguments.size(); ++i)
		{
			node->arguments[i]->Walk(this);
			args.push_back(lastValue);
		}
		std::string funcName = std::string(((NameReferenceNode*)node->left)->token.str, ((NameReferenceNode*)node->left)->token.length);
		if (functions.find(funcName) != functions.end())
		{
			lastValue = functions[funcName](args);
		}
		else
		{
			LOGW("Function " << funcName << " does not exist");
		}
		return 0;
	}
	int Visit(IndexNode *node)
	{
		Value lhs;
		Value rhs;
		if (node->left)
		{
			node->left->Walk(this);
			lhs = lastValue;
		}
		if (node->index)
		{
			node->index->Walk(this);
			rhs = lastValue;
		}
		if (lhs.type == Vector)
		{
			if (rhs.type == Integer)
			{
				lastValue = (*lhs.m_vector)[rhs.m_int];
				lastValue.pointer = &(*lhs.m_vector)[rhs.m_int];
				if (lhs.pointer)
				{
					lastValue = (*lhs.pointer->m_vector)[rhs.m_int];
					lastValue.pointer = &(*lhs.pointer->m_vector)[rhs.m_int];
				}
			}
			else if (rhs.type == Unsigned)
			{
				lastValue = (*lhs.m_vector)[rhs.m_unsigned];
				lastValue.pointer = &(*lhs.m_vector)[rhs.m_unsigned];
				if (lhs.pointer)
				{
					lastValue = (*lhs.pointer->m_vector)[rhs.m_unsigned];
					lastValue.pointer = &(*lhs.pointer->m_vector)[rhs.m_unsigned];
				}
			}
			else if (rhs.type == Short)
			{
				lastValue = (*lhs.m_vector)[rhs.m_short];
				lastValue.pointer = &(*lhs.m_vector)[rhs.m_short];
				if (lhs.pointer)
				{
					lastValue = (*lhs.pointer->m_vector)[rhs.m_short];
					lastValue.pointer = &(*lhs.pointer->m_vector)[rhs.m_short];
				}
			}
			else if (rhs.type == Char)
			{
				lastValue = (*lhs.m_vector)[rhs.m_char];
				lastValue.pointer = &(*lhs.m_vector)[rhs.m_char];
				if (lhs.pointer)
				{
					lastValue = (*lhs.pointer->m_vector)[rhs.m_char];
					lastValue.pointer = &(*lhs.pointer->m_vector)[rhs.m_char];
				}
			}
			else if (rhs.type == Float)
			{
				lastValue = (*lhs.m_vector)[rhs.m_float];
				lastValue.pointer = &(*lhs.m_vector)[rhs.m_float];
				if (lhs.pointer)
				{
					lastValue = (*lhs.pointer->m_vector)[rhs.m_float];
					lastValue.pointer = &(*lhs.pointer->m_vector)[rhs.m_float];
				}
			}
			else if (rhs.type == Double)
			{
				lastValue = (*lhs.m_vector)[rhs.m_double];
				lastValue.pointer = &(*lhs.m_vector)[rhs.m_double];
				if (lhs.pointer)
				{
					lastValue = (*lhs.pointer->m_vector)[rhs.m_double];
					lastValue.pointer = &(*lhs.pointer->m_vector)[rhs.m_double];
				}
			}
			else if (rhs.type == String)
			{
				//convert the string to double because it could be larger than a string or unsigned
				rhs = ToDouble(ToVector(rhs));
				lastValue = (*lhs.m_vector)[rhs.m_double];
				lastValue.pointer = &(*lhs.m_vector)[rhs.m_double];
				if (lhs.pointer)
				{
					lastValue = (*lhs.pointer->m_vector)[rhs.m_double];
					lastValue.pointer = &(*lhs.pointer->m_vector)[rhs.m_double];
				}
			}
			else
			{
				LOGW("Indexing vector with type " << rhs.type);
			}
		}
		else if (lhs.type == Map)
		{
			//need to use lhs.pointer
			lastValue = (*lhs.m_map)[rhs];
			lastValue.pointer = &(*lhs.m_map)[rhs];
			if (lhs.pointer)
			{
				lastValue = (*lhs.pointer->m_map)[rhs];
				lastValue.pointer = &(*lhs.pointer->m_map)[rhs];
			}
		}
		else
		{
			LOGW("Can only index into vectors and maps");
		}
		return 0;
	}
	//this is how to create variables
	int Visit(NameReferenceNode *node)
	{
		std::string varName = std::string(node->token.str, node->token.length);
		//note that if the variable is not already made this makes it an initalizes it
		lastValue = variables[varName];
		//it can be assigned to
		lastValue.pointer = &variables[varName];
		return 0;
	}
};

#endif