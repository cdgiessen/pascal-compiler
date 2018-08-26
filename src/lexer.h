#pragma once

#include <algorithm>
#include <any>
#include <array>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include <cctype>
#include <cstdio>
#include <cstring>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "enumstring.h"

constexpr int line_buffer_length = 72;
constexpr int identifier_length = 10;
constexpr int integer_digit_length = 10;

// xx.yyEzz
constexpr int real_base_length = 5;
constexpr int real_decimal_length = 5;
constexpr int real_exponent_length = 2;

class AST_root
{
};

using ProgramLine = std::vector<char>;

std::ostream &operator<< (std::ostream &os, const ProgramLine &t)
{
	for (auto &c : t)
		os << c;
	return os;
}

enum class TokenType
{

	PROGRAM,
	ID,
	STANDARD_TYPE,
	INTEGER,
	REAL,
	FUNCTION,
	PROCEDURE,
	ASSIGNOP,
	VARIABLE,
	ARRAY,
	RELOP,
	SIMPLE_EXPRESSION,
	ADDOP,
	TERM,
	MULOP,
	FACTOR,
	SIGN,
	BEGIN,
	END,
	NOT,
	OF,
	IF,
	THEN,
	ELSE,
	WHILE,
	DO,
	PAREN_OPEN,
	PAREN_CLOSE,
	SEMICOLON,
	DOT,
	COMMA,
	COLON,
	BRACKET_OPEN,
	BRACKET_CLOSE,
	DOT_DOT,
	END_FILE,
	LEXERR,
};

template <>
char const *enumStrings<TokenType>::data[] = { "PROG",  "ID",       "STD_TYPE", "INT",    "REAL",
	                                           "FUNC",  "PROC",     "ASSIGN",   "VAR",    "ARRAY",
	                                           "RELOP", "SIMP_EXP", "ADDOP",    "TERM",   "MULOP",
	                                           "FACT",  "SIGN",     "BEGIN",    "END",    "NOT",
	                                           "OF",    "IF",       "THEN",     "ELSE",   "WHILE",
	                                           "DO",    "PAREN_O",  "PAREN_C",  "SEMIC",  "DOT",
	                                           "COMMA", "COLON",    "BRKT_O",   "BRKT_C", "DOT_DOT",
	                                           "EOF",   "LEXERR" };

struct NoAttrib
{
	friend std::ostream &operator<< (std::ostream &os, const NoAttrib &t) { return os << "(NULL)"; }
};

enum class StandardEnum
{
	integer,
	real
};

template <> char const *enumStrings<StandardEnum>::data[] = { "INT", "REAL" };

struct IntType
{
	int val;
	IntType (int a) : val (a) {}

	friend std::ostream &operator<< (std::ostream &os, const IntType &t)
	{
		return os << t.val << "(INT)";
	}
};

struct FloatType
{
	float val;
	FloatType (float a) : val (a) {}

	friend std::ostream &operator<< (std::ostream &os, const FloatType &t)
	{
		return os << t.val << "(FLOAT)";
	}
};

struct SymbolType
{
	int loc = -1;
	SymbolType (int loc) : loc (loc) {}

	friend std::ostream &operator<< (std::ostream &os, const SymbolType &t)
	{
		return os << t.loc << " (index in symbol table)";
	}
};

enum class AddOpEnum
{
	plus,  //+
	minus, //-
	t_or   // or
};
template <> char const *enumStrings<AddOpEnum>::data[] = { "PLUS", "MINUS", "OR" };

enum class MulOpEnum
{
	mul,  //*
	div,  // div or /
	mod,  // mod
	t_and // and
};

template <> char const *enumStrings<MulOpEnum>::data[] = { "MUL", "DIV", "MOD", "AND" };

enum class SignOpEnum
{
	plus, //+
	minus //-
};

template <> char const *enumStrings<SignOpEnum>::data[] = { "PLUS", "MINUS" };

enum class RelOpEnum
{
	equal,                //=
	not_equal,            //<>
	less_than,            //<
	less_than_or_equal,   //<=
	greater_than,         //>
	greater_than_or_equal //>=
};

template <> char const *enumStrings<RelOpEnum>::data[] = { "EQ", "NEQ", "LT", "LEQ", "GT", "GEQ" };

enum class LexerErrorEnum
{
	Id_TooLong,
	Int_InvalidNumericLiteral,
	Int_TooLong,
	Int_LeadingZero,
	SReal_InvalidNumericLiteral,
	SReal1_TooLong,
	SReal2_TooLong,
	SReal1_LeadingZero,
	LReal_InvalidNumericLiteral,
	LReal1_LeadingZero,
	LReal1_TooLong,
	LReal2_TooLong,
	LReal3_TooLong,
};

template <>
char const *enumStrings<LexerErrorEnum>::data[] = {
	"Id_TooLong",
	"Int_InvalidNumericLiteral",
	"Int_TooLong",
	"Int_LeadingZero",
	"SReal_InvalidNumericLiteral",
	"SReal1_TooLong",
	"SReal2_TooLong",
	"SReal1_LeadingZero",
	"LReal_InvalidNumericLiteral",
	"LReal1_LeadingZero",
	"LReal1_TooLong",
	"LReal2_TooLong",
	"LReal3_TooLong",
};

std::string Str_ProgramLine (const ProgramLine &line);
ProgramLine Sub_ProgramLine (const ProgramLine &line, int indexesToCopy);

struct LexerError
{
	LexerErrorEnum type;
	ProgramLine errorData;
	LexerError (LexerErrorEnum type, ProgramLine errorData) : type (type), errorData (errorData) {}

	friend std::ostream &operator<< (std::ostream &os, const LexerError &t)
	{
		return os << enumToString(t.type) << " " << Str_ProgramLine(t.errorData);
	}
};

using TokenAttribute =
std::variant<NoAttrib, StandardEnum, AddOpEnum, MulOpEnum, SignOpEnum, RelOpEnum, IntType, FloatType, SymbolType, LexerError>;

std::ostream &operator<< (std::ostream &os, const TokenAttribute &t)
{
	if (t.index () == 0) return os << std::get<0> (t);
	if (t.index () == 1) return os << enumToString (std::get<1> (t));
	if (t.index () == 2) return os << enumToString (std::get<2> (t));
	if (t.index () == 3) return os << enumToString (std::get<3> (t));
	if (t.index () == 4) return os << enumToString (std::get<4> (t));
	if (t.index () == 5) return os << enumToString (std::get<5> (t));
	if (t.index () == 6) return os << std::get<6> (t);
	if (t.index () == 7) return os << std::get<7> (t);
	if (t.index () == 8) return os << std::get<8> (t);
	if (t.index () == 9) return os << std::get<9> (t);
}

struct TokenInfo
{
	TokenType type;
	TokenAttribute attrib;
	int line_location = -1;
	int column_location = -1;

	TokenInfo (TokenType type, TokenAttribute attrib) : type (type), attrib (attrib) {}

	TokenInfo (TokenType type, TokenAttribute attrib, int line, int column)
	: type (type), attrib (attrib), line_location (line), column_location (column)
	{
	}

	friend std::ostream &operator<< (std::ostream &os, const TokenInfo &t)
	{
		return os << '\t' << t.line_location << '\t' << enumToString (t.type) << '\t' << t.attrib;
	}
};

struct ReservedWord
{
	std::string word;
	TokenType type;
	TokenAttribute attrib;

	ReservedWord (std::string word, TokenType type, TokenAttribute attrib)
	: word (word), type (type), attrib (attrib){};

	bool operator== (const ReservedWord &other) const
	{
		return (word == other.word && type == other.type);
	}

	bool operator== (const std::string &s) const { return (word == s); }

	TokenInfo GetToken () const { return TokenInfo (type, attrib); }
};

namespace std
{
template <> struct hash<ReservedWord>
{
	size_t operator() (const ReservedWord &w) const { return hash<std::string> () (w.word); }
};
} // namespace std

using ReservedWordList = std::unordered_set<ReservedWord>;

class OutputFileHandle
{
	public:
	OutputFileHandle (std::string file_name)
	{
		fp = std::fopen (file_name.c_str (), "w");
		if (!fp) { fmt::print ("File opening failed"); }
	}

	~OutputFileHandle () { std::fclose (fp); }

	FILE *FP () const { return fp; };

	private:
	FILE *fp = nullptr;
};

class SymbolTable
{
	public:
	int AddSymbol (std::string &&symbol)
	{
		int i = 0;
		for (i = 0; i < symbols.size (); i++)
		{
			if (symbol == symbols[i]) return i;
		}
		symbols.push_back (symbol);
		return i;
	}

	int GetSymbolLocation (std::string &symbol)
	{
		for (int i = 0; i < symbols.size (); i++)
		{
			if (symbol == symbols[i]) return i;
		}
		return -1;
	}

	private:
	std::vector<std::string> symbols;
};

struct LexerMachineReturn
{
	using OptionalToken = std::optional<TokenInfo>;

	int chars_to_eat = 0;
	OptionalToken content;

	LexerMachineReturn (int chars_to_eat) : chars_to_eat (chars_to_eat){};

	LexerMachineReturn (int chars_to_eat, TokenInfo token)
	: chars_to_eat (chars_to_eat), content (token)
	{
	}
};

using LexerMachineFuncSig = std::function<std::optional<LexerMachineReturn> (ProgramLine &line)>;

struct LexerMachine
{
	std::string name;
	int precedence = 10;
	LexerMachineFuncSig machine;

	LexerMachine (std::string name, int precedence, LexerMachineFuncSig machine)
	: name (name), precedence (precedence), machine (machine){};
};


class Lexer
{
	public:
	Lexer () : listing_file ("listing_file.txt"), token_file ("token_file.txt")
	{
		fmt::print (token_file.FP (), "{:^14}{:^14}{:^14}{:^14}\n", "Line No.", "Lexeme", "TOKEN-TYPE", "ATTRIBUTE");
		CreateMachines ();
	}

	void LoadReservedWords (ReservedWordList list) { reservedWords = list; };
	void CreateMachines ();

	void AddMachine (LexerMachine &&machine);

	std::vector<TokenInfo> GetTokens (std::vector<std::string> lines);

	void TokenFilePrinter (int line_num, std::string lexeme, LexerMachineReturn::OptionalToken content);

	private:
	std::vector<LexerMachine> machines;
	bool isInComment = false;
	ReservedWordList reservedWords;
	SymbolTable symbolTable;
	OutputFileHandle listing_file;
	OutputFileHandle token_file;
};