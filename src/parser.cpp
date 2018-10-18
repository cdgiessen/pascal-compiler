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
	Match (TT::END_FILE);
}
void PascalParser::Match (TT tt)
{
	using namespace std::string_literals;

	if (tt == pc.Current ().type && tt != TT::END_FILE)

		pc.Advance ();

	else if (tt == pc.Current ().type && tt == TT::END_FILE)

		HaltParse ();

	else if (tt != pc.Current ().type)
	{
		pc.LogError ("SE: Expected "s + tt + ", Recieved "s + pc.Current ().type
		             + "\n\t Error at line " + std::to_string (pc.Current ().line_location) + ":"
		             + std::to_string (pc.Current ().column_location));
		pc.Advance ();
	}
}

void PascalParser::Match (std::vector<TT> match_set)
{
	using namespace std::string_literals;
	for (auto &tt : match_set)
	{
		if (tt == pc.Current ().type && tt != TT::END_FILE)

			pc.Advance ();

		else if (tt == pc.Current ().type && tt == TT::END_FILE)

			HaltParse ();

		else if (tt != pc.Current ().type)
		{
			pc.LogError ("SE: Expected "s + tt + ", Recieved "s + pc.Current ().type
			             + "\n\t Error at line " + std::to_string (pc.Current ().line_location)
			             + ":" + std::to_string (pc.Current ().column_location));
			pc.Advance ();
		}
	}
}

void PascalParser::HaltParse ()
{
	// stop i guess?
	pc.LogError ("HALT");
}

void PascalParser::Synch (std::vector<TT> set)
{
	if (set.size () == 0) return;
	TT tt = pc.Current ().type;
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
	switch (pc.Current ().type)
	{
		case (TT::PROGRAM):
			Match ({ TT::PROGRAM, TT::ID, TT::PAREN_OPEN });
			IdentifierList ();
			Match ({ TT::PAREN_CLOSE, TT::SEMICOLON });
			ProgramStatementFactored ();
			break;
		default:
			Synch ({ TT::END_FILE });
	}
}

void PascalParser::IdentifierList ()
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			Match (TT::ID);
			IdentifierListPrime ();
			break;

		default:
			Synch ({ TT::PAREN_CLOSE });
	}
}
void PascalParser::Declarations ()
{
	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
			Match (TT::VARIABLE);
			Match (TT::ID);
			Match (TT::COLON);
			Type ();
			Match (TT::SEMICOLON);
			DeclarationsPrime ();
			break;

		default:
			Synch ({ TT::PROCEDURE, TT::BEGIN });
	}
}
void PascalParser::SubprogramDeclarations ()
{
	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramDeclaration ();
			Match (TT::SEMICOLON);
			SubprogramDeclarationsPrime ();
			break;

		default:
			Synch ({ TT::BEGIN });
	}
}
void PascalParser::CompoundStatement ()
{
	switch (pc.Current ().type)
	{
		case (TT::BEGIN):
			Match (TT::BEGIN);
			CompoundStatementFactored ();
			break;

		default:
			Synch ({ TT::SEMICOLON, TT::DOT });
	}
}
void PascalParser::Type ()
{
	switch (pc.Current ().type)
	{
		case (TT::ARRAY):
			Match (TT::ARRAY);
			Match (TT::BRACKET_OPEN);
			Match (TT::NUM);
			Match (TT::DOT_DOT);
			Match (TT::NUM);
			Match (TT::BRACKET_CLOSE);
			Match (TT::OF);
			StandardType ();
			break;
		case (TT::STANDARD_TYPE):
			StandardType ();

			break;

		default:
			Synch ({ TT::PAREN_CLOSE, TT::SEMICOLON });
	}
}
void PascalParser::StandardType ()
{
	switch (pc.Current ().type)
	{
		case (TT::STANDARD_TYPE):
			Match (TT::STANDARD_TYPE);
			break;

		default:
			Synch ({ TT::PAREN_CLOSE, TT::SEMICOLON });
	}
}
void PascalParser::SubprogramDeclaration ()
{
	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramHead ();
			SubprogramDeclarationFactored ();
			break;

		default:
			Synch ({ TT::SEMICOLON });
	}
}
void PascalParser::SubprogramHead ()
{
	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			Match (TT::PROCEDURE);
			Match (TT::ID);
			SubprogramHeadFactored ();
			break;

		default:
			Synch ({ TT::PROCEDURE, TT::VARIABLE, TT::BEGIN });
	}
}
void PascalParser::Arguments ()
{
	switch (pc.Current ().type)
	{
		case (TT::PAREN_OPEN):
			Match (TT::PAREN_OPEN);
			ParameterList ();
			Match (TT::PAREN_CLOSE);
			break;

		default:
			Synch ({ TT::SEMICOLON });
	}
}
void PascalParser::ParameterList ()
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			Match (TT::ID);
			Match (TT::COLON);
			Type ();
			ParameterListPrime ();
			break;

		default:
			Synch ({ TT::PAREN_CLOSE });
	}
}
void PascalParser::OptionalStatements ()
{
	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
		case (TT::WHILE):
		case (TT::BEGIN):
		case (TT::IF):
		case (TT::CALL):
			StatementList ();
			break;

		default:
			Synch ({ TT::END });
	}
}
void PascalParser::StatementList ()
{
	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
		case (TT::WHILE):
		case (TT::BEGIN):
		case (TT::IF):
		case (TT::CALL):
			Statement ();
			StatementListPrime ();
			break;

		default:
			Synch ({ TT::END });
	}
}
void PascalParser::Statement ()
{
	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
			Match (TT::VARIABLE);
			Match (TT::ASSIGNOP);
			Expression ();
			break;
		case (TT::WHILE):
			Match (TT::WHILE);
			Expression ();
			Match (TT::DO);
			Statement ();
			break;
		case (TT::BEGIN):
			Match (TT::BEGIN);
			StatementFactoredBegin ();
			break;
		case (TT::IF):
			Match (TT::IF);
			Expression ();
			Match (TT::THEN);
			Statement ();
			StatementFactoredElse ();
			break;
		case (TT::CALL):
			ProcedureStatement ();
			break;

		default:
			Synch ({ TT::SEMICOLON, TT::ELSE });
	}
}
void PascalParser::Variable ()
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			Match (TT::ID);
			VariableFactored ();
			break;

		default:
			Synch ({ TT::ASSIGNOP });
	}
}
void PascalParser::Expression ()
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::PAREN_OPEN):
		case (TT::NUM):
		case (TT::NOT):
		case (TT::SIGN):
			SimpleExpression ();
			ExpressionFactored ();
			break;

		default:
			Synch ({ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::THEN, TT::ELSE, TT::DO });
	}
}
void PascalParser::ProcedureStatement ()
{
	switch (pc.Current ().type)
	{
		case (TT::CALL):
			Match (TT::CALL);
			Match (TT::ID);
			ProcedureStatmentFactored ();
			break;

		default:
			Synch ({ TT::SEMICOLON, TT::ELSE });
	}
}
void PascalParser::ExpressionList ()
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::PAREN_OPEN):
		case (TT::NUM):
		case (TT::NOT):
		case (TT::SIGN):
			Expression ();
			ExpressionListPrime ();
			break;

		default:
			Synch ({ TT::PAREN_CLOSE });
	}
}
void PascalParser::SimpleExpression ()
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::PAREN_OPEN):
		case (TT::NUM):
		case (TT::NOT):
			Term ();
			SimpleExpressionPrime ();
			break;
		case (TT::SIGN):
			Match (TT::SIGN);
			Term ();
			SimpleExpressionPrime ();

			break;

		default:
			Synch ({ TT::PAREN_CLOSE, TT::SEMICOLON, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO });
	}
}
void PascalParser::Term ()
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::NUM):
		case (TT::PAREN_OPEN):
		case (TT::NOT):
			Factor ();
			TermPrime ();
			break;

		default:
			Synch ({ TT::ADDOP });
	}
}
void PascalParser::Sign ()
{
	switch (pc.Current ().type)
	{
		case (TT::SIGN):
			Match (TT::SIGN);
			break;

		default:
			Synch ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT });
	}
}
void PascalParser::Factor ()
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			Match (TT::ID);
			break;
		case (TT::NUM):
			Match (TT::NUM);
			break;
		case (TT::PAREN_OPEN):
			Match (TT::PAREN_OPEN);
			Expression ();
			Match (TT::PAREN_CLOSE);
			break;
		case (TT::NOT):
			Match (TT::NOT);
			Factor ();
			break;

		default:
			Synch ({ TT::MULOP });
	}
}
void PascalParser::IdentifierListPrime ()
{
	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			Match (TT::COMMA);
			Match (TT::ID);
			IdentifierListPrime ();
			break;
		case (TT::PAREN_CLOSE):
			break;
		default:
			Synch ({ TT::PAREN_CLOSE });
	}
	// e-prod
}
void PascalParser::DeclarationsPrime ()
{
	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
			Match (TT::VARIABLE);
			Match (TT::ID);
			Match (TT::COLON);
			Type ();
			Match (TT::SEMICOLON);
			DeclarationsPrime ();
			break;
		case (TT::PROCEDURE):
		case (TT::BEGIN):
			break;
		default:
			Synch ({ TT::PROCEDURE, TT::BEGIN });
	}
	// e-prod
}
void PascalParser::SubprogramDeclarationsPrime ()
{
	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramDeclaration ();
			Match (TT::SEMICOLON);
			SubprogramDeclarationsPrime ();
			break;
		case (TT::BEGIN):
			break;
		default:
			Synch ({ TT::BEGIN });
	}
	// e-prod
}
void PascalParser::ParameterListPrime ()
{
	switch (pc.Current ().type)
	{
		case (TT::SEMICOLON):
			Match (TT::SEMICOLON);
			Match (TT::ID);
			Match (TT::COLON);
			Type ();
			ParameterListPrime ();
			break;
		case (TT::PAREN_CLOSE):
			break;
		default:
			Synch ({ TT::PAREN_CLOSE });
	}
	// e-prod
}
void PascalParser::StatementListPrime ()
{
	switch (pc.Current ().type)
	{
		case (TT::SEMICOLON):
			Match (TT::SEMICOLON);
			Statement ();
			StatementListPrime ();
			break;
		case (TT::END):
			break;
		default:
			Synch ({ TT::END });
	}
	// e -prod
}
void PascalParser::ExpressionListPrime ()
{
	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			Match (TT::COMMA);
			Expression ();
			ExpressionListPrime ();
			break;

		case (TT::PAREN_CLOSE):
			break;
		default:
			Synch ({ TT::PAREN_CLOSE });
	}
	// e -prod
}
void PascalParser::SimpleExpressionPrime ()
{
	switch (pc.Current ().type)
	{
		case (TT::ADDOP):
			Match (TT::ADDOP);
			Term ();
			SimpleExpressionPrime ();
			break;
		case (TT::PAREN_CLOSE):
		case (TT::SEMICOLON):
		case (TT::BRACKET_CLOSE):
		case (TT::COMMA):
		case (TT::RELOP):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
			break;
		default:
			Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO });
	}
	// e -prod
}
void PascalParser::TermPrime ()
{
	switch (pc.Current ().type)
	{
		case (TT::MULOP):
			Match (TT::MULOP);
			Factor ();
			TermPrime ();
			break;
		case (TT::ADDOP):
			break;
		default:
			Synch({ TT::ADDOP });
	}
	// e -prod
}
void PascalParser::ProgramStatementFactored ()
{
	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
			Declarations ();
			ProgramStatementFactoredFactored ();
			break;
		case (TT::PROCEDURE):
			SubprogramDeclarations ();
			CompoundStatement ();
			Match (TT::DOT);
			break;
		case (TT::BEGIN):
			CompoundStatement ();
			Match (TT::DOT);
			break;

		default:
			Synch ({ TT::END_FILE });
	}
}
void PascalParser::CompoundStatementFactored ()
{
	switch (pc.Current ().type)
	{
		case (TT::END):
			Match (TT::END);
			break;
		case (TT::BEGIN):
		case (TT::ID):
		case (TT::CALL):
		case (TT::IF):
		case (TT::WHILE):
			OptionalStatements ();
			Match (TT::END);
			break;

		default:
			Synch ({ TT::SEMICOLON, TT::DOT });
	}
}
void PascalParser::SubprogramDeclarationFactored ()
{
	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
			Declarations ();
			ProgramStatementFactoredFactored ();
			break;
		case (TT::PROCEDURE):
			SubprogramDeclarations ();
			CompoundStatement ();
			break;
		case (TT::BEGIN):
			CompoundStatement ();
			break;

		default:
			Synch ({ TT::SEMICOLON });
	}
}
void PascalParser::SubprogramHeadFactored ()
{
	switch (pc.Current ().type)
	{
		case (TT::PAREN_OPEN):
			Arguments ();
			Match (TT::SEMICOLON);
			break;
		case (TT::SEMICOLON):
			Match (TT::SEMICOLON);
			break;

		default:
			Synch ({ TT::VARIABLE, TT::BEGIN });
	}
}
void PascalParser::StatementFactoredBegin ()
{
	switch (pc.Current ().type)
	{
		case (TT::END):
			Match (TT::END);
			break;
		case (TT::BEGIN):
		case (TT::ID):
		case (TT::CALL):
		case (TT::IF):
		case (TT::WHILE):
			OptionalStatements ();
			Match (TT::END);
			break;
		default:
			Synch ({ TT::SEMICOLON, TT::ELSE });
	}
}
void PascalParser::StatementFactoredElse ()
{
	switch (pc.Current ().type)
	{
		case (TT::ELSE):
			Match (TT::ELSE);
			Statement ();
			break;
		case (TT::SEMICOLON):
			break;
		default:
			Synch({ TT::SEMICOLON});
	}
	// e-prod
}
void PascalParser::VariableFactored ()
{
	switch (pc.Current ().type)
	{
		case (TT::BRACKET_OPEN):
			Match (TT::BRACKET_OPEN);
			Expression ();
			Match (TT::BRACKET_CLOSE);
			break;
		case (TT::ASSIGNOP):
			break;
		default:
			Synch({ TT::ASSIGNOP});
	}
	// e-prod
}
void PascalParser::ExpressionFactored ()
{
	switch (pc.Current ().type)
	{
		case (TT::RELOP):
			Match (TT::RELOP);
			SimpleExpression ();
			break;
		case (TT::PAREN_CLOSE):
		case (TT::SEMICOLON):
		case (TT::BRACKET_CLOSE):
		case (TT::COMMA):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
			break;
		default:
		Synch({ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::THEN, TT::ELSE, TT::DO });

	}

	// e-prod
}
void PascalParser::ProcedureStatmentFactored ()
{
	switch (pc.Current ().type)
	{
		case (TT::PAREN_OPEN):
			Match (TT::PAREN_OPEN);
			ExpressionList ();
			Match (TT::PAREN_CLOSE);
			break;
		case (TT::SEMICOLON):
		case (TT::ELSE):
			break;
		default:
			Synch({ TT::ELSE, TT::SEMICOLON });
	}
	// e-prod
}
void PascalParser::ProgramStatementFactoredFactored ()
{
	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramDeclarations ();
			CompoundStatement ();
			Match (TT::DOT);
			break;
		case (TT::BEGIN):
			CompoundStatement ();
			Match (TT::DOT);
			break;

		default:
			Synch ({ TT::END_FILE });
	}
}
void PascalParser::SubprogramStatementFactoredFactored ()
{
	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramDeclarations ();
			CompoundStatement ();
			break;
		case (TT::BEGIN):
			CompoundStatement ();
			break;

		default:
			Synch ({ TT::SEMICOLON });
	}
}
