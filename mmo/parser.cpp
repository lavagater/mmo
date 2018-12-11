#include "parser.h"
#include "logger.h"

static Token *lastAccepted;
std::vector<AbstractNode*> all_nodes;

size_t ValueHash::operator()(const Value &rhs) const
{
	size_t ret = 0;
	switch (rhs.type)
	{
	case Char:
		ret = std::hash<char>()(rhs.m_char);
		break;
	case Integer:
		ret = std::hash<int>()(rhs.m_int);
		break;
	case Unsigned:
		ret = std::hash<unsigned>()(rhs.m_unsigned);
		break;
	case Short:
		ret = std::hash<short>()(rhs.m_short);
		break;
	case Float:
		ret = std::hash<float>()(rhs.m_float);
		break;
	case Double:
		ret = std::hash<double>()(rhs.m_double);
		break;
	case String:
		ret = std::hash<std::string>()(rhs.m_string);
		break;
	case Blob:
		ret = std::hash<char *>()(rhs.data);
		break;
	}
	return ret;
}

bool operator==(const Value &lhs, const Value &rhs)
{
	if (lhs.type == rhs.type)
	{
		switch (rhs.type)
		{
		case Char:
			return lhs.m_char == rhs.m_char;
			break;
		case Integer:
			return lhs.m_int == rhs.m_int;
			break;
		case Unsigned:
			return lhs.m_unsigned == rhs.m_unsigned;
			break;
		case Short:
			return lhs.m_short == rhs.m_short;
			break;
		case Float:
			return lhs.m_float == rhs.m_float;
			break;
		case Double:
			return lhs.m_double == rhs.m_double;
			break;
		case String:
			return lhs.m_string == rhs.m_string;
			break;
		case Blob:
			return lhs.data == rhs.data;
			break;
		}
	}
	return false;
}

void FreeAbstractNodes()
{
	for (unsigned i = 0; i < all_nodes.size(); ++i)
	{
		delete all_nodes[i];
	}
	all_nodes.clear();
}

bool Accept(std::vector<Token>& tokens, Token::TokenType type, unsigned &index)
{
	if (index >= tokens.size())
	{
		return false;
	}
	if (tokens[index].tokenType == type)
	{
		lastAccepted = &tokens[index];
		index += 1;
		return true;
	}
	return false;
}

std::string ShowLine(std::vector<Token>& tokens, Token token)
{
	const char *start = tokens[0].str;
	const char *end = tokens.back().str;
	const char *begin = token.str;
	int num_lines = 2;
	while (begin != start && num_lines && token.str - begin <= 30)
	{
		begin -= 1;
		if (*begin == '\n')
		{
			num_lines -= 1;
		}
	}
	const char *walk = token.str;
	while (walk != end && *walk != '\n' && walk - token.str <= 30)
	{
		walk += 1;
	}
	return std::string(begin, walk - begin);
}

template <typename T>
T Expect(T state)
{
	if (state != 0)
	{
		return std::move(state);
	}

	LOGW("Bad thingy near " << std::string(lastAccepted->str, lastAccepted->length + 15));
	throw std::exception();
}

bool Expect(std::vector<Token>& tokens, Token::TokenType type, unsigned &index)
{
	if (index >= tokens.size())
	{
		throw std::exception();
	}
	if (tokens[index].tokenType == type)
	{
		lastAccepted = &tokens[index];
		index += 1;
		return true;
	}
	LOGW("Did not expect token " << std::string(tokens[index].str, tokens[index].length));
	LOGW(ShowLine(tokens, tokens[index]));
	throw std::exception();
}

//MemberAccess = <Dot> <Identifier>
MemberAccessNode *MemberAccessRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::Dot, index))
	{
		return 0;
	}
	Expect(tokens, Token::Identifier, index);
	MemberAccessNode *node = CreateAbstractNode<MemberAccessNode>();
	return node;
}

//Call = <OpenParentheses> (Expression(<Comma> Expression)*) ? <CloseParentheses>
CallNode *CallRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::OpenParentheses, index))
	{
		return 0;
	}
	CallNode *node = CreateAbstractNode<CallNode>();
	while (ExpressionNode *p = ExpressionRule(tokens, index))
	{
		node->arguments.push_back(p);
		if (!Accept(tokens, Token::Comma, index))
		{
			break;
		}
	}
	Expect(tokens, Token::CloseParentheses, index);
	return node;
}

//Index = <OpenBracket> Expression <CloseBracket>
IndexNode *IndexRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::OpenBracket, index))
	{
		return 0;
	}
	IndexNode *node = CreateAbstractNode<IndexNode>();
	node->index = Expect(ExpressionRule(tokens, index));
	Expect(tokens, Token::CloseBracket, index);
	return node;
}


//Expression = Expression1((<Assignment> | <AssignmentPlus> | <AssignmentMinus> | <AssignmentMultiply> | <AssignmentDivide> | <AssignmentModulo>) Expression) ?
ExpressionNode *ExpressionRule(std::vector<Token> &tokens, unsigned &index)
{
	ExpressionNode *node = Expression1Rule(tokens, index);
	if (node == 0)
	{
		return 0;
	}
	if (Accept(tokens, Token::Assignment, index) ||
		Accept(tokens, Token::AssignmentPlus, index) ||
		Accept(tokens, Token::AssignmentMinus, index) ||
		Accept(tokens, Token::AssignmentMultiply, index) ||
		Accept(tokens, Token::AssignmentDivide, index) ||
		Accept(tokens, Token::AssignmentModulo, index))
	{
		BinaryOperatorNode *binary = CreateAbstractNode<BinaryOperatorNode>();
		binary->binaryOperator = *lastAccepted;
		binary->left = node;
		binary->right = Expect(ExpressionRule(tokens, index));
		node = binary;
	}
	return node;
}


//Expression1 = Expression2(<LogicalOr> Expression2)*
ExpressionNode *Expression1Rule(std::vector<Token> &tokens, unsigned &index)
{
	ExpressionNode *node = Expression2Rule(tokens, index);
	if (node == 0)
	{
		return 0;
	}
	while (Accept(tokens, Token::LogicalOr, index))
	{
		BinaryOperatorNode *binary = CreateAbstractNode<BinaryOperatorNode>();
		binary->binaryOperator = *lastAccepted;
		binary->left = node;
		binary->right = Expect(Expression2Rule(tokens, index));
		node = binary;
	}
	return node;
}

//Expression2 = Expression3(<LogicalAnd> Expression3)*
ExpressionNode *Expression2Rule(std::vector<Token> &tokens, unsigned &index)
{
	ExpressionNode *node = Expression3Rule(tokens, index);
	if (node == 0)
	{
		return 0;
	}
	while (Accept(tokens, Token::LogicalAnd, index))
	{
		BinaryOperatorNode *binary = CreateAbstractNode<BinaryOperatorNode>();
		binary->binaryOperator = *lastAccepted;
		binary->left = node;
		binary->right = Expect(Expression3Rule(tokens, index));
		node = binary;
	}
	return node;
}

//Expression3 = Expression4((<LessThan> | <GreaterThan> | <LessThanOrEqualTo> | <GreaterThanOrEqualTo> | <Equality> | <Inequality>) Expression4)*
ExpressionNode *Expression3Rule(std::vector<Token> &tokens, unsigned &index)
{
	ExpressionNode *node = Expression4Rule(tokens, index);
	if (node == 0)
	{
		return 0;
	}
	while (Accept(tokens, Token::LessThan, index) ||
		Accept(tokens, Token::GreaterThan, index) ||
		Accept(tokens, Token::LessThanOrEqualTo, index) ||
		Accept(tokens, Token::GreaterThanOrEqualTo, index) ||
		Accept(tokens, Token::Equality, index) ||
		Accept(tokens, Token::Inequality, index))
	{
		BinaryOperatorNode *binary = CreateAbstractNode<BinaryOperatorNode>();
		binary->binaryOperator = *lastAccepted;
		binary->left = node;
		binary->right = Expect(Expression4Rule(tokens, index));
		node = binary;
	}
	return node;
}

//Expression4 = Expression5((<Plus> | <Minus>) Expression5)*
ExpressionNode *Expression4Rule(std::vector<Token> &tokens, unsigned &index)
{
	ExpressionNode *node = Expression5Rule(tokens, index);
	if (node == 0)
	{
		return 0;
	}
	while (Accept(tokens, Token::Plus, index) ||
		Accept(tokens, Token::Minus, index))
	{
		BinaryOperatorNode *binary = CreateAbstractNode<BinaryOperatorNode>();
		binary->binaryOperator = *lastAccepted;
		binary->left = node;
		binary->right = Expect(Expression5Rule(tokens, index));
		node = binary;
	}
	return node;
}

//Expression5 = Expression6((<Asterisk> | <Divide> | <Modulo>) Expression6)*
ExpressionNode *Expression5Rule(std::vector<Token> &tokens, unsigned &index)
{
	ExpressionNode *node = Expression6Rule(tokens, index);
	if (node == 0)
	{
		return 0;
	}
	while (Accept(tokens, Token::Asterisk, index) ||
		Accept(tokens, Token::Divide, index) ||
		Accept(tokens, Token::Modulo, index))
	{
		BinaryOperatorNode *binary = CreateAbstractNode<BinaryOperatorNode>();
		binary->binaryOperator = *lastAccepted;
		binary->left = node;
		binary->right = Expect(Expression6Rule(tokens, index));
		node = binary;
	}
	return node;
}

//Expression6 = (<Plus> | <Minus> | <LogicalNot> | <Increment> | <Decrement>)* Expression7
ExpressionNode *Expression6Rule(std::vector<Token> &tokens, unsigned &index)
{
	std::vector<Token> operators;
	while (Accept(tokens, Token::Plus, index) ||
		Accept(tokens, Token::Minus, index) ||
		Accept(tokens, Token::LogicalNot, index) ||
		Accept(tokens, Token::Increment, index) ||
		Accept(tokens, Token::Decrement, index))
	{
		operators.push_back(*lastAccepted);
	}
	ExpressionNode *right = Expression7Rule(tokens, index);
	if (right == 0)
	{
		if (operators.size() != 0)
		{
			//throw
			Expect(0);
		}
		return 0;
	}
	if (operators.size() == 0)
	{
		return right;
	}
	UnaryOperatorNode *node = CreateAbstractNode<UnaryOperatorNode>();
	node->right = right;
	node->unaryOperator = operators[operators.size() - 1];
	for (int i = operators.size() - 2; i >= 0; --i)
	{
		UnaryOperatorNode *temp = node;
		node = CreateAbstractNode<UnaryOperatorNode>();
		node->unaryOperator = operators[i];
		node->right = temp;
	}
	return node;
}

//Expression7 = Value(MemberAccess | Call | Index | <Increment> | <Decrement>)*
ExpressionNode *Expression7Rule(std::vector<Token> &tokens, unsigned &index)
{
	ExpressionNode *node = ValueRule(tokens, index);
	if (node == 0)
	{
		return 0;
	}

	while (1)
	{
		MemberAccessNode *mem = MemberAccessRule(tokens, index);
		if (mem)
		{
			mem->left = node;
			node = mem;
			continue;
		}
		CallNode *call = CallRule(tokens, index);
		if (call)
		{
			call->left = node;
			node = call;
			continue;
		}
		IndexNode *ind = IndexRule(tokens, index);
		if (ind)
		{
			ind->left = node;
			node = ind;
			continue;
		}
		if (Accept(tokens, Token::Increment, index) || Accept(tokens, Token::Decrement, index))
		{
			PostExpressionNode *post = CreateAbstractNode<PostExpressionNode>();
			post->postOperator = *lastAccepted;
			post->left = node;
			node = post;
			continue;
		}
		return node;
	}
}

//Value = Literal | NameReference+ | GroupedExpression
ExpressionNode *ValueRule(std::vector<Token> &tokens, unsigned &index)
{
	ExpressionNode *node = 0;
	if (node = LiteralRule(tokens, index))
	{
	}
	else if (node = GroupedExpressionRule(tokens, index))
	{
	}
	else if (node = NameReferenceRule(tokens, index))
	{
		while (ExpressionNode *temp = NameReferenceRule(tokens, index))
		{
			node = temp;
		}
	}
	return node;
}

//Literal = <True> | <False> | <IntegerLiteral> | <FloatLiteral> | <StringLiteral> | <CharacterLiteral>
LiteralNode *LiteralRule(std::vector<Token> &tokens, unsigned &index)
{
	if (Accept(tokens, Token::True, index) ||
		Accept(tokens, Token::False, index) ||
		Accept(tokens, Token::IntegerLiteral, index) ||
		Accept(tokens, Token::FloatLiteral, index) ||
		Accept(tokens, Token::StringLiteral, index) ||
		Accept(tokens, Token::CharacterLiteral, index))
	{
		LiteralNode *node = CreateAbstractNode<LiteralNode>();
		node->token = *lastAccepted;
		return node;
	}
	else
	{
		return 0;
	}
}
//GroupedExpression = <OpenParentheses> Expression <CloseParentheses>
ExpressionNode *GroupedExpressionRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::OpenParentheses, index))
	{
		return false;
	}
	ExpressionNode *node = Expect(ExpressionRule(tokens, index));
	Expect(tokens, Token::CloseParentheses, index);
	return node;
}

//NameReference = <Identifier>
NameReferenceNode *NameReferenceRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::Identifier, index))
	{
		return 0;
	}
	NameReferenceNode *node = CreateAbstractNode<NameReferenceNode>();
	node->token = *lastAccepted;
	return node;
}

//While = <While> GroupedExpression Scope
WhileNode *WhileRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::While, index))
	{
		return 0;
	}
	WhileNode *node = CreateAbstractNode<WhileNode>();
	node->condition = Expect(GroupedExpressionRule(tokens, index));
	node->scope = Expect(ScopeRule(tokens, index));

	return node;
}

//If = <If> GroupedExpression Scope Else ?
IfNode *IfRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::If, index))
	{
		return 0;
	}
	IfNode *node = CreateAbstractNode<IfNode>();
	node->condition = Expect(GroupedExpressionRule(tokens, index));
	node->scope = Expect(ScopeRule(tokens, index));
	node->elseIf = ElseRule(tokens, index);

	return node;
}
//Else = <Else> (If | Scope)
IfNode *ElseRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::Else, index))
	{
		return 0;
	}
	IfNode *node = CreateAbstractNode<IfNode>();
	if (node->elseIf = IfRule(tokens, index))
	{
	}
	else
	{
		node->scope = Expect(ScopeRule(tokens, index));
	}

	return node;
}


//Return = <Return> (Expression) ?
ReturnNode *ReturnRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::Return, index))
	{
		return 0;
	}
	ReturnNode *node = CreateAbstractNode<ReturnNode>();
	node->retrunValue = ExpressionRule(tokens, index);

	return node;
}

//Scope = <OpenCurley> Statement* <CloseCurley>
ScopeNode *ScopeRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::OpenCurley, index))
	{
		return 0;
	}
	ScopeNode *scope = CreateAbstractNode<ScopeNode>();

	while (StatementNode *statement = StatementRule(tokens, index))
	{
		scope->statements.push_back(statement);
	}

	Expect(tokens, Token::CloseCurley, index);

	return scope;
}

//Parameter = NameReference+
NameReferenceNode *ParameterRule(std::vector<Token> &tokens, unsigned &index)
{
	NameReferenceNode *node = NameReferenceRule(tokens, index);
	if (node == 0)
	{
		return 0;
	}
	while (NameReferenceNode *temp = NameReferenceRule(tokens, index))
	{
		node = temp;
	}
	node->token = *lastAccepted;
	return node;
}

//Statement = FreeStatement | DelimitedStatement <Semicolon>
StatementNode *StatementRule(std::vector<Token> &tokens, unsigned &index)
{
	StatementNode *node = 0;
	if (node = FreeStatementRule(tokens, index))
	{
	}
	else if (node = DelimitedStatementRule(tokens, index))
	{
		Expect(tokens, Token::Semicolon, index);
	}
	return node;
}

//FreeStatement = If | While
StatementNode *FreeStatementRule(std::vector<Token> &tokens, unsigned&index)
{
	StatementNode *node = 0;
	if (node = IfRule(tokens, index))
	{
	}
	else if (node = WhileRule(tokens, index))
	{
	}
	return node;
}

//DelimitedStatement = Return | <Break> | <Continue> | Expression
StatementNode *DelimitedStatementRule(std::vector<Token> &tokens, unsigned &index)
{
	StatementNode *node = 0;
	if (Accept(tokens, Token::Break, index))
	{
		node = CreateAbstractNode<BreakNode>();
	}
	else if (Accept(tokens, Token::Continue, index))
	{
		node = CreateAbstractNode<ContinueNode>();
	}
	else if (node = ReturnRule(tokens, index))
	{
	}
	else if (node = ExpressionRule(tokens, index))
	{
	}
	return node;
}

//main  = <main> <open paremtheses> (Parameter (<comma> Parameter)*)? <closed parenthese> Scope
MainNode *MainRule(std::vector<Token> &tokens, unsigned &index)
{
	if (!Accept(tokens, Token::Main, index))
	{
		return 0;
	}
	Expect(tokens, Token::OpenParentheses, index);
	MainNode *node = CreateAbstractNode<MainNode>();
	while (NameReferenceNode *p = ParameterRule(tokens, index))
	{
		node->parameters.push_back(p);
		if (!Accept(tokens, Token::Comma, index))
		{
			break;
		}
	}
	Expect(tokens, Token::CloseParentheses, index);

	node->scope = Expect(ScopeRule(tokens, index));

	return node;
}

void MainNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	for (unsigned i = 0; i < parameters.size(); ++i)
	{
		if (parameters[i])
		{
			parameters[i]->Walk(visitor);
		}
	}
	if (scope)
	{
		scope->Walk(visitor);
	}
}
void StatementNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;
}
void ScopeNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	for (unsigned i = 0; i < statements.size(); ++i)
	{
		if (statements[i])
		{
			statements[i]->Walk(visitor);
		}
	}
}
void ReturnNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	StatementNode::Walk(visitor, false);

	if (retrunValue)
	{
		retrunValue->Walk(visitor, false);
	}
}
void ContinueNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	StatementNode::Walk(visitor, false);
}
void BreakNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	StatementNode::Walk(visitor, false);
}
void IfNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	StatementNode::Walk(visitor, false);

	if (condition)
	{
		condition->Walk(visitor, false);
	}
	if (scope)
	{
		scope->Walk(visitor, false);
	}
	if (elseIf)
	{
		elseIf->Walk(visitor, false);
	}
}
void WhileNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	StatementNode::Walk(visitor, false);

	if (condition)
	{
		condition->Walk(visitor);
	}
	if (scope)
	{
		scope->Walk(visitor);
	}
}
void NameReferenceNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	ExpressionNode::Walk(visitor, false);
}
void LiteralNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	StatementNode::Walk(visitor, false);
}
void ExpressionNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	StatementNode::Walk(visitor, false);
}
void BinaryOperatorNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	ExpressionNode::Walk(visitor, false);

	if (left)
	{
		left->Walk(visitor);
	}
	if (right)
	{
		right->Walk(visitor);
	}
}
void UnaryOperatorNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	ExpressionNode::Walk(visitor, false);

	if (right)
	{
		right->Walk(visitor);
	}
}
void PostExpressionNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;

	ExpressionNode::Walk(visitor, false);

	if (left)
	{
		left->Walk(visitor);
	}
}
void MemberAccessNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;
	ExpressionNode::Walk(visitor, false);
}
void CallNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;
	ExpressionNode::Walk(visitor, false);
	for (unsigned i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i])
		{
			arguments[i]->Walk(visitor);
		}
	}
}
void IndexNode::Walk(Visitor *visitor, bool visit)
{
	if (visit && visitor->Visit(this) == 0)
		return;
	PostExpressionNode::Walk(visitor, false);
	if (index)
	{
		index->Walk(visitor);
	}
}
