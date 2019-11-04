#ifndef QUERY_H
#define QUERY_H

#include "database.h"
#include "parser.h"
#include "tokenizer.h"
#include "interpreter.h"
#include <vector>
#include <string>

#define STRINGIZE(...) #__VA_ARGS__

class Query
{
public:
	std::vector<char *> data;
	void FreeData();
	Query(Database &db);
	~Query();
	Database &db;
	//the interpreter is a pointer so that it can stay alive after the compile function call
	Interpreter *interpreter;
	bool Compile(std::string code, std::vector<Value> &parameters, Value &);
	static int PackValue(char *buffer,Value value);
	//functions for scripting language
	Value GetDatabase(std::vector<Value> args);
	Value GetDatabaseRange(std::vector<Value> args);
	Value CreateDatabase(std::vector<Value> args);
	Value FindDatabase(std::vector<Value> args, Interpreter &interpreter);
	Value FindRangeDatabase(std::vector<Value> args, Interpreter &interpreter);
	Value DeleteDatabase(std::vector<Value> args);
	Value SetDatabase(std::vector<Value> args);
	Value SetDatabaseRange(std::vector<Value> args);
};

#endif