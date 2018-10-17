#include "parser.h"


ParserContext::ParserContext (TokenStream ts, std::string errorFileHandle)
: ts (ts), ofp (errorFileHandle)
{
}

TokenInfo ParserContext::Current () const { return ts.Current (); }
TokenInfo ParserContext::Advance () { return ts.Advance (); }


void ParserContext::LogError (std::string str) { fmt::print (ofp.FP (), "{}\n", str); }

PascalParser::PascalParser (ParserContext &context) : pc (context) {}

void PascalParser::Parse ()
{
	ProgramStatement ();
	Match (TokenType::END_FILE);
}
void PascalParser::Match (TokenType tt)
{
	using namespace std::string_literals;
	if (tt == pc.Current ().type && tt != TokenType::END_FILE)
	{
		pc.Advance ();
		int a = 456;
	}
	else if (tt == pc.Current ().type && tt == TokenType::END_FILE)
	{
		HaltParse ();
		int a = 45645;
	}
	else if (tt != pc.Current ().type)
	{
		pc.LogError ("SE: Expected "s + tt + ", Recieved "s + pc.Current ().type +
		             "\n\t Error at line " + std::to_string (pc.Current ().line_location) + ":" +
		             std::to_string (pc.Current ().column_location));
		pc.Advance ();
	}
}

void PascalParser::HaltParse ()
{
	// stop i guess?
}

void PascalParser::Synch (std::vector<TokenType> set)
{
	TokenType tt = pc.Current ().type;
	bool found = false;
	while (!found)
	{
		for (auto &s : set)
			if (s == tt) found = true;
		tt = pc.Advance ().type;
	}
}

void PascalParser::ProgramStatement ()
{
	Match (TokenType::PROGRAM);
	Match (TokenType::ID);
	Match (TokenType::PAREN_OPEN);
	IdentifierList ();
	Match (TokenType::PAREN_CLOSE);
	Match (TokenType::SEMICOLON);
	ProgramStatementFactored ();
}

void PascalParser::IdentifierList ()
{
	Match (TokenType::ID);
	IdentifierListPrime ();
}
void PascalParser::Declarations ()
{
	Match (TokenType::VARIABLE);
	Match (TokenType::ID);
	Match (TokenType::COLON);
	Type ();
	Match (TokenType::SEMICOLON);
	DeclarationsPrime ();
}
void PascalParser::SubprogramDeclarations ()
{
	SubprogramDeclaration ();
	Match (TokenType::SEMICOLON);
	SubprogramDeclarationsPrime ();
}
void PascalParser::CompoundStatement ()
{
	Match (TokenType::BEGIN);
	CompoundStatementFactored ();
}
void PascalParser::Type ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::ARRAY):
			Match (TokenType::ARRAY);
			Match (TokenType::BRACKET_OPEN);
			Match (TokenType::NUM);
			Match (TokenType::DOT_DOT);
			Match (TokenType::NUM);
			Match (TokenType::BRACKET_CLOSE);
			Match (TokenType::OF);
			StandardType ();
			break;
		case (TokenType::STANDARD_TYPE):
			StandardType ();

			break;
	}
}
void PascalParser::StandardType ()
{
	Match (TokenType::STANDARD_TYPE);
	// switch (pc.Current ().type)
	// {
	// 	case (TokenType::STANDARD_TYPE):
	// 		Match (TokenType::INTEGER);

	// 		break;
	// 	case (TokenType::REAL):
	// 		Match (TokenType::REAL);
	// 		break;
	// }
}
void PascalParser::SubprogramDeclaration ()
{
	SubprogramHead ();
	SubprogramDeclarationFactored ();
}
void PascalParser::SubprogramHead ()
{
	Match (TokenType::PROCEDURE);
	Match (TokenType::ID);
	SubprogramHeadFactored ();
}
void PascalParser::Arguments ()
{
	Match (TokenType::PAREN_OPEN);
	ParameterList ();
	Match (TokenType::PAREN_CLOSE);
}
void PascalParser::ParameterList ()
{
	Match (TokenType::ID);
	Match (TokenType::COLON);
	Type ();
	ParameterListPrime ();
}
void PascalParser::OptionalStatements () { StatementList (); }
void PascalParser::StatementList ()
{
	Statement ();
	StatementListPrime ();
}
void PascalParser::Statement ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
			Match (TokenType::VARIABLE);
			Match (TokenType::ASSIGNOP);
			Expression ();
			break;
		case (TokenType::WHILE):
			Match (TokenType::WHILE);
			Expression ();
			Match (TokenType::DO);
			Statement ();
			break;
		case (TokenType::BEGIN):
			Match (TokenType::BEGIN);
			StatementFactoredTwo ();
			break;
		case (TokenType::IF):
			Match (TokenType::IF);
			Expression ();
			Match (TokenType::THEN);
			Statement ();
			StatementFactoredOne ();
			break;
		case (TokenType::CALL):
			ProcedureStatement ();
			break;
	}
}
void PascalParser::Variable ()
{
	Match (TokenType::ID);
	VariableFactored ();
}
void PascalParser::Expression ()
{
	SimpleExpression ();
	ExpressionFactored ();
}
void PascalParser::ProcedureStatement ()
{
	Match (TokenType::CALL);
	Match (TokenType::ID);
	ProcedureStatmentFactored ();
}
void PascalParser::ExpressionList ()
{
	Expression ();
	ExpressionListPrime ();
}
void PascalParser::SimpleExpression ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
		case (TokenType::PAREN_OPEN):
		case (TokenType::NUM):
		case (TokenType::NOT):
			Term ();
			SimpleExpressionPrime ();
			break;
		case (TokenType::SIGN):
			Match (TokenType::SIGN);
			Term ();
			SimpleExpressionPrime ();

			break;
	}
}
void PascalParser::Term ()
{
	Factor ();
	TermPrime ();
}
void PascalParser::Sign () { Match (TokenType::SIGN); }
void PascalParser::Factor ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
			Match (TokenType::ID);
			break;
		case (TokenType::NUM):
			Match (TokenType::NUM);
			break;
		case (TokenType::PAREN_OPEN):
			Match (TokenType::PAREN_OPEN);
			Expression ();
			Match (TokenType::PAREN_CLOSE);
			break;
		case (TokenType::NOT):
			Match (TokenType::NOT);
			Factor ();
			break;
	}
}
void PascalParser::IdentifierListPrime ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::COMMA):
			Match (TokenType::COMMA);
			Match (TokenType::ID);
			IdentifierListPrime ();
			break;
	}
	// e-prod
}
void PascalParser::DeclarationsPrime ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
			Match (TokenType::VARIABLE);
			Match (TokenType::ID);
			Match (TokenType::COLON);
			Type ();
			Match (TokenType::SEMICOLON);
			DeclarationsPrime ();
			break;
	}
	// e-prod
}
void PascalParser::SubprogramDeclarationsPrime ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROCEDURE):
			SubprogramDeclaration ();
			Match (TokenType::SEMICOLON);
			SubprogramDeclarationsPrime ();
			break;
	}
	// e-prod
}
void PascalParser::ParameterListPrime ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::SEMICOLON):
			Match (TokenType::SEMICOLON);
			Match (TokenType::ID);
			Match (TokenType::COLON);
			Type ();
			ParameterListPrime ();
			break;
	}
	// e-prod
}
void PascalParser::StatementListPrime ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::SEMICOLON):
			Match (TokenType::SEMICOLON);
			Statement ();
			StatementListPrime ();
			break;
	}
}
void PascalParser::ExpressionListPrime ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::COMMA):
			Match (TokenType::COMMA);
			Expression ();
			ExpressionListPrime ();
			break;
	}
}
void PascalParser::SimpleExpressionPrime ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::ADDOP):
			Match (TokenType::ADDOP);
			Term ();
			SimpleExpressionPrime ();
			break;
	}
}
void PascalParser::TermPrime ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::MULOP):
			Match (TokenType::MULOP);
			Factor ();
			TermPrime ();
			break;
	}
}
void PascalParser::ProgramStatementFactored ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
			Declarations ();
			ProgramStatementFactoredFactored ();
			break;
		case (TokenType::PROCEDURE):
			SubprogramDeclarations ();
			CompoundStatement ();
			Match (TokenType::DOT);
			break;
		case (TokenType::BEGIN):
			CompoundStatement ();
			Match (TokenType::DOT);
			break;
	}
}
void PascalParser::CompoundStatementFactored ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::END):
			Match (TokenType::END);
			break;
		case (TokenType::BEGIN):
		case (TokenType::ID):
		case (TokenType::CALL):
		case (TokenType::IF):
		case (TokenType::WHILE):
			OptionalStatements ();
			Match (TokenType::END);
			break;
	}
}
void PascalParser::SubprogramDeclarationFactored ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
			Declarations ();
			ProgramStatementFactoredFactored ();
			break;
		case (TokenType::PROCEDURE):
			SubprogramDeclarations ();
			CompoundStatement ();
			Match (TokenType::DOT);
			break;
		case (TokenType::BEGIN):
			CompoundStatement ();
			Match (TokenType::DOT);
			break;
	}
}
void PascalParser::SubprogramHeadFactored ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::PAREN_OPEN):
			Arguments ();
			break;
		case (TokenType::SEMICOLON):
			Match (TokenType::SEMICOLON);
			break;
	}
}
void PascalParser::StatementFactoredOne ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::END):
			Match (TokenType::END);
			break;
		case (TokenType::BEGIN):
		case (TokenType::ID):
		case (TokenType::CALL):
		case (TokenType::IF):
		case (TokenType::WHILE):
			OptionalStatements ();
			Match (TokenType::END);
	}
}
void PascalParser::StatementFactoredTwo ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::ELSE):
			Match (TokenType::ELSE);
			Statement ();
			break;
	}
	// e-prod
}
void PascalParser::VariableFactored ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::BRACKET_OPEN):
			Match (TokenType::BRACKET_OPEN);
			Expression ();
			Match (TokenType::BRACKET_CLOSE);
			break;
	}
	// e-prod
}
void PascalParser::ExpressionFactored ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::RELOP):
			Match (TokenType::RELOP);
			SimpleExpression ();
			break;
	}

	// e-prod
}
void PascalParser::ProcedureStatmentFactored ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::PAREN_OPEN):
			Match (TokenType::PAREN_OPEN);
			ExpressionList ();
			Match (TokenType::PAREN_CLOSE);
			break;
	}
	// e-prod
}
void PascalParser::ProgramStatementFactoredFactored ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROCEDURE):
			SubprogramDeclarations ();
			CompoundStatement ();
			Match (TokenType::DOT);
			break;
		case (TokenType::BEGIN):
			CompoundStatement ();
			Match (TokenType::DOT);
			break;
	}
}
void PascalParser::SubprogramStatementFactoredFactored ()
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROCEDURE):
			SubprogramDeclarations ();
			CompoundStatement ();
			break;
		case (TokenType::BEGIN):
			CompoundStatement ();
			break;
	}
}
