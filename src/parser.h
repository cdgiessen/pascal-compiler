#pragma once

#include <functional>

#include "lexer.h"

#include "grammar_massager.h"

// void match(Token t) {
//	//if t == current token & t != end token
//	//advance to next token (consume it)
//
//	//if t == current token & t == end token
//	// end of parse!
//
//	//otherwise if t != token,
//	//Error, got token expected t
//}
//
// void E() {
//	//swtich (token)
//}
//
// void Parse() {
//	//get the first token from the sequence
//	//call start production
//	//match with end token
//
//}

using signature_production = std::function < void(TokenStream &ts);
using signature_match = std::function < void(TokenStream &ts, TokenType);

struct ProductionSignature
{
	Token start;
	std::vector<std::variant<signature_production, signature_match>> funcs;
	void operator() (TokenStream &ts) {}
};

struct ProductionGroup
{
	Token variable;
	bool isEProd = false;
	std::vector<ProductionSignature> signatures;

	void operator() (TokenStream &ts) { TokenType t = ts.GetNextToken (); }
}


TokenType
FromTerminalIndex ();

class ParserGenerator
{
	public:
	ParserGenerator (ParseTable table)
	{
		int varibleCount = table.table.size ();
		int terminalCount = table.table.at (0).size ();

		auto match[](TokenStream & ts, Token t)
		{
			if (ts.GetNextToken () == t) {}
		}
	}

	private:
	TokenType FromTerminalIndex ();

	std::vector<Func> functions;
};

class Parser
{
	public:
	Parser (TokenStream ts, ParserGenerator pg);

	private:
};