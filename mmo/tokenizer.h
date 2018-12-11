#ifndef TOKINIZER_H
#define TOKINIZER_H

class Token
{
public:
	enum TokenType
	{
#define TOKEN(Name, Value) Name,
#include "Tokens.inl"
#undef TOKEN
		NumToken
	};
	TokenType tokenType = Invalid;
	unsigned length;
	const char *str;
};


class DfaState
{
public:
	DfaState();
	DfaState *links[129];
	Token::TokenType acceptingToken;
};

void ReadLanguageToken(DfaState *startingState, const char *stream, Token &out);

DfaState* CreateLanguageDfa();

#endif