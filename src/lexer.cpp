

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

std::optional<TokenType> ReadTokenTypeFromString (std::string s)
{
	if (s == "program") return TokenType::program;
	if (s == "variable") return TokenType::variable;
	if (s == "array") return TokenType::array;
	if (s == "of") return TokenType::of;
	if (s == "integer") return TokenType::standard_type;
	if (s == "real") return TokenType::standard_type;
	if (s == "function") return TokenType::function;
	if (s == "procedure") return TokenType::procedure;
	if (s == "begin") return TokenType::begin;
	if (s == "end") return TokenType::end;
	if (s == "t_if") return TokenType::t_if;
	if (s == "t_then") return TokenType::t_then;
	if (s == "t_else") return TokenType::t_else;
	if (s == "t_while") return TokenType::t_while;
	if (s == "t_do") return TokenType::t_do;
	if (s == "not") return TokenType::t_not;
	if (s == "addop") return TokenType::addop;
	if (s == "mulop") return TokenType::mulop;
	return {};
}

std::optional<TokenAttribute> ReadTokenAttributeFromString (std::string s)
{
	if (s == "0") return NoAttrib{};
	if (s == "div") return MulOpType::div;
	if (s == "mod") return MulOpType::mod;
	if (s == "and") return MulOpType::t_and;
	if (s == "or") return AddOpType::t_or;
	if (s == "integer") return StandardType::integer;
	if (s == "real") return StandardType::real;
	return {};
}

using ReservedWordList = std::unordered_set<ReservedWord>;

ReservedWordList ReadReservedWordsFile ()
{
	ReservedWordList res_words;

	std::ifstream reserved_word_file (std::string ("reserved_words.txt"));

	if (reserved_word_file)
	{

		while (reserved_word_file.good ())
		{
			std::string word, type_s, attrib_s;
			reserved_word_file >> word >> type_s >> attrib_s;
			auto type = ReadTokenTypeFromString (type_s);
			auto attrib = ReadTokenAttributeFromString (attrib_s);
			if (type.has_value () && attrib.has_value ())
				res_words.emplace (word, type.value (), attrib.value ());
			else
				fmt::print ("Reserved word not found! {}\n", word);
		}

		fmt::print ("Res Word size = {}\n", res_words.size ());
		fmt::print ("Reserved Words:\n");
		// for(auto& item : res_words){
		//    fmt::print("Word: {} \t {}\n", item.word,
		//    static_cast<int>(item.type));
		//}
	}
	else
	{
		fmt::print ("Reserved Word List not found!\n");
	}
	return res_words;
}

std::optional<TokenInfo> CheckReseredWords (ReservedWordList &list, std::string s)
{
	for (auto &item : list)
	{
		if (item == s) return item.GetToken ();
	}
	return {};
}

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
template <> char const *enumStrings<enum class LexerErrorType>::data[] = { "id", "int", "sreal", "lreal" };

enum class LexerErrorSubType
{
	TooLong,
	ZeroLength,
	LeadingZero,
	TrailingZero,
	InvalidNumericLiteral,
};
template <>
char const *enumStrings<enum class LexerErrorSubType>::data[] = { "TooLong", "ZeroLength", "LeadingZero", "TrailingZero","InvalidNumericLiteral" };

using ProgramLine = std::vector<char>;

std::ostream &operator<< (std::ostream &os, const ProgramLine &t)
{
	for (auto &c : t)
		os << c;
	return os;
}

std::string Str_ProgramLine (const ProgramLine &line)
{
	std::string s;
	for (auto &c : line)
		s.append (1, c);
	return s;
}

ProgramLine Sub_ProgramLine (const ProgramLine &line, int indexesToCopy)
{
	if (indexesToCopy >= line.size ())
		return ProgramLine (line);
	else
	{
		ProgramLine output (indexesToCopy);
		std::memcpy (output.data (), line.data (), indexesToCopy);
		return output;
	}
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

void Lexer::AddMachine (LexerMachine &&machine)
{
	machines.push_back (std::move (machine));
	std::sort (std::begin (machines), std::end (machines),
	           [](LexerMachine a, LexerMachine b) { return a.precedence > b.precedence; });
}

void Lexer::TokenFilePrinter (int line_num, std::string lexeme, LexerMachineReturn::ReturnVariant content)
{
	if (content.index () == 1)
	{
		fmt::print (token_file.FP (), "{:^14}{:<14}{:<14}{:<14}{:<14}\n", line_num, lexeme,
		            enumToString(std::get<1> (content).type), std::get<1> (content).attrib.index (),
		            std::get<1> (content).attrib);
	}
	else if (content.index () == 2)
	{
		fmt::print (token_file.FP (), "{:^14}{:<14}{:<14} ({} {})\n", line_num, lexeme, "99 (LEXERR)",
		            enumToString (std::get<2> (content).type),  enumToString (std::get<2> (content).subType));
	}
	else
	{
		fmt::print (token_file.FP (), "{:^14}{:<14}{:<14}{:<14} (Unrecog Symbol)\n", line_num,
		            lexeme, "99 (LEXERR)", "1");
	}
}

std::vector<TokenInfo> Lexer::GetTokens (std::vector<std::string> lines)
{
	std::vector<TokenInfo> tokens;
	int backward_index = 0;
	int cur_line_number = 0;

	for (auto &s_line : lines)
	{

		fmt::print (listing_file.FP (), "{}\t{}\n", cur_line_number, s_line);

		ProgramLine full_line = std::vector<char> (s_line.size ());
		std::memcpy (full_line.data (), s_line.c_str (), s_line.length ());

		ProgramLine buffer;
		while (backward_index < full_line.size ())
		{
			buffer.clear ();
			buffer.insert (std::begin (buffer), std::begin (full_line) + backward_index, std::end (full_line));

			auto iter = std::begin (machines);
			std::optional<LexerMachineReturn> machine_ret = {};
			while (!machine_ret.has_value () && iter != std::end (machines))
			{
				machine_ret = iter->machine (buffer);

				iter++;
			}
			if (machine_ret.has_value ())
			{
				if (machine_ret->chars_to_eat > 0 && machine_ret->chars_to_eat < line_buffer_length)
				{

					backward_index += machine_ret->chars_to_eat;
					if (machine_ret->content.index () != 0)
					{
						TokenFilePrinter (cur_line_number,
						                  Str_ProgramLine (Sub_ProgramLine (buffer, machine_ret->chars_to_eat)),
						                  machine_ret->content);
					}
					if (machine_ret->content.index () == 1)
					{
						std::get<1> ((machine_ret->content)).line_location = cur_line_number;
						std::get<1> ((machine_ret->content)).column_location = backward_index;

						tokens.push_back (std::get<1> ((machine_ret->content)));
					}
					else if (machine_ret->content.index () == 2)
					{
						std::get<2> ((machine_ret->content)).line_location = cur_line_number;
						std::get<2> ((machine_ret->content)).column_location = backward_index;

						fmt::print (listing_file.FP (), "LEXERR:\t{}\n", std::get<2> ((machine_ret->content)));
					}
				}
			}
			else
			{
				fmt::print (listing_file.FP (), "LEXERR:\t{}\t{}\n", "Unrecognized Symbol: ", buffer[0]);
				std::string s;
				s.append (1, buffer[0]);
				TokenFilePrinter (cur_line_number, s, machine_ret->content);
				backward_index++;
			}
		}
		cur_line_number++;
		backward_index = 0;
	}
	return tokens;
}

void Lexer::CreateMachines ()
{
	AddMachine ({ "Comment", 110, [&](ProgramLine &line) -> std::optional<LexerMachineReturn> {
		             if (line.size () > 0 && line[0] == '{') { isInComment = true; }
		             if (isInComment)
		             {
			             int i = 0;
			             while (i < line.size () && line[i] != '}')
				             i++;
			             if (i < line.size () && line[i] == '}')
			             {
				             isInComment = false;
				             i++;
				             //if (i > line.size ()) i = line.size () - 1; // overflow?
			             }
			             return LexerMachineReturn (i);
		             }
		             return {};
	             } });

	AddMachine ({ "Whitespace", 100, [](ProgramLine &line) -> std::optional<LexerMachineReturn> {
		             int i = 0;
		             while (line[i] == ' ' || line[i] == '\t' || line[i] == '\n')
		             {
			             i++;
		             }

		             if (i > 0) return LexerMachineReturn (i);
		             return {};
	             } });

	AddMachine (
	{ "IdRes", 90, [&](ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.size () >= 1 && std::isalpha (line[0]))
		 {
			 int index = 0;

			 while (index < line.size () && std::isalnum (line[index]))
			 {
				 index++;
			 }
			 if (index <= identifier_length)
			 {
				 auto res_word =
				 CheckReseredWords (reservedWords, Str_ProgramLine (Sub_ProgramLine (line, index)));
				 if (res_word.has_value ())
				 { return LexerMachineReturn (index, res_word.value ()); }
				 int loc = symbolTable.AddSymbol (Str_ProgramLine (Sub_ProgramLine (line, index)));

				 return LexerMachineReturn (index, TokenInfo (TokenType::id, SymbolType (loc)));
			 }
			 return LexerMachineReturn (index, LexerError (LexerErrorType::Id, LexerErrorSubType::TooLong,
			                                               Sub_ProgramLine (line, index)));
		 }

		 return {};
	 } });

	AddMachine (
	{ "Catch-all", 80, [](ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.size () >= 2)
		 {
			 if (line[0] == ':' && line[1] == '=')
				 return LexerMachineReturn (2, TokenInfo (TokenType::assignop, NoAttrib ()));
			 if (line[0] == '.' && line[1] == '.')
				 return LexerMachineReturn (2, TokenInfo (TokenType::dot_dot, NoAttrib ()));
		 }
		 if (line.size () >= 1)
		 {
			 if (line[0] == '\0')
				 return LexerMachineReturn (1, TokenInfo (TokenType::eof, NoAttrib ()));
			 if (line[0] == '(')
				 return LexerMachineReturn (1, TokenInfo (TokenType::paren_open, NoAttrib ()));
			 if (line[0] == ')')
				 return LexerMachineReturn (1, TokenInfo (TokenType::paren_close, NoAttrib ()));
			 if (line[0] == ';')
				 return LexerMachineReturn (1, TokenInfo (TokenType::semicolon, NoAttrib ()));
			 if (line[0] == '.')
				 return LexerMachineReturn (1, TokenInfo (TokenType::dot, NoAttrib ()));
			 if (line[0] == ',')
				 return LexerMachineReturn (1, TokenInfo (TokenType::comma, NoAttrib ()));
			 if (line[0] == ':')
				 return LexerMachineReturn (1, TokenInfo (TokenType::colon, NoAttrib ()));
			 if (line[0] == '[')
				 return LexerMachineReturn (1, TokenInfo (TokenType::bracket_open, NoAttrib ()));
			 if (line[0] == ']')
				 return LexerMachineReturn (1, TokenInfo (TokenType::bracket_close, NoAttrib ()));
			 if (line[0] == '+')
				 return LexerMachineReturn (1, TokenInfo (TokenType::sign, SignOpType::plus));
			 if (line[0] == '-')
				 return LexerMachineReturn (1, TokenInfo (TokenType::sign, SignOpType::minus));
		 }
		 return {};
	 } });

	AddMachine (
	{ "Real", 60, [](ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 int base_size = 0;
		 int decimal_size = 0;
		 int pow_size = 0;
		 int i = 0;
		 while (i < line.size () && std::isdigit (line[i]))
		 {
			 i++;
			 base_size++;
		 }
		 if (i < line.size () && line[i] == '.')
		 {
			 i++;
			 while (i < line.size () && std::isdigit (line[i]))
			 {
				 i++;
				 decimal_size++;
			 }
			 if (i < line.size () && line[i] == 'E')
			 {
				 i++;
				 while (i < line.size () && std::isdigit (line[i]))
				 {
					 i++;
					 pow_size++;
				 }
				 if (base_size < real_base_length && decimal_size < real_decimal_length && pow_size < real_exponent_length)
				 { return LexerMachineReturn (i, TokenInfo (TokenType::real, NoAttrib ())); } else
				 {
					 return LexerMachineReturn (i, LexerError (LexerErrorType::LReal, LexerErrorSubType::TooLong,
					                                           Sub_ProgramLine (line, i)));
				 }
			 }
			 else
			 {
				 if (base_size < real_base_length && decimal_size < real_decimal_length)
				 { return LexerMachineReturn (i, TokenInfo (TokenType::real, NoAttrib ())); } else
				 {
					 return LexerMachineReturn (i, LexerError (LexerErrorType::SReal, LexerErrorSubType::TooLong,
					                                           Sub_ProgramLine (line, i)));
				 }
			 }
		 }
		 return {};
	 } });

	AddMachine (
	{ "Integer", 50, [](ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 int i = 0;
		 while (i < line.size () && std::isdigit (line[i]))
		 {
			 i++;
		 }
		 if (i > 0)
		 {
			 auto seq = Sub_ProgramLine (line, i);
			 int val = 0;
			 try
			 {
				 int val = std::stoi (Str_ProgramLine (seq));

				 if (i < integer_digit_length)
				 {
					 if (i > 1 && line[0] == '0')
					 {
						 return LexerMachineReturn (i, LexerError (LexerErrorType::Int,
						                                           LexerErrorSubType::LeadingZero, seq));
					 }
					 else
					 {
						 return LexerMachineReturn (i, TokenInfo (TokenType::integer, IntType (val)));
					 }
				 }
				 else
				 {
					 return LexerMachineReturn (i, LexerError (LexerErrorType::Int,
					                                           LexerErrorSubType::TooLong, seq));
				 }
			 }
			 catch (std::out_of_range e)
			 {
				 return LexerMachineReturn (i, LexerError (LexerErrorType::Int, LexerErrorSubType::TooLong, seq));
			 }
			 catch (std::invalid_argument e)
			 {
				 return LexerMachineReturn (i, LexerError (LexerErrorType::Int,
				                                           LexerErrorSubType::InvalidNumericLiteral, seq));
			 }
		 }
		 return {};
	 } });

	AddMachine (
	{ "Relop", 70, [](ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.size () >= 2)
		 {
			 if (line[0] == '>' && line[1] == '=')
				 return LexerMachineReturn (2, TokenInfo (TokenType::relop, RelOpType::greater_than_or_equal));
			 if (line[0] == '<' && line[1] == '=')
				 return LexerMachineReturn (2, TokenInfo (TokenType::relop, RelOpType::less_than_or_equal));
			 if (line[0] == '<' && line[1] == '>')
				 return LexerMachineReturn (2, TokenInfo (TokenType::relop, RelOpType::not_equal));
		 }
		 if (line[0] == '>')
			 return LexerMachineReturn (1, TokenInfo (TokenType::relop, RelOpType::greater_than));
		 if (line[0] == '<')
			 return LexerMachineReturn (1, TokenInfo (TokenType::relop, RelOpType::less_than));
		 if (line[0] == '=')
			 return LexerMachineReturn (1, TokenInfo (TokenType::relop, RelOpType::equal));
		 return {};
	 } });
}

int main (int argc, char *argv[])
{
	auto reserved_words = ReadReservedWordsFile ();

	std::vector<std::string> file_list;
	file_list.push_back ("test_input/test_passing.txt");
	// file_list.push_back("test_input/test_error.txt");

	// if (argc == 2) { inFileName = std::string (argv[1]); }

	Lexer lexer;
	lexer.LoadReservedWords (reserved_words);


	try
	{

		std::fstream inFile (file_list.at (0), std::ios::in);
		if (inFile)
		{
			std::vector<std::string> lines;
			int cur_line_number = 1;
			while (inFile.good ())
			{
				std::string line;

				std::getline (inFile, line);
				lines.push_back (line);
				if (line.size () > 0) {}

				cur_line_number++;
			}
			lexer.GetTokens (lines);
		}
		else
		{
			fmt::print ("File not read, was there an error?");
		}
	}
	catch (const std::exception &e)
	{
		fmt::print ("Exception {}", e.what ());
	}
	return 0;
}
