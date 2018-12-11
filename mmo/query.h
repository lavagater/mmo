#ifndef QUERY_H
#define QUERY_H

#include "database.h"
#include "Parser.h"
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
	Interpreter interpreter;
	bool Compile(std::string code, char *data);
	//functions for scripting language
	Value GetDatabase(std::vector<Value> args);
	Value CreateDatabase(std::vector<Value> args);
	Value FindDatabase(std::vector<Value> args);
	Value DeleteDatabase(std::vector<Value> args);
	Value SetDatabase(std::vector<Value> args);
};

#endif