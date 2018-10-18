#include "parser.h"


ParserContext::ParserContext (TokenStream &ts, Logger &logger) : ts (ts), logger (logger) {}

TokenInfo ParserContext::Current () const { return ts.Current (); }
TokenInfo ParserContext::Advance () { return ts.Advance (); }


void ParserContext::LogError (std::string str)
{
	fmt::print (logger.listing_file.FP (), "{}\n", str);
}

void ParserContext::Match (TokenType tt)
{
	using namespace std::string_literals;

	if (tt == Current ().type && tt != TokenType::END_FILE)

		Advance ();

	else if (tt == Current ().type && tt == TokenType::END_FILE)

		HaltParse ();

	else if (tt != Current ().type)
	{
		LogError ("SE: Expected "s + tt + ", Recieved "s + Current ().type + "\n\t Error at line "
		          + std::to_string (Current ().line_location) + ":"
		          + std::to_string (Current ().column_location));
		Advance ();
	}
}

void ParserContext::Match (std::vector<TokenType> match_set)
{
	for (auto &tt : match_set)
	{
		Match (tt);
	}
}

void ParserContext::HaltParse ()
{
	// stop i guess?
	LogError ("HALT");
}

void ParserContext::Synch (std::vector<TokenType> set)
{
	set.push_back(TokenType::END_FILE);
	TokenType tt = Current ().type;

	bool found = false;
	for (auto &s : set)
		if (s == tt) found = true;
	while (!found)
	{
		tt = Advance ().type;
		for (auto &s : set)
			if (s == tt) found = true;
	}
}

PascalParser::PascalParser (Logger &logger) : logger (logger) {}

void PascalParser::Parse (ParserContext &pc)
{
	ProgramStatement (pc);
	pc.Match (TokenType::END_FILE);
}

void PascalParser::ProgramStatement (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROGRAM):
			pc.Match ({ TokenType::PROGRAM, TokenType::ID, TokenType::PAREN_OPEN });
			IdentifierList (pc);
			pc.Match ({ TokenType::PAREN_CLOSE, TokenType::SEMICOLON });
			ProgramStatementFactored (pc);
			break;
		default:
			pc.Synch ({ TokenType::END_FILE });
	}
}

void PascalParser::IdentifierList (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
			pc.Match (TokenType::ID);
			IdentifierListPrime (pc);
			break;

		default:
			pc.Synch ({ TokenType::PAREN_CLOSE });
	}
}
void PascalParser::Declarations (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
			pc.Match ({ TokenType::VARIABLE, TokenType::ID, TokenType::COLON });
			Type (pc);
			pc.Match (TokenType::SEMICOLON);
			DeclarationsPrime (pc);
			break;

		default:
			pc.Synch ({ TokenType::PROCEDURE, TokenType::BEGIN });
	}
}
void PascalParser::SubprogramDeclarations (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROCEDURE):
			SubprogramDeclaration (pc);
			pc.Match (TokenType::SEMICOLON);
			SubprogramDeclarationsPrime (pc);
			break;

		default:
			pc.Synch ({ TokenType::BEGIN });
	}
}
void PascalParser::CompoundStatement (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::BEGIN):
			pc.Match (TokenType::BEGIN);
			CompoundStatementFactored (pc);
			break;

		default:
			pc.Synch ({ TokenType::SEMICOLON, TokenType::DOT });
	}
}
void PascalParser::Type (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ARRAY):
			pc.Match ({ TokenType::ARRAY,
			TokenType::BRACKET_OPEN,
			TokenType::NUM,
			TokenType::DOT_DOT,
			TokenType::NUM,
			TokenType::BRACKET_CLOSE,
			TokenType::OF });
			StandardType (pc);
			break;
		case (TokenType::STANDARD_TYPE):
			StandardType (pc);

			break;

		default:
			pc.Synch ({ TokenType::PAREN_CLOSE, TokenType::SEMICOLON });
	}
}
void PascalParser::StandardType (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::STANDARD_TYPE):
			pc.Match (TokenType::STANDARD_TYPE);
			break;

		default:
			pc.Synch ({ TokenType::PAREN_CLOSE, TokenType::SEMICOLON });
	}
}
void PascalParser::SubprogramDeclaration (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROCEDURE):
			SubprogramHead (pc);
			SubprogramDeclarationFactored (pc);
			break;

		default:
			pc.Synch ({ TokenType::SEMICOLON });
	}
}
void PascalParser::SubprogramHead (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROCEDURE):
			pc.Match ({ TokenType::PROCEDURE, TokenType::ID });
			SubprogramHeadFactored (pc);
			break;

		default:
			pc.Synch ({ TokenType::PROCEDURE, TokenType::VARIABLE, TokenType::BEGIN });
	}
}
void PascalParser::Arguments (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PAREN_OPEN):
			pc.Match (TokenType::PAREN_OPEN);
			ParameterList (pc);
			pc.Match (TokenType::PAREN_CLOSE);
			break;

		default:
			pc.Synch ({ TokenType::SEMICOLON });
	}
}
void PascalParser::ParameterList (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
			pc.Match (TokenType::ID);
			pc.Match (TokenType::COLON);
			Type (pc);
			ParameterListPrime (pc);
			break;

		default:
			pc.Synch ({ TokenType::PAREN_CLOSE });
	}
}
void PascalParser::OptionalStatements (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
		case (TokenType::WHILE):
		case (TokenType::BEGIN):
		case (TokenType::IF):
		case (TokenType::CALL):
			StatementList (pc);
			break;

		default:
			pc.Synch ({ TokenType::END });
	}
}
void PascalParser::StatementList (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
		case (TokenType::WHILE):
		case (TokenType::BEGIN):
		case (TokenType::IF):
		case (TokenType::CALL):
			Statement (pc);
			StatementListPrime (pc);
			break;

		default:
			pc.Synch ({ TokenType::END });
	}
}
void PascalParser::Statement (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
			pc.Match (TokenType::VARIABLE);
			pc.Match (TokenType::ASSIGNOP);
			Expression (pc);
			break;
		case (TokenType::WHILE):
			pc.Match (TokenType::WHILE);
			Expression (pc);
			pc.Match (TokenType::DO);
			Statement (pc);
			break;
		case (TokenType::BEGIN):
			pc.Match (TokenType::BEGIN);
			StatementFactoredBegin (pc);
			break;
		case (TokenType::IF):
			pc.Match (TokenType::IF);
			Expression (pc);
			pc.Match (TokenType::THEN);
			Statement (pc);
			StatementFactoredElse (pc);
			break;
		case (TokenType::CALL):
			ProcedureStatement (pc);
			break;

		default:
			pc.Synch ({ TokenType::SEMICOLON, TokenType::ELSE, TokenType::END });
	}
}
void PascalParser::Variable (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
			pc.Match (TokenType::ID);
			VariableFactored (pc);
			break;

		default:
			pc.Synch ({ TokenType::ASSIGNOP });
	}
}
void PascalParser::Expression (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
		case (TokenType::PAREN_OPEN):
		case (TokenType::NUM):
		case (TokenType::NOT):
		case (TokenType::SIGN):
			SimpleExpression (pc);
			ExpressionFactored (pc);
			break;

		default:
			pc.Synch ({ TokenType::PAREN_CLOSE,
			TokenType::SEMICOLON,
			TokenType::BRACKET_CLOSE,
			TokenType::COMMA,
			TokenType::THEN,
			TokenType::ELSE,
			TokenType::DO, TokenType::END});
	}
}
void PascalParser::ProcedureStatement (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::CALL):
			pc.Match (TokenType::CALL);
			pc.Match (TokenType::ID);
			ProcedureStatmentFactored (pc);
			break;

		default:
			pc.Synch ({ TokenType::SEMICOLON, TokenType::ELSE, TokenType::END });
	}
}
void PascalParser::ExpressionList (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
		case (TokenType::PAREN_OPEN):
		case (TokenType::NUM):
		case (TokenType::NOT):
		case (TokenType::SIGN):
			Expression (pc);
			ExpressionListPrime (pc);
			break;

		default:
			pc.Synch ({ TokenType::PAREN_CLOSE });
	}
}
void PascalParser::SimpleExpression (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
		case (TokenType::PAREN_OPEN):
		case (TokenType::NUM):
		case (TokenType::NOT):
			Term (pc);
			SimpleExpressionPrime (pc);
			break;
		case (TokenType::SIGN):
			pc.Match (TokenType::SIGN);
			Term (pc);
			SimpleExpressionPrime (pc);

			break;

		default:
			pc.Synch ({ TokenType::PAREN_CLOSE,
			TokenType::SEMICOLON,
			TokenType::BRACKET_CLOSE,
			TokenType::COMMA,
			TokenType::RELOP,
			TokenType::THEN,
			TokenType::ELSE,
			TokenType::DO,
			TokenType::END});
	}
}
void PascalParser::Term (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
		case (TokenType::NUM):
		case (TokenType::PAREN_OPEN):
		case (TokenType::NOT):
			Factor (pc);
			TermPrime (pc);
			break;
		default:
			pc.Synch({ TokenType::PAREN_CLOSE,
			TokenType::SEMICOLON,
			TokenType::BRACKET_CLOSE,
			TokenType::COMMA,
			TokenType::RELOP,
			TokenType::ADDOP,
			TokenType::THEN,
			TokenType::ELSE,
			TokenType::DO,
			TokenType::END });
	}
}
void PascalParser::Sign (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::SIGN):
			pc.Match (TokenType::SIGN);
			break;

		default:
			pc.Synch ({ TokenType::ID, TokenType::PAREN_OPEN, TokenType::NUM, TokenType::NOT });
	}
}
void PascalParser::Factor (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ID):
			pc.Match (TokenType::ID);
			break;
		case (TokenType::NUM):
			pc.Match (TokenType::NUM);
			break;
		case (TokenType::PAREN_OPEN):
			pc.Match (TokenType::PAREN_OPEN);
			Expression (pc);
			pc.Match (TokenType::PAREN_CLOSE);
			break;
		case (TokenType::NOT):
			pc.Match (TokenType::NOT);
			Factor (pc);
			break;
		default:
			pc.Synch({ TokenType::PAREN_CLOSE,
			TokenType::SEMICOLON,
			TokenType::BRACKET_CLOSE,
			TokenType::COMMA,
			TokenType::RELOP,
			TokenType::ADDOP,
			TokenType::MULOP,
			TokenType::THEN,
			TokenType::ELSE,
			TokenType::DO,
			TokenType::END });
	}
}
void PascalParser::IdentifierListPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::COMMA):
			pc.Match (TokenType::COMMA);
			pc.Match (TokenType::ID);
			IdentifierListPrime (pc);
			break;
		case (TokenType::PAREN_CLOSE):
			break;
		default:
			pc.Synch ({ TokenType::PAREN_CLOSE });
	}
	// e-prod
}
void PascalParser::DeclarationsPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
			pc.Match (TokenType::VARIABLE);
			pc.Match (TokenType::ID);
			pc.Match (TokenType::COLON);
			Type (pc);
			pc.Match (TokenType::SEMICOLON);
			DeclarationsPrime (pc);
			break;
		case (TokenType::PROCEDURE):
		case (TokenType::BEGIN):
			break;
		default:
			pc.Synch ({ TokenType::PROCEDURE, TokenType::BEGIN });
	}
	// e-prod
}
void PascalParser::SubprogramDeclarationsPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROCEDURE):
			SubprogramDeclaration (pc);
			pc.Match (TokenType::SEMICOLON);
			SubprogramDeclarationsPrime (pc);
			break;
		case (TokenType::BEGIN):
			break;
		default:
			pc.Synch ({ TokenType::BEGIN });
	}
	// e-prod
}
void PascalParser::ParameterListPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::SEMICOLON):
			pc.Match (TokenType::SEMICOLON);
			pc.Match (TokenType::ID);
			pc.Match (TokenType::COLON);
			Type (pc);
			ParameterListPrime (pc);
			break;
		case (TokenType::PAREN_CLOSE):
			break;
		default:
			pc.Synch ({ TokenType::PAREN_CLOSE });
	}
	// e-prod
}
void PascalParser::StatementListPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::SEMICOLON):
			pc.Match (TokenType::SEMICOLON);
			Statement (pc);
			StatementListPrime (pc);
			break;
		case (TokenType::END):
			break;
		default:
			pc.Synch ({ TokenType::END });
	}
	// e -prod
}
void PascalParser::ExpressionListPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::COMMA):
			pc.Match (TokenType::COMMA);
			Expression (pc);
			ExpressionListPrime (pc);
			break;

		case (TokenType::PAREN_CLOSE):
			break;
		default:
			pc.Synch ({ TokenType::PAREN_CLOSE });
	}
	// e -prod
}
void PascalParser::SimpleExpressionPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ADDOP):
			pc.Match (TokenType::ADDOP);
			Term (pc);
			SimpleExpressionPrime (pc);
			break;
		case (TokenType::PAREN_CLOSE):
		case (TokenType::SEMICOLON):
		case (TokenType::BRACKET_CLOSE):
		case (TokenType::COMMA):
		case (TokenType::RELOP):
		case (TokenType::THEN):
		case (TokenType::ELSE):
		case (TokenType::DO):
		case (TokenType::END):
			break;
		default:
			pc.Synch ({ TokenType::PAREN_CLOSE,
			TokenType::SEMICOLON,
			TokenType::BRACKET_CLOSE,
			TokenType::COMMA,
			TokenType::RELOP,
			TokenType::THEN,
			TokenType::ELSE,
			TokenType::DO,
			TokenType::END});
	}
	// e -prod
}
void PascalParser::TermPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::MULOP):
			pc.Match (TokenType::MULOP);
			Factor (pc);
			TermPrime (pc);
			break;
		case (TokenType::ADDOP):
		case (TokenType::RELOP):
		case (TokenType::PAREN_CLOSE):
		case (TokenType::SEMICOLON):
		case (TokenType::BRACKET_CLOSE):
		case (TokenType::COMMA):
		case (TokenType::THEN):
		case (TokenType::ELSE):
		case (TokenType::DO):
		case (TokenType::END):
			break;
		default:
			pc.Synch ({ TokenType::PAREN_CLOSE,
			TokenType::SEMICOLON,
			TokenType::BRACKET_CLOSE,
			TokenType::COMMA,
			TokenType::ADDOP,
			TokenType::RELOP,
			TokenType::THEN,
			TokenType::ELSE,
			TokenType::END,
			TokenType::DO });
	}
	// e -prod
}
void PascalParser::ProgramStatementFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
			Declarations (pc);
			ProgramStatementFactoredFactored (pc);
			break;
		case (TokenType::PROCEDURE):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			pc.Match (TokenType::DOT);
			break;
		case (TokenType::BEGIN):
			CompoundStatement (pc);
			pc.Match (TokenType::DOT);
			break;

		default:
			pc.Synch ({ TokenType::END_FILE });
	}
}
void PascalParser::CompoundStatementFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::END):
			pc.Match (TokenType::END);
			break;
		case (TokenType::BEGIN):
		case (TokenType::ID):
		case (TokenType::CALL):
		case (TokenType::IF):
		case (TokenType::WHILE):
			OptionalStatements (pc);
			pc.Match (TokenType::END);
			break;

		default:
			pc.Synch ({ TokenType::SEMICOLON, TokenType::DOT });
	}
}
void PascalParser::SubprogramDeclarationFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::VARIABLE):
			Declarations (pc);
			ProgramStatementFactoredFactored (pc);
			break;
		case (TokenType::PROCEDURE):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			break;
		case (TokenType::BEGIN):
			CompoundStatement (pc);
			break;

		default:
			pc.Synch ({ TokenType::SEMICOLON });
	}
}
void PascalParser::SubprogramHeadFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PAREN_OPEN):
			Arguments (pc);
			pc.Match (TokenType::SEMICOLON);
			break;
		case (TokenType::SEMICOLON):
			pc.Match (TokenType::SEMICOLON);
			break;

		default:
			pc.Synch ({ TokenType::VARIABLE, TokenType::BEGIN, TokenType::PROCEDURE });
	}
}
void PascalParser::StatementFactoredBegin (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::END):
			pc.Match (TokenType::END);
			break;
		case (TokenType::BEGIN):
		case (TokenType::ID):
		case (TokenType::CALL):
		case (TokenType::IF):
		case (TokenType::WHILE):
			OptionalStatements (pc);
			pc.Match (TokenType::END);
			break;
		default:
			pc.Synch ({ TokenType::SEMICOLON, TokenType::ELSE, TokenType::END });
	}
}
void PascalParser::StatementFactoredElse (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::ELSE):
			pc.Match (TokenType::ELSE);
			Statement (pc);
			break;
		case (TokenType::SEMICOLON):
		case (TokenType::END):
			break;
		default:
			pc.Synch ({ TokenType::SEMICOLON, TokenType::ELSE , TokenType::END });
	}
	// e-prod
}
void PascalParser::VariableFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::BRACKET_OPEN):
			pc.Match (TokenType::BRACKET_OPEN);
			Expression (pc);
			pc.Match (TokenType::BRACKET_CLOSE);
			break;
		case (TokenType::ASSIGNOP):
			break;
		default:
			pc.Synch ({ TokenType::ASSIGNOP });
	}
	// e-prod
}
void PascalParser::ExpressionFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::RELOP):
			pc.Match (TokenType::RELOP);
			SimpleExpression (pc);
			break;
		case (TokenType::PAREN_CLOSE):
		case (TokenType::SEMICOLON):
		case (TokenType::BRACKET_CLOSE):
		case (TokenType::COMMA):
		case (TokenType::THEN):
		case (TokenType::ELSE):
		case (TokenType::DO):
		case (TokenType::END):
			break;
		default:
			pc.Synch ({ TokenType::PAREN_CLOSE,
			TokenType::SEMICOLON,
			TokenType::BRACKET_CLOSE,
			TokenType::COMMA,
			TokenType::THEN,
			TokenType::ELSE,
			TokenType::DO, 
			TokenType::END });
	}

	// e-prod
}
void PascalParser::ProcedureStatmentFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PAREN_OPEN):
			pc.Match (TokenType::PAREN_OPEN);
			ExpressionList (pc);
			pc.Match (TokenType::PAREN_CLOSE);
			break;
		case (TokenType::SEMICOLON):
		case (TokenType::ELSE):
			break;
		default:
			pc.Synch ({ TokenType::ELSE, TokenType::SEMICOLON, TokenType::END });
	}
	// e-prod
}
void PascalParser::ProgramStatementFactoredFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROCEDURE):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			pc.Match (TokenType::DOT);
			break;
		case (TokenType::BEGIN):
			CompoundStatement (pc);
			pc.Match (TokenType::DOT);
			break;

		default:
			pc.Synch ({ TokenType::END_FILE });
	}
}
void PascalParser::SubprogramStatementFactoredFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TokenType::PROCEDURE):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			break;
		case (TokenType::BEGIN):
			CompoundStatement (pc);
			break;

		default:
			pc.Synch ({ TokenType::SEMICOLON });
	}
}
