#pragma once

#include <functional>
#include <vector>

#include "common.h"
#include "lexer.h"

class ParserContext
{
	public:
	ParserContext (TokenStream &ts, Logger &logger);

	TokenInfo Current () const;

	void Match (TokenType tt);
	void Match (std::vector<TokenType> match_set);
	void HaltParse ();
	void Synch (std::vector<TokenType> set);

	void LogError (int line_loc, std::string str);
	void LogErrorExpectedGot (std::vector<TokenType> types);

	private:
	TokenInfo Advance ();
	Logger &logger;
	TokenStream &ts;
};

class PascalParser
{
	public:
	PascalParser (Logger &logger);

	void Parse (ParserContext &pc);

	private:
	using TT = TokenType;

	Logger &logger;

	void ProgramStatement (ParserContext &pc);
	void IdentifierList (ParserContext &pc);
	void Declarations (ParserContext &pc);
	void SubprogramDeclarations (ParserContext &pc);
	void CompoundStatement (ParserContext &pc);
	void Type (ParserContext &pc);
	void StandardType (ParserContext &pc);
	void SubprogramDeclaration (ParserContext &pc);
	void SubprogramHead (ParserContext &pc);
	void Arguments (ParserContext &pc);
	void ParameterList (ParserContext &pc);
	void OptionalStatements (ParserContext &pc);
	void StatementList (ParserContext &pc);
	void Statement (ParserContext &pc);
	void Variable (ParserContext &pc);
	void Expression (ParserContext &pc);
	void ProcedureStatement (ParserContext &pc);
	void ExpressionList (ParserContext &pc);
	void SimpleExpression (ParserContext &pc);
	void Term (ParserContext &pc);
	void Sign (ParserContext &pc);
	void Factor (ParserContext &pc);
	void IdentifierListPrime (ParserContext &pc);
	void DeclarationsPrime (ParserContext &pc);
	void SubprogramDeclarationsPrime (ParserContext &pc);
	void ParameterListPrime (ParserContext &pc);
	void StatementListPrime (ParserContext &pc);
	void ExpressionListPrime (ParserContext &pc);
	void SimpleExpressionPrime (ParserContext &pc);
	void TermPrime (ParserContext &pc);
	void ProgramStatementFactored (ParserContext &pc);
	void CompoundStatementFactored (ParserContext &pc);
	void SubprogramDeclarationFactored (ParserContext &pc);
	void SubprogramHeadFactored (ParserContext &pc);
	void StatementFactoredBegin (ParserContext &pc);
	void StatementFactoredElse (ParserContext &pc);
	void VariableFactored (ParserContext &pc);
	void ExpressionFactored (ParserContext &pc);
	void ProcedureStatmentFactored (ParserContext &pc);
	void ProgramStatementFactoredFactored (ParserContext &pc);
	void SubprogramStatementFactoredFactored (ParserContext &pc);
};
