#pragma once

#include <functional>

#include "lexer.h"

#include "grammar_massager.h"

//void match(Token t) {
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
//void E() {
//	//swtich (token)
//}
//
//void Parse() {
//	//get the first token from the sequence
//	//call start production
//	//match with end token
//
//}

struct Func{



	std::vector<std::function<void()>> funcs;

};


struct NonTerminal {

};

TokenType FromTerminalIndex();

class ParserGenerator {
public:
	ParserGenerator(ParseTable table );
private:

	TokenType FromTerminalIndex();


}; 

class Parser {
public:
	Parser(TokenStream ts, ParserGenerator pg);
private:


};