#include "tokenizer.h"
#include <cstring>

DfaState::DfaState()
{
	acceptingToken = Token::Invalid;
	for (unsigned i = 0; i < sizeof(links) / sizeof(links[0]); ++i)
	{
		links[i] = 0;
	}
}

DfaState *AddState(Token::TokenType acceptingState)
{
	DfaState *ret = new DfaState();
	ret->acceptingToken = acceptingState;
	return ret;
}

void AddEdge(DfaState *from, DfaState *to, char c)
{
	from->links[(int)c] = to;
}

void AddDefaultEdge(DfaState *from, DfaState *to)
{
	from->links[sizeof(from->links) / sizeof(from->links[0]) - 1] = to;
}

void ReadToken(DfaState *startingState, const char *stream, Token &out)
{
	DfaState *lastAccepted = 0;
	int index = 0;
	int i;
	for (i = 0; stream[i]; ++i)
	{
		if (startingState->links[(int)stream[i]] == 0)
		{
			if (startingState->links[sizeof(startingState->links) / sizeof(startingState->links[0]) - 1] != 0)
			{
				startingState = startingState->links[sizeof(startingState->links) / sizeof(startingState->links[0]) - 1];
				if (startingState->acceptingToken != 0)
				{
					lastAccepted = startingState;
					index = i;
				}
			}
			else
				break;
		}
		else
		{
			startingState = startingState->links[(int)stream[i]];
			if (startingState->acceptingToken != 0)
			{
				lastAccepted = startingState;
				index = i;
			}
		}
	}
	if (lastAccepted == 0)
	{
		out.tokenType = Token::Invalid;
		out.str = stream;
		out.length = i;
	}
	else
	{
		out.tokenType = lastAccepted->acceptingToken;
		out.str = stream;
		out.length = index + 1;
	}
}

void DeleteStateAndChildren(DfaState *root)
{
	if (root == 0)
	{
		return;
	}
	for (unsigned i = 0; i < sizeof(root->links) / sizeof(root->links[0]); ++i)
	{
		DeleteStateAndChildren(root->links[0]);
	}
	delete root;
}

static void CheckForKeyWord(Token &out, const char *str, Token::TokenType type)
{
	unsigned len = strlen(str);
	if (len == out.length && memcmp(out.str, str, out.length) == 0)
	{
		out.tokenType = type;
	}
}

void ReadLanguageToken(DfaState *startingState, const char *stream, Token &out)
{
	ReadToken(startingState, stream, out);
	if (out.tokenType == Token::Identifier)
	{
#define TOKEN(Name, Value) CheckForKeyWord(out, Value, Token::Name);
#include "TokenKeywords.inl"
#undef TOKEN
	}
}

DfaState *CreateOrAdd(DfaState *node, char c)
{
	DfaState *ret;
	if (node->links[(int)c] == 0)
	{
		ret = new DfaState;
		ret->acceptingToken = Token::Invalid;
		node->links[(int)c] = ret;
	}
	else
	{
		ret = node->links[(int)c];
	}
	return ret;
}

//helper function for identifiers to the dfa
static void AddIdentifierRule(DfaState *dfa)
{
	DfaState *first = AddState(Token::Identifier);
	//[a-zA-Z_]
	for (char c = 'a'; c <= 'z'; ++c)
	{
		AddEdge(dfa, first, c);
		AddEdge(first, first, c);
	}
	for (char c = 'A'; c <= 'Z'; ++c)
	{
		AddEdge(dfa, first, c);
		AddEdge(first, first, c);
	}
	AddEdge(dfa, first, '_');
	AddEdge(first, first, '_');
	//numbers
	for (char c = '0'; c <= '9'; ++c)
	{
		AddEdge(first, first, c);
	}
}

static void AddIntegerRule(DfaState *dfa)
{
	DfaState *first = AddState(Token::IntegerLiteral);
	for (char c = '0'; c <= '9'; ++c)
	{
		AddEdge(dfa, first, c);
		AddEdge(first, first, c);
	}
	//deal with U,UL,LU,LL
	DfaState *second = AddState(Token::IntegerLiteral);
	AddEdge(first, second, 'U');
	DfaState *third = AddState(Token::IntegerLiteral);
	AddEdge(first, third, 'L');
	DfaState *fourth = AddState(Token::IntegerLiteral);
	AddEdge(second, fourth, 'L');
	AddEdge(third, fourth, 'L');
}

//this assumes that integer has already been added
static void AddFloatingPointRule(DfaState *dfa)
{
	DfaState *num = dfa->links['0'];
	DfaState *decimal = AddState(Token::Invalid);
	AddEdge(num, decimal, '.');
	DfaState *floating = AddState(Token::FloatLiteral);
	for (char c = '0'; c <= '9'; ++c)
	{
		AddEdge(decimal, floating, c);
		AddEdge(floating, floating, c);
	}
	//do eponetial
	DfaState *e = AddState(Token::Invalid);
	AddEdge(floating, e, 'e');
	AddEdge(floating, e, 'E');
	DfaState *plus = AddState(Token::Invalid);
	DfaState *minus = AddState(Token::Invalid);
	AddEdge(e, plus, '+');
	AddEdge(e, minus, '-');
	DfaState *expo = AddState(Token::FloatLiteral);
	for (char c = '0'; c <= '9'; ++c)
	{
		AddEdge(e, expo, c);
		AddEdge(expo, expo, c);
		AddEdge(plus, expo, c);
		AddEdge(minus, expo, c);
	}
	//do ending f
	DfaState *f = AddState(Token::FloatLiteral);
	AddEdge(expo, f, 'f');
	AddEdge(floating, f, 'f');
}

static void AddStringRule(DfaState *dfa)
{
	DfaState *start = AddState(Token::Invalid);
	DfaState *escape = AddState(Token::Invalid);
	DfaState *string = AddState(Token::Invalid);
	DfaState *end = AddState(Token::StringLiteral);
	AddEdge(dfa, start, '"');
	AddEdge(start, end, '"');
	AddEdge(start, escape, '\\');
	AddDefaultEdge(start, string);
	AddDefaultEdge(string, string);
	AddEdge(escape, string, 'n');
	AddEdge(escape, string, 'r');
	AddEdge(escape, string, 't');
	AddEdge(escape, string, '"');
	AddEdge(escape, string, '\'');
	AddEdge(string, escape, '\\');
	AddEdge(string, end, '"');
}

static void AddCharRule(DfaState *dfa)
{
	DfaState *start = AddState(Token::Invalid);
	DfaState *escape = AddState(Token::Invalid);
	DfaState *middle = AddState(Token::Invalid);
	DfaState *end = AddState(Token::CharacterLiteral);
	AddEdge(dfa, start, '\'');
	AddEdge(start, end, '\'');
	AddEdge(start, escape, '\\');
	AddDefaultEdge(start, middle);
	AddEdge(escape, middle, 'n');
	AddEdge(escape, middle, 'r');
	AddEdge(escape, middle, 't');
	AddEdge(escape, middle, '"');
	AddEdge(escape, middle, '\'');
	AddEdge(middle, end, '\'');
}

static void AddWhiteSpaceRule(DfaState *dfa)
{
	DfaState *white = AddState(Token::Whitespace);
	AddEdge(dfa, white, ' ');
	AddEdge(dfa, white, '\t');
	AddEdge(dfa, white, '\n');
	AddEdge(dfa, white, '\r');
	AddEdge(white, white, ' ');
	AddEdge(white, white, '\t');
	AddEdge(white, white, '\n');
	AddEdge(white, white, '\r');
}

//assume that symbols have already been done
static void AddSingleLineCommentRule(DfaState *dfa)
{
	//.*(\r|\n|\0)
	DfaState *slash1 = dfa->links['/'];
	DfaState *slash2 = AddState(Token::Invalid);
	DfaState *anything = AddState(Token::Invalid);
	DfaState *end = AddState(Token::SingleLineComment);
	AddEdge(dfa, slash1, '/');
	AddEdge(slash1, slash2, '/');
	AddEdge(slash2, end, '\n');
	AddEdge(slash2, end, '\r');
	AddEdge(slash2, end, '\0');
	AddDefaultEdge(slash2, anything);
	AddDefaultEdge(anything, anything);
	AddEdge(anything, end, '\n');
	AddEdge(anything, end, '\r');
	AddEdge(anything, end, '\0');
}

//assume that symbols have already been done
static void AddMultiLineCommentRule(DfaState *dfa)
{
	DfaState *slash = dfa->links['/'];
	DfaState *star = AddState(Token::Invalid);
	DfaState *stuff = AddState(Token::Invalid);
	DfaState *endstar = AddState(Token::Invalid);
	DfaState *endslash = AddState(Token::MultiLineComment);
	AddEdge(slash, star, '*');
	AddEdge(star, endstar, '*');
	AddDefaultEdge(star, stuff);
	AddDefaultEdge(stuff, stuff);
	AddEdge(stuff, endstar, '*');
	AddDefaultEdge(endstar, stuff);
	AddEdge(endstar, endslash, '/');
}

static void AddOperatorRule(DfaState *dfa, const char *str, Token::TokenType state)
{
	int len = strlen(str);
	DfaState *next = dfa;
	for (int i = 0; i < len; ++i)
	{
		next = CreateOrAdd(next, str[i]);
	}
	next->acceptingToken = state;
}

DfaState* CreateLanguageDfa()
{
	DfaState *dfa = new DfaState();

	//do the include trick
#define TOKEN(Name, Value) AddOperatorRule(dfa, Value, Token::Name);
#include "TokenSymbols.inl"
#undef TOKEN
	AddIdentifierRule(dfa);
	AddIntegerRule(dfa);
	AddFloatingPointRule(dfa);
	AddStringRule(dfa);
	AddCharRule(dfa);
	AddWhiteSpaceRule(dfa);
	AddSingleLineCommentRule(dfa);
	AddMultiLineCommentRule(dfa);
	return dfa;
}
