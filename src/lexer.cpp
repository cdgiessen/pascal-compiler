

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
	identifier_list,
	id,
	declarations,
	type,
	standard_type,
	integer,
	real,
	subprogram_declarations,
	subprogram_declaration,
	subprogram_head,
	function,
	procedure,
	arguments,
	parameter_list,
	compound_statement,
	optional_statements,
	statement_list,
	statement,
	assignop,
	variable,
	array,
	procedure_statement,
	expression_list,
	expression,
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

};

std::ostream &operator<< (std::ostream &os, const TokenType &t)
{
	if (t == TokenType::program) return os << static_cast<int> (t) << " (PROG)";
	if (t == TokenType::identifier_list) return os << static_cast<int> (t) << " (ID_LST)";
	if (t == TokenType::id) return os << static_cast<int> (t) << " (ID)";
	if (t == TokenType::declarations) return os << static_cast<int> (t) << " (DEC)";
	if (t == TokenType::type) return os << static_cast<int> (t) << " (TYPE)";
	if (t == TokenType::standard_type) return os << static_cast<int> (t) << " (STD_TYPE)";
	if (t == TokenType::integer) return os << static_cast<int> (t) << " (INT)";
	if (t == TokenType::real) return os << static_cast<int> (t) << " (REAL)";
	if (t == TokenType::subprogram_declarations) return os << static_cast<int> (t) << " (SUB_DECS)";
	if (t == TokenType::subprogram_declaration) return os << static_cast<int> (t) << " (SUB_DEC)";
	if (t == TokenType::subprogram_head) return os << static_cast<int> (t) << " (SUB_HEAD)";
	if (t == TokenType::function) return os << static_cast<int> (t) << " (FUNC)";
	if (t == TokenType::procedure) return os << static_cast<int> (t) << " (PROC)";
	if (t == TokenType::arguments) return os << static_cast<int> (t) << " (ARGS)";
	if (t == TokenType::parameter_list) return os << static_cast<int> (t) << " (PRM_LST)";
	if (t == TokenType::compound_statement) return os << static_cast<int> (t) << " (COM_STMT)";
	if (t == TokenType::optional_statements) return os << static_cast<int> (t) << " (OPT_STMT)";
	if (t == TokenType::statement_list) return os << static_cast<int> (t) << " (STMT_LST)";
	if (t == TokenType::statement) return os << static_cast<int> (t) << " (STMT)";
	if (t == TokenType::assignop) return os << static_cast<int> (t) << " (ASSIGN)";
	if (t == TokenType::variable) return os << static_cast<int> (t) << " (VAR)";
	if (t == TokenType::array) return os << static_cast<int> (t) << " (ARRAY)";
	if (t == TokenType::procedure_statement) return os << static_cast<int> (t) << " (PROC_STMT)";
	if (t == TokenType::expression_list) return os << static_cast<int> (t) << " (EXP_LIST)";
	if (t == TokenType::expression) return os << static_cast<int> (t) << " (EXP)";
	if (t == TokenType::relop) return os << static_cast<int> (t) << " (RELOP)";
	if (t == TokenType::simple_expression) return os << static_cast<int> (t) << " (SIMP_EXP)";
	if (t == TokenType::addop) return os << static_cast<int> (t) << " (ADDOP)";
	if (t == TokenType::term) return os << static_cast<int> (t) << " (TERM)";
	if (t == TokenType::mulop) return os << static_cast<int> (t) << " (MULOP)";
	if (t == TokenType::factor) return os << static_cast<int> (t) << " (FACT)";
	if (t == TokenType::sign) return os << static_cast<int> (t) << " (SIGN)";
	if (t == TokenType::begin) return os << static_cast<int> (t) << " (BEGIN)";
	if (t == TokenType::end) return os << static_cast<int> (t) << " (END)";
	if (t == TokenType::t_not) return os << static_cast<int> (t) << " (NOT)";
	if (t == TokenType::of) return os << static_cast<int> (t) << " (OF)";
	if (t == TokenType::t_if) return os << static_cast<int> (t) << " (IF)";
	if (t == TokenType::t_then) return os << static_cast<int> (t) << " (THEN)";
	if (t == TokenType::t_else) return os << static_cast<int> (t) << " (ELSE)";
	if (t == TokenType::t_while) return os << static_cast<int> (t) << " (WHILE)";
	if (t == TokenType::t_do) return os << static_cast<int> (t) << " (DO)";
	if (t == TokenType::paren_open) return os << static_cast<int> (t) << " (PAREN_O)";
	if (t == TokenType::paren_close) return os << static_cast<int> (t) << " (PAREN_C)";
	if (t == TokenType::semicolon) return os << static_cast<int> (t) << " (SEMIC)";
	if (t == TokenType::dot) return os << static_cast<int> (t) << " (DOT)";
	if (t == TokenType::comma) return os << static_cast<int> (t) << " (COMMA)";
	if (t == TokenType::colon) return os << static_cast<int> (t) << " (COLON)";
	if (t == TokenType::bracket_open) return os << static_cast<int> (t) << " (BRKT_O)";
	if (t == TokenType::bracket_close) return os << static_cast<int> (t) << " (BRKT_C)";
	if (t == TokenType::dot_dot) return os << static_cast<int> (t) << " (DOT_DOT)";
}

struct NoAttrib
{
	friend std::ostream &operator<< (std::ostream &os, const NoAttrib &t) { return os << "(NULL)"; }
};

enum class StandardType
{
	integer,
	real
};

std::ostream &operator<< (std::ostream &os, const StandardType &t)
{
	if (t == StandardType::integer) return os << static_cast<int> (t) << '\t' << "(ST_INT)";
	return os << static_cast<int> (t) << '\t' << "(ST_REAL)";
}

struct NumberType
{
};

struct SymbolType
{
};

enum class AddOpType
{
	plus,  //+
	minus, //-
	t_or   // or
};

std::ostream &operator<< (std::ostream &os, const AddOpType &t)
{
	if (t == AddOpType::plus) return os << static_cast<int> (t) << '\t' << "(PLUS)";
	if (t == AddOpType::minus) return os << static_cast<int> (t) << '\t' << "(MINUS)";
	return os << static_cast<int> (t) << '\t' << "or";
}


enum class MulOpType
{
	mul,  //*
	div,  // div or /
	mod,  // mod
	t_and // and
};

std::ostream &operator<< (std::ostream &os, const MulOpType &t)
{
	if (t == MulOpType::mul) return os << static_cast<int> (t) << '\t' << "(MUL)";
	if (t == MulOpType::div) return os << static_cast<int> (t) << '\t' << "(DIV)";
	if (t == MulOpType::mod) return os << static_cast<int> (t) << '\t' << "(MOD)";
	return os << static_cast<int> (t) << '\t' << "(AND)";
}

enum class SignOpType
{
	plus, //+
	minus //-
};
std::ostream &operator<< (std::ostream &os, const enum class SignOpType &t)
{
	if (t == SignOpType ::plus) return os << static_cast<int> (t) << '\t' << "(PLUS)";
	return os << static_cast<int> (t) << '\t' << "(MINUS)";
}

enum class RelOpType
{
	equal,                //=
	not_equal,            //<>
	less_than,            //<
	less_than_or_equal,   //<=
	greater_than,         //>
	greater_than_or_equal //>=
};

std::ostream &operator<< (std::ostream &os, const RelOpType &t)
{
	if (t == RelOpType::equal) return os << static_cast<int> (t) << '\t' << "(EQ)";
	if (t == RelOpType::not_equal) return os << static_cast<int> (t) << '\t' << "(NEQ)";
	if (t == RelOpType::less_than) return os << static_cast<int> (t) << '\t' << "(LT)";
	if (t == RelOpType::less_than_or_equal) return os << static_cast<int> (t) << '\t' << "(LEQ)";
	if (t == RelOpType::greater_than) return os << static_cast<int> (t) << '\t' << "(GT)";
	return os << static_cast<int> (t) << '\t' << "(GEQ)";
}

using TokenAttribute = std::variant<NoAttrib, StandardType, AddOpType, MulOpType, SignOpType, RelOpType>;

std::ostream &operator<< (std::ostream &os, const TokenAttribute &t)
{
	if (t.index () == 0) return os << std::get<0> (t);
	if (t.index () == 1) return os << std::get<1> (t);
	if (t.index () == 2) return os << std::get<2> (t);
	if (t.index () == 3) return os << std::get<3> (t);
	if (t.index () == 4) return os << std::get<4> (t);
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
		return os << '\t' << t.line_location << '\t' << t.type << '\t' << t.attrib;

		//  << static_cast<int>(t.attrib)
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
		if (!fp)
		{
			fmt::print ("File opening failed");
		}
	}

	~OutputFileHandle () { std::fclose (fp); }

	FILE *FP () const { return fp; };

	private:
	FILE *fp = nullptr;
};

class SymbolTable
{
	std::vector<std::string> symbols;
};

enum class LexerErrorType
{
	Id,
	Int,
	SReal,
	LReal,
};

std::ostream &operator<< (std::ostream &os, LexerErrorType t)
{
	if (t == LexerErrorType::Id) return os << "id";
	if (t == LexerErrorType::Int) return os << "int";
	if (t == LexerErrorType::SReal) return os << "sReal";
	return os << "lReal"; // guaranteed return value, no bad paths
}

enum class LexerErrorSubType
{
	TooLong,
	ZeroLength,
	LeadingZero,
	TrailingZero,
};

std::ostream &operator<< (std::ostream &os, LexerErrorSubType t)
{
	if (t == LexerErrorSubType::TooLong) return os << "Too Long";
	if (t == LexerErrorSubType::ZeroLength) return os << "Zero Length";
	if (t == LexerErrorSubType::TrailingZero) return os << "Trailing Zero";
	return os << "Leading Zero"; // guaranteed return value, no bad paths
}

using ProgramLine = std::vector<char>;

std::ostream &operator<< (std::ostream &os, const ProgramLine &t)
{
	for (auto &c : t)
		os << c;
	return os;
}

std::string Str_ProgramLine (ProgramLine &line)
{
	std::string s;
	for (auto &c : line)
		s.append (1, c);
	return s;
}

ProgramLine Sub_ProgramLine (ProgramLine &line, int indexesToCopy)
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
		return os << t.type << '\t' << t.subType << '\t' << t.errorData;
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
		fmt::print (token_file.FP (), "Line No.\t Lexeme\tTOKEN-TYPE\tATTRIBUTE\n");
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
		fmt::print (token_file.FP (), "{}\t{}\t{}\t{}\t{}\n", line_num, lexeme,
		            std::get<1> (content).type, std::get<1> (content).attrib.index (),
		            std::get<1> (content).attrib);
	}
	else if (content.index () == 2)
	{
		fmt::print (token_file.FP (), "{}\t{}\t99 (LEXERR)\t{} ({} {})\n", line_num, lexeme,
		            std::get<2> (content).type, std::get<2> (content).subType);
	}
	else
	{
		fmt::print (token_file.FP (), "{}\t{}\t99 (LEXERR)\t{} (Unrecog Symbol)\n", line_num, lexeme, "1");
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
			if (machine_ret.has_value () && iter != std::end (machines))
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
		             if (line.size () > 0 && line[0] == '{')
		             {
						 isInComment = true;             
		             } 
					 if (isInComment)
		             {
			             int i = 0;
			             while (i < line.size () && line[i] != '}')
				             i++;
			             if (i < line.size () && line[i] == '}')
			             {
							 isInComment = false;
				             i++;
				             if (i >= line.size ()) i = line.size () - 1;//overflow?
						 }
			             return LexerMachineReturn (i);
					 }
		             return {};
	             } });

	AddMachine ({ "Whitespace", 100, [](ProgramLine &line) -> std::optional<LexerMachineReturn> {
		             int i = 0;
		             while (line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\0')
		             {
			             i++;
		             }

		             if (i > 0) return LexerMachineReturn (i);
		             return {};
	             } });
	AddMachine ({ "IdRes", 70, [&](ProgramLine &line) -> std::optional<LexerMachineReturn> {
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
				                 CheckReseredWords (reservedWords,
				                                    Str_ProgramLine (Sub_ProgramLine (line, index)));
				             if (res_word.has_value ())
				             {
					             return LexerMachineReturn (index, res_word.value ());
				             }
				             return LexerMachineReturn (index, TokenInfo (TokenType::id, NoAttrib ()));
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
	    { "Real", 40, [](ProgramLine &line) -> std::optional<LexerMachineReturn> { return {}; } });
	AddMachine ({ "Integer", 60, [](ProgramLine &line) -> std::optional<LexerMachineReturn> {
		             int i = 0;
		             while (i < line.size () && std::isdigit (line[i]))
			             i++;
		             if (i > 0)
			             return LexerMachineReturn (i, TokenInfo (TokenType::integer, NoAttrib ()));
		             return {};
	             } });
	AddMachine (
	    { "Relop", 50, [](ProgramLine &line) -> std::optional<LexerMachineReturn> {
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

	std::string inFileName = "test_pascal.txt";

	if (argc == 2)
	{
		inFileName = std::string (argv[1]);
	}

	Lexer lexer;
	lexer.LoadReservedWords (reserved_words);

	try
	{

		std::fstream inFile (inFileName, std::ios::in);
		if (inFile)
		{
			std::vector<std::string> lines;
			int cur_line_number = 1;
			while (inFile.good ())
			{
				std::string line;

				std::getline (inFile, line);
				lines.push_back (line);
				if (line.size () > 0)
				{
				}

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
