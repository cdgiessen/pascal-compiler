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

enum class TokenType
{

	program,
	id,
	standard_type,
	integer,
	real,
	function,
	procedure,
	assignop,
	variable,
	array,
	relop,
	simple_expression,
	addop,
	term,
	mulop,
	factor,
	sign,
	begin,
	end,
	t_not,
	of,
	t_if,
	t_then,
	t_else,
	t_while,
	t_do,
	paren_open,
	paren_close,
	semicolon,
	dot,
	comma,
	colon,
	bracket_open,
	bracket_close,
	dot_dot,
	eof,
};

template <> char const *enumStrings<TokenType>::data[] = {
	"(PROG)",   "(ID)",     "(STD_TYPE)", "(INT)",      "(REAL)",  "(FUNC)", "(PROC)",  "(ASSIGN)",
	"(VAR)",    "(ARRAY)",  "(RELOP)",    "(SIMP_EXP)", "(ADDOP)", "(TERM)", "(MULOP)", "(FACT)",
	"(SIGN)",   "(BEGIN)",  "(END)",      "(NOT)",      "(OF)",    "(IF)",   "(THEN)",  "(ELSE)",
	"(WHILE)",  "(DO)",     "(PAREN_O)",  "(PAREN_C)",  "(SEMIC)", "(DOT)",  "(COMMA)", "(COLON)",
	"(BRKT_O)", "(BRKT_C)", "(DOT_DOT)",  "(EOF)",
};

struct NoAttrib
{
	friend std::ostream &operator<< (std::ostream &os, const NoAttrib &t) { return os << "(NULL)"; }
};

enum class StandardType
{
	integer,
	real
};

template <> char const *enumStrings<StandardType>::data[] = { "INT", "REAL" };

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

enum class AddOpType
{
	plus,  //+
	minus, //-
	t_or   // or
};
template <> char const *enumStrings<AddOpType>::data[] = { "PLUS", "MINUS", "OR" };

enum class MulOpType
{
	mul,  //*
	div,  // div or /
	mod,  // mod
	t_and // and
};

template <> char const *enumStrings<MulOpType>::data[] = { "MUL", "DIV", "MOD", "AND" };

enum class SignOpType
{
	plus, //+
	minus //-
};

template <> char const *enumStrings<SignOpType>::data[] = {	"PLUS",	"MINUS"};

enum class RelOpType
{
	equal,                //=
	not_equal,            //<>
	less_than,            //<
	less_than_or_equal,   //<=
	greater_than,         //>
	greater_than_or_equal //>=
};

template <> char const *enumStrings<RelOpType>::data[] = { "EQ", "NEQ", "LT", "LEQ", "GT", "GEQ" };

using TokenAttribute =
std::variant<NoAttrib, StandardType, AddOpType, MulOpType, SignOpType, RelOpType, IntType, FloatType, SymbolType>;

std::ostream &operator<< (std::ostream &os, const TokenAttribute &t)
{
	if (t.index () == 0) return os << std::get<0> (t);
	if (t.index () == 1) return os << enumToString(std::get<1> (t));
	if (t.index () == 2) return os << enumToString(std::get<2> (t));
	if (t.index () == 3) return os << enumToString(std::get<3> (t));
	if (t.index () == 4) return os << enumToString(std::get<4> (t));
	if (t.index () == 5) return os << enumToString(std::get<5> (t));
	if (t.index () == 6) return os << std::get<6> (t);
	if (t.index () == 7) return os << std::get<7> (t);
	if (t.index () == 8) return os << std::get<8> (t);
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

enum class LexerErrorType
{
	Id,
	Int,
	SReal,
	LReal,
};
template <> char const *enumStrings<LexerErrorType>::data[] = { "id", "int", "sreal", "lreal" };

enum class LexerErrorSubType
{
	TooLong,
	ZeroLength,
	LeadingZero,
	TrailingZero,
	InvalidNumericLiteral,
};
template <>
char const *enumStrings<LexerErrorSubType>::data[] = { "TooLong", "ZeroLength", "LeadingZero", "TrailingZero","InvalidNumericLiteral" };

using ProgramLine = std::vector<char>;

std::ostream &operator<< (std::ostream &os, const ProgramLine &t)
{
	for (auto &c : t)
		os << c;
	return os;
}

struct LexerError
{
	LexerErrorType type;
	LexerErrorSubType subType;
	ProgramLine errorData;
	int line_location = -1;
	int column_location = -1;

	LexerError (LexerErrorType type, LexerErrorSubType subType, ProgramLine errorData)
	: type (type), subType (subType), errorData (errorData)
	{
	}

	LexerError (LexerErrorType type, LexerErrorSubType subType, int line, int column)
	: type (type), subType (subType), line_location (line), column_location (column)
	{
	}

	friend std::ostream &operator<< (std::ostream &os, const LexerError &t)
	{
		return os << enumToString (t.type) << '\t' << enumToString(t.subType) << '\t' << t.errorData;
	}
};

struct LexerMachineReturn
{
	int chars_to_eat = 0;
	using ReturnVariant = std::variant<std::monostate, TokenInfo, LexerError>;

	ReturnVariant content;

	LexerMachineReturn (int chars_to_eat) : chars_to_eat (chars_to_eat){};
	LexerMachineReturn (int chars_to_eat, TokenInfo token)
	: chars_to_eat (chars_to_eat), content (token)
	{
	}
	LexerMachineReturn (int chars_to_eat, LexerError error)
	: chars_to_eat (chars_to_eat), content (error)
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

	void TokenFilePrinter (int line_num, std::string lexeme, LexerMachineReturn::ReturnVariant content);

	private:
	std::vector<LexerMachine> machines;
	bool isInComment = false;
	ReservedWordList reservedWords;
	SymbolTable symbolTable;
	OutputFileHandle listing_file;
	OutputFileHandle token_file;
};