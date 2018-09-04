#pragma once

#include <algorithm>
#include <any>
#include <array>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
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
constexpr int string_literal_length = 10;
constexpr int integer_digit_length = 10;

// xx.yyEzz
constexpr int real_base_length = 5;
constexpr int real_decimal_length = 5;
constexpr int real_exponent_length = 2;

using ProgramLine = std::string_view;

// std::string Str_ProgramLine (const ProgramLine &line);
// ProgramLine Sub_ProgramLine (const ProgramLine &line, int indexesToCopy);
// ProgramLine Sub_ProgramLine (const ProgramLine &line, int firstIndex, int indexesToCopy);


// std::ostream &operator<< (std::ostream &os, const ProgramLine &t);


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
	ADDOP,
	MULOP,
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
	STR_LIT,
	END_FILE,
	LEXERR,
};

struct NoAttrib
{
	friend std::ostream &operator<< (std::ostream &os, const NoAttrib &t) { return os << "(NULL)"; }
};

enum class StandardEnum
{
	integer,
	real
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

enum class MulOpEnum
{
	mul,  //*
	div,  // div or /
	mod,  // mod
	t_and // and
};

enum class SignOpEnum
{
	plus, //+
	minus //-
};

enum class RelOpEnum
{
	equal,                //=
	not_equal,            //<>
	less_than,            //<
	less_than_or_equal,   //<=
	greater_than,         //>
	greater_than_or_equal //>=
};

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

struct StringLiteral
{
	int loc = -1;
	StringLiteral (int loc) : loc (loc) {}

	friend std::ostream &operator<< (std::ostream &os, const StringLiteral &t)
	{
		return os << t.loc << "(index in literal table)";
	}
};

enum class LexerErrorEnum
{
	Unrecognized_Symbol,
	Id_TooLong,
	StrLit_TooLong,
	StrLit_NotTerminated,
	Int_InvalidNumericLiteral,
	Int_TooLong,
	Int_LeadingZero,
	SReal_InvalidNumericLiteral,
	SReal1_LeadingZero,
	SReal1_TooLong,
	SReal2_TooLong,
	LReal_InvalidNumericLiteral,
	LReal1_LeadingZero,
	LReal2_TrailingZero,
	LReal3_LeadingZero,
	LReal1_TooLong,
	LReal2_TooLong,
	LReal3_TooLong,
	LReal3_TooShort,
	CommentContains2ndLeftCurlyBrace
};

struct LexerError
{
	LexerErrorEnum type;
	std::string errorData;
		LexerError (LexerErrorEnum type, std::string errorData) : type (type), errorData (errorData) {}
	LexerError (LexerErrorEnum type, std::string_view errorData) : type (type), errorData (std::string(errorData)) {}

	friend std::ostream &operator<< (std::ostream &os, const LexerError &t)
	{
		return os << enumToString (t.type) << " " << t.errorData;
	}
};

using TokenAttribute =
std::variant<NoAttrib, StandardEnum, AddOpEnum, MulOpEnum, SignOpEnum, RelOpEnum, IntType, FloatType, SymbolType, StringLiteral, LexerError>;

std::ostream &operator<< (std::ostream &os, const TokenAttribute &t);

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
		return os << enumToString (t.type) << '\t' << t.attrib;
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
	bool operator== (const std::string_view &s) const { return (word == s); }

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
	int AddSymbol (std::string_view symbol)
	{
		int i = 0;
		for (i = 0; i < symbols.size (); i++)
		{
			if (symbol == symbols[i]) return i;
		}
		symbols.push_back (std::string(symbol));
		return i;
	}

	int GetSymbolLocation (std::string_view symbol)
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

struct LexerContext
{
	bool isInComment = false;
	bool isInLiteral = false;
	ReservedWordList reservedWords;

	SymbolTable symbolTable;
	SymbolTable literalTable;

	LexerContext (ReservedWordList reservedWords) : reservedWords (reservedWords) {}
};

using LexerMachineFuncSig =
std::function<std::optional<LexerMachineReturn> (LexerContext &context, ProgramLine &line)>;

struct LexerMachine
{
	std::string name;
	int precedence = 10;
	LexerMachineFuncSig machine;

	LexerMachine (std::string name, int precedence, LexerMachineFuncSig machine)
	: name (name), precedence (precedence), machine (machine){};
};

struct TokenStream
{
	TokenStream (std::vector<TokenInfo> tokens, SymbolTable symbolTable)
	: tokens (tokens), symbolTable (symbolTable)
	{
	}

	std::vector<TokenInfo> tokens;
	SymbolTable symbolTable;
};

class Lexer
{
	public:
	Lexer () : listing_file ("listing_file.txt"), token_file ("token_file.txt")
	{
		fmt::print (token_file.FP (), "{:^14}{:<14}{:<16}{:<14}\n", "Line No.", "Lexeme", "TOKEN-TYPE", "ATTRIBUTE");
		CreateMachines ();
	}

	// void LoadReservedWords (ReservedWordList list) { reservedWords = list; };
	void CreateMachines ();

	void AddMachine (LexerMachine &&machine);

	TokenStream GetTokens (ReservedWordList &list, std::vector<std::string> lines);

	void TokenFilePrinter (int line_num, std::string_view lexeme, LexerMachineReturn::OptionalToken content);

	private:
	std::vector<LexerMachine> machines;

	OutputFileHandle listing_file;
	OutputFileHandle token_file;
};
