#pragma once

#include <functional>
#include <stack>

#include "lexer.h"

class ParserContext
{
	public:
	ParserContext (TokenStream ts, std::string errorFileHandle);

	TokenInfo Current () const;
	TokenInfo Advance ();

	void LogError (std::string str);

	private:
	TokenStream ts;
	OutputFileHandle ofp;
};

class PascalParser
{
	public:
	PascalParser (ParserContext &pc);

	void Parse ();

	private:
	ParserContext &pc;

	void Match (TokenType tt);
	void HaltParse ();
	void Synch (std::vector<TokenType> set);

	void ProgramStatement ();
	void IdentifierList ();
	void Declarations ();
	void SubprogramDeclarations ();
	void CompoundStatement ();
	void Type ();
	void StandardType ();
	void SubprogramDeclaration ();
	void SubprogramHead ();
	void Arguments ();
	void ParameterList ();
	void OptionalStatements ();
	void StatementList ();
	void Statement ();
	void Variable ();
	void Expression ();
	void ProcedureStatement ();
	void ExpressionList ();
	void SimpleExpression ();
	void Term ();
	void Sign ();
	void Factor ();
	void IdentifierListPrime ();
	void DeclarationsPrime ();
	void SubprogramDeclarationsPrime ();
	void ParameterListPrime ();
	void StatementListPrime ();
	void ExpressionListPrime ();
	void SimpleExpressionPrime ();
	void TermPrime ();
	void ProgramStatementFactored ();
	void CompoundStatementFactored ();
	void SubprogramDeclarationFactored ();
	void SubprogramHeadFactored ();
	void StatementFactoredOne ();
	void StatementFactoredTwo ();
	void VariableFactored ();
	void ExpressionFactored ();
	void ProcedureStatmentFactored ();
	void ProgramStatementFactoredFactored ();
	void SubprogramStatementFactoredFactored ();
};

class Parser
{
	public:
	Parser (TokenStream ts);

	private:
};
