#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <iostream>
#include "tokenizer.h"
#include "types.h"
#include "logger.h"

class Visitor;
class MainNode;
class AbstractNode;
class ClassNode;
class StatementNode;
class ScopeNode;
class ParameterNode;
class FunctionNode;
class TypeNode;
class PointerTypeNode;
class ReferenceTypeNode;
class NamedTypeNode;
class FunctionTypeNode;
class LabelNode;
class GotoNode;
class ReturnNode;
class BreakNode;
class ContinueNode;
class ExpressionNode;
class IfNode;
class WhileNode;
class ForNode;
class LiteralNode;
class NameReferenceNode;
class BinaryOperatorNode;
class UnaryOperatorNode;
class PostExpressionNode;
class MemberAccessNode;
class CallNode;
class CastNode;
class IndexNode;

class Value;

class ValueHash
{
public:
	size_t operator()(const Value &rhs) const;
};

class Value
{
public:
	char m_char = 0;
	short m_short = 0;
	int m_int = 0;
	unsigned m_unsigned = 0;
	float m_float = 0;
	double m_double = 0;
	std::string m_string;
	std::vector<Value> *m_vector = 0;
	std::unordered_map<Value, Value, ValueHash> *m_map = 0;
	char *data = 0;
	unsigned size = 0;
	Types type = Types::Blob;
	//this is for variables/namereferences in the interpreter to be able to modify 
	//the value that 
	Value *pointer = 0;
};
bool operator==(const Value &lhs, const Value &rhs);

class AbstractNode
{
public:
	virtual ~AbstractNode() {};
	virtual void Walk(Visitor *visitor, bool visit = true) = 0;
	AbstractNode *parent = 0;
};


//main  = <main> <open paremtheses> (Parameter (<comma> Parameter)*)? <closed parenthese> Scope
class MainNode : public AbstractNode
{
public:
	void Walk(Visitor *visitor, bool visit = true) override;
	std::vector<NameReferenceNode*> parameters;
	ScopeNode *scope = 0;
};
MainNode *MainRule(std::vector<Token> &tokens, unsigned &index);

//DelimitedStatement = Return | <Break> | <Continue> | Var | Expression
//FreeStatement = If | While
//Statement = FreeStatement | DelimitedStatement <Semicolon>
class StatementNode : public AbstractNode
{
public:
	virtual void Walk(Visitor *visitor, bool visit = true) override;
};
StatementNode *DelimitedStatementRule(std::vector<Token> &tokens, unsigned &index);
StatementNode *FreeStatementRule(std::vector<Token> &tokens, unsigned &index);
StatementNode *StatementRule(std::vector<Token> &tokens, unsigned &index);


NameReferenceNode *ParameterRule(std::vector<Token> &tokens, unsigned &index);

//Scope = <OpenCurley> Statement* <CloseCurley>
class ScopeNode : public AbstractNode
{
public:
	std::vector<StatementNode*> statements;
	void Walk(Visitor *visitor, bool visit = true) override;
};
ScopeNode *ScopeRule(std::vector<Token> &tokens, unsigned &index);

//Return = <Return> (Expression) ?
class ReturnNode : public StatementNode
{
public:
	ExpressionNode * retrunValue = 0;
	void Walk(Visitor *visitor, bool visit = true) override;
};
ReturnNode *ReturnRule(std::vector<Token> &tokens, unsigned &index);

class ContinueNode : public StatementNode
{
public:
	void Walk(Visitor *visitor, bool visit = true) override;
};

class BreakNode : public StatementNode
{
public:
	void Walk(Visitor *visitor, bool visit = true) override;
};

//If = <If> GroupedExpression Scope Else ?
//Else = <Else> (If | Scope)
class IfNode : public StatementNode
{
public:
	ExpressionNode * condition = 0;
	ScopeNode *scope = 0;
	IfNode *elseIf = 0;
	void Walk(Visitor *visitor, bool visit = true) override;
};
IfNode *IfRule(std::vector<Token> &tokens, unsigned &index);
IfNode *ElseRule(std::vector<Token> &tokens, unsigned &index);

//While = <While> GroupedExpression Scope
class WhileNode : public StatementNode
{
public:
	ExpressionNode * condition = 0;
	ScopeNode *scope = 0;
	virtual void Walk(Visitor *visitor, bool visit = true) override;
};
WhileNode *WhileRule(std::vector<Token> &tokens, unsigned &index);


class ExpressionNode : public StatementNode
{
public:
	virtual void Walk(Visitor *visitor, bool visit = true) override;
};

// Literals, variables, or grouped expressions
//GroupedExpression = <OpenParentheses> Expression <CloseParentheses>
//Literal = <True> | <False> | <IntegerLiteral> | <FloatLiteral> | <StringLiteral> | <CharacterLiteral>
//NameReference = <Identifier>
class NameReferenceNode : public ExpressionNode
{
public:
	Token token;
	void Walk(Visitor *visitor, bool visit = true) override;
};
NameReferenceNode *NameReferenceRule(std::vector<Token> &tokens, unsigned &index);

//Value = Literal | NameReference | GroupedExpression
class LiteralNode : public ExpressionNode
{
public:
	Token token;
	void Walk(Visitor *visitor, bool visit = true) override;
	Value value;
};
LiteralNode *LiteralRule(std::vector<Token> &tokens, unsigned &index);
ExpressionNode *GroupedExpressionRule(std::vector<Token> &tokens, unsigned &index);
ExpressionNode *ValueRule(std::vector<Token> &tokens, unsigned &index);

// Right to left binary operators (note that Expression recurses into itself and is only optional, this preserves right to left)
//Expression = Expression1((<Assignment> | <AssignmentPlus> | <AssignmentMinus> | <AssignmentMultiply> | <AssignmentDivide> | <AssignmentModulo>) Expression) ?
ExpressionNode *ExpressionRule(std::vector<Token> &tokens, unsigned &index);

// Left to right binary operators
//Expression1 = Expression2(<LogicalOr> Expression2)*
//Expression2 = Expression3(<LogicalAnd> Expression3)*
//Expression3 = Expression4((<LessThan> | <GreaterThan> | <LessThanOrEqualTo> | <GreaterThanOrEqualTo> | <Equality> | <Inequality>) Expression4)*
//Expression4 = Expression5((<Plus> | <Minus>) Expression5)*
//Expression5 = Expression6((<Asterisk> | <Divide> | <Modulo>) Expression6)*
ExpressionNode *Expression1Rule(std::vector<Token> &tokens, unsigned &index);
ExpressionNode *Expression2Rule(std::vector<Token> &tokens, unsigned &index);
ExpressionNode *Expression3Rule(std::vector<Token> &tokens, unsigned &index);
ExpressionNode *Expression4Rule(std::vector<Token> &tokens, unsigned &index);
ExpressionNode *Expression5Rule(std::vector<Token> &tokens, unsigned &index);

// Right to left unary operators
//Expression6 = (<Plus> | <Minus> | <LogicalNot> | <Increment> | <Decrement>)* Expression7
ExpressionNode *Expression6Rule(std::vector<Token> &tokens, unsigned &index);

// Left to right binary operators
//Expression7 = Value(MemberAccess | Call | Index | <Increment> | <Decrement>)*
ExpressionNode *Expression7Rule(std::vector<Token> &tokens, unsigned &index);

class BinaryOperatorNode : public ExpressionNode
{
public:
	Token binaryOperator;
	ExpressionNode *left = 0;
	ExpressionNode *right = 0;
	void Walk(Visitor *visitor, bool visit = true) override;
};

class UnaryOperatorNode : public ExpressionNode
{
public:
	Token unaryOperator;
	ExpressionNode *right = 0;
	void Walk(Visitor *visitor, bool visit = true) override;
};

class PostExpressionNode : public ExpressionNode
{
public:
	ExpressionNode * left = 0;
	Token postOperator;
	virtual void Walk(Visitor *visitor, bool visit = true) override;
};

//MemberAccess = (<Dot> | <Arrow>) <Identifier>
class MemberAccessNode : public PostExpressionNode
{
public:
	void Walk(Visitor *visitor, bool visit = true) override;
};
MemberAccessNode *MemberAccessRule(std::vector<Token> &tokens, unsigned &index);

//Call = <OpenParentheses> (Expression(<Comma> Expression)*) ? <CloseParentheses>
class CallNode : public PostExpressionNode
{
public:
	std::vector<ExpressionNode*> arguments;
	void Walk(Visitor *visitor, bool visit = true) override;
};
CallNode *CallRule(std::vector<Token> &tokens, unsigned &index);

//Index = <OpenBracket> Expression <CloseBracket>
class IndexNode : public PostExpressionNode
{
public:
	ExpressionNode * index = 0;
	void Walk(Visitor *visitor, bool visit = true) override;
};
IndexNode *IndexRule(std::vector<Token> &tokens, unsigned &index);

class Visitor
{
public:
  virtual ~Visitor() {}
	virtual int Visit(__attribute__((unused))AbstractNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))MainNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))ClassNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))StatementNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))TypeNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))PointerTypeNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))ReferenceTypeNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))NamedTypeNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))FunctionTypeNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))ScopeNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))FunctionNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))LabelNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))GotoNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))ReturnNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))BreakNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))ContinueNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))ExpressionNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))IfNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))WhileNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))ForNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))LiteralNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))NameReferenceNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))BinaryOperatorNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))UnaryOperatorNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))PostExpressionNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))MemberAccessNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))CallNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))CastNode *node) { return 1; }
	virtual int Visit(__attribute__((unused))IndexNode *node) { return 1; }
};

class SetupLiterals : public Visitor
{
public:
	int Visit(MainNode *node)
	{
		node->Walk(this, false);
		return 0;
	}
	int Visit(LiteralNode *node)
	{
		std::stringstream ss;
		ss.str(node->token.str);
		switch (node->token.tokenType)
		{
		case Token::StringLiteral:
		{
			bool escaped = false;
			//ignore the double quotes
			for (unsigned i = 0; i < node->token.length; ++i)
			{
				char c = node->token.str[i];
				if (c == '\"' || c == '\'')
				{
					if (escaped)
					{
						node->value.m_string += c;
						escaped = false;
						continue;
					}
				}
				else if (c == 'n')
				{
					if (escaped)
					{
						node->value.m_string += "\n";
						escaped = false;
						continue;
					}
					else
					{
						node->value.m_string += c;
						continue;
					}
				}
				else if (c == 'r')
				{
					if (escaped)
					{
						node->value.m_string += "\r";
						escaped = false;
						continue;
					}
					else
					{
						node->value.m_string += c;
						continue;
					}
				}
				else if (c == 't')
				{
					if (escaped)
					{
						node->value.m_string += "\t";
						escaped = false;
						continue;
					}
					else
					{
						node->value.m_string += c;
						continue;
					}
				}
				else if (c == '\\')
				{
					if (escaped)
					{
						node->value.m_string += "\\";
						escaped = false;
						continue;
					}
					else
					{
						escaped = true;
						continue;
					}
				}
				else
				{
					node->value.m_string += c;
				}
			}
			node->value.type = String;
			break;
		}
		case Token::CharacterLiteral:
			node->value.m_char = node->token.str[1];
			if (node->token.str[1] == '\\')
			{
				switch (node->token.str[2])
				{
				case 'n':
					node->value.m_char = '\n';
					break;
				case 'r':
					node->value.m_char = '\r';
					break;
				case 't':
					node->value.m_char = '\t';
					break;
				case '\"':
					node->value.m_char = '\"';
					break;
				case '\'':
					node->value.m_char = '\'';
					break;
				case '\\':
					node->value.m_char = '\\';
					break;
				default:
					break;
				}
			}
			node->value.type = Char;
			break;
		case Token::IntegerLiteral:
			ss >> node->value.m_int;
			node->value.type = Integer;
			break;
		case Token::FloatLiteral:
			ss >> node->value.m_float;
			node->value.type = Float;
			break;
		case Token::True:
			node->value.m_int = 1;
			node->value.type = Integer;
			break;
		case Token::False:
			node->value.m_int = 0;
			node->value.type = Integer;
			break;
		default:
			break;
		}
		return 0;
	}
};

extern std::vector<AbstractNode*> all_nodes;

void FreeAbstractNodes();

template<typename T>
T *CreateAbstractNode()
{
	T *node = new T;
	all_nodes.push_back(node);
	return node;
}

#endif