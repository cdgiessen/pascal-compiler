#include "lexer.h"

template <>
char const *enumStrings<TokenType>::data[] =
{ "PROG",   "ID",     "STD_TYPE", "INT",      "REAL",  "FUNC",  "PROC",  "ASSIGN",
  "VAR",    "ARRAY",  "RELOP",    "SIMP_EXP", "ADDOP", "TERM",  "MULOP", "FACT",
  "SIGN",   "BEGIN",  "END",      "NOT",      "OF",    "IF",    "THEN",  "ELSE",
  "WHILE",  "DO",     "PAREN_O",  "PAREN_C",  "SEMIC", "DOT",   "COMMA", "COLON",
  "BRKT_O", "BRKT_C", "DOT_DOT",  "STR_LIT",  "EOF",   "LEXERR" };

template <> char const *enumStrings<StandardEnum>::data[] = { "INT", "REAL" };

template <> char const *enumStrings<AddOpEnum>::data[] = { "PLUS", "MINUS", "OR" };

template <> char const *enumStrings<MulOpEnum>::data[] = { "MUL", "DIV", "MOD", "AND" };

template <> char const *enumStrings<SignOpEnum>::data[] = { "PLUS", "MINUS" };

template <> char const *enumStrings<RelOpEnum>::data[] = { "EQ", "NEQ", "LT", "LEQ", "GT", "GEQ" };

template <>
char const *enumStrings<LexerErrorEnum>::data[] = {
	"Unrecognized_Symbol",
	"Id_TooLong",
	"StrLit_TooLong",
	"StrLit_StrLit_NotTerminated",
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
	if (t.index () == 10) return os << std::get<10> (t);
	return os << "ATTRIB OSTREAM NOT UPDATED!";
}

std::optional<ReservedWord> GetReservedWord (std::string s)
{
	if (s == "program") return ReservedWord (s, TokenType::PROGRAM, NoAttrib{});
	if (s == "var") return ReservedWord (s, TokenType::VARIABLE, NoAttrib{});
	if (s == "array") return ReservedWord (s, TokenType::ARRAY, NoAttrib{});
	if (s == "of") return ReservedWord (s, TokenType::OF, NoAttrib{});
	if (s == "integer") return ReservedWord (s, TokenType::STANDARD_TYPE, StandardEnum::integer);
	if (s == "real") return ReservedWord (s, TokenType::STANDARD_TYPE, StandardEnum::real);
	if (s == "function") return ReservedWord (s, TokenType::FUNCTION, NoAttrib{});
	if (s == "procedure") return ReservedWord (s, TokenType::PROCEDURE, NoAttrib{});
	if (s == "begin") return ReservedWord (s, TokenType::BEGIN, NoAttrib{});
	if (s == "end") return ReservedWord (s, TokenType::END, NoAttrib{});
	if (s == "if") return ReservedWord (s, TokenType::IF, NoAttrib{});
	if (s == "then") return ReservedWord (s, TokenType::THEN, NoAttrib{});
	if (s == "else") return ReservedWord (s, TokenType::ELSE, NoAttrib{});
	if (s == "while") return ReservedWord (s, TokenType::WHILE, NoAttrib{});
	if (s == "do") return ReservedWord (s, TokenType::DO, NoAttrib{});
	if (s == "not") return ReservedWord (s, TokenType::NOT, NoAttrib{});
	if (s == "or") return ReservedWord (s, TokenType::ADDOP, AddOpEnum::t_or);
	if (s == "and") return ReservedWord (s, TokenType::MULOP, MulOpEnum::t_and);
	if (s == "div") return ReservedWord (s, TokenType::MULOP, MulOpEnum::div);
	if (s == "mod") return ReservedWord (s, TokenType::MULOP, MulOpEnum::mod);
	// if (s == "read") return ReservedWord(s, TokenType::ID, Symbo)
	return {};
}

ReservedWordList ReadReservedWordsFile ()
{
	ReservedWordList res_words;

	std::ifstream reserved_word_file (std::string ("test_input/reserved_words.txt"));

	if (reserved_word_file)
	{

		while (reserved_word_file.good ())
		{
			std::string word;
			reserved_word_file >> word;
			auto res_word = GetReservedWord (word);
			if (res_word.has_value ())
				res_words.insert (res_word.value ());
			else
				fmt::print ("Reserved word not found! {}\n", word);
		}

		fmt::print ("Res Word size = {}\n", res_words.size ());
		fmt::print ("Reserved Words:\n");
		for (auto &item : res_words)
		{
			fmt::print ("Word: {} \t {}\n", item.word, enumToString (item.type));
		}
	}
	else
	{
		fmt::print ("Reserved Word List not found!\n");
	}
	return res_words;
}

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

ProgramLine Sub_ProgramLine (const ProgramLine &line, int firstIndex, int indexesToCopy)
{
	if (indexesToCopy >= line.size ())
		return ProgramLine (line);
	else
	{
		if (firstIndex >= line.size ()) { return ProgramLine (); }
		if (firstIndex + indexesToCopy >= line.size ())
		{
			ProgramLine output (line.size () - firstIndex);
			std::memcpy (output.data (), &line[firstIndex], line.size () - firstIndex);
			return output;
		}
		ProgramLine output (indexesToCopy);
		std::memcpy (output.data (), &line[firstIndex], indexesToCopy);
		return output;
	}
}


std::optional<TokenInfo> CheckReseredWords (ReservedWordList &list, std::string s)
{
	for (auto &item : list)
	{
		if (item == s) return item.GetToken ();
	}
	return {};
}

void Lexer::AddMachine (LexerMachine &&machine)
{
	machines.push_back (std::move (machine));
	std::sort (std::begin (machines), std::end (machines),
	           [](LexerMachine a, LexerMachine b) { return a.precedence > b.precedence; });
}

void Lexer::TokenFilePrinter (int line_num, std::string lexeme, LexerMachineReturn::OptionalToken content)
{
	if (content.has_value ())
	{
		if (lexeme[0] == EOF) // EOF doesn't play nice in the output
			lexeme = "EOF";
		fmt::print (token_file.FP (), "{:^14}{:<14}{:<4}{:<12}{:<4}{:<4}\n", line_num, lexeme,
		            static_cast<int> (content->type), enumToString ((content)->type),
		            (content)->attrib.index (), (content)->attrib);
		if ((content)->attrib.index () == 10) // lexer error
		{ fmt::print (listing_file.FP (), "LEXERR:\t{}\n", content->attrib); } }
	else
	{
		// fmt::print (token_file.FP (), "{:^14}{:<14} {:<14}{:<4} (Unrecog Symbol)\n", line_num,
		//            lexeme, "99 (LEXERR)", "1");
	}
}

TokenStream Lexer::GetTokens (ReservedWordList &list, std::vector<std::string> lines)
{
	LexerContext context (list);

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
				machine_ret = iter->machine (context, buffer);

				iter++;
			}
			if (machine_ret.has_value ())
			{
				if (machine_ret->chars_to_eat > 0 && machine_ret->chars_to_eat < line_buffer_length)
				{

					backward_index += machine_ret->chars_to_eat;
					if (machine_ret->content.has_value ())
					{
						TokenFilePrinter (cur_line_number,
						                  Str_ProgramLine (Sub_ProgramLine (buffer, machine_ret->chars_to_eat)),
						                  machine_ret->content);

						machine_ret->content->line_location = cur_line_number;
						machine_ret->content->column_location = backward_index;

						tokens.push_back (*machine_ret->content);
					}
				}
			}
			else
			{

				machine_ret =
				LexerMachineReturn (1, TokenInfo (TokenType::LEXERR,
				                                  LexerError (LexerErrorEnum::Unrecognized_Symbol,
				                                              Sub_ProgramLine (buffer, 1))));

				if (buffer[0] == EOF)
					fmt::print (listing_file.FP (), "LEXERR:\t{}\t\n", "Unrecognized Symbol: EOF");
				else
					fmt::print (listing_file.FP (), "LEXERR:\t{}\t{}\n", "Unrecognized Symbol: ", buffer[0]);
				std::string s;
				s.append (1, buffer[0]);
				TokenFilePrinter (cur_line_number, s, machine_ret->content);
				tokens.push_back (*machine_ret->content);
				backward_index++;
			}
		}
		cur_line_number++;
		backward_index = 0;
	}
	return TokenStream (tokens, context.symbolTable);
}

void Lexer::CreateMachines ()
{
	AddMachine ({ "Comment", 110, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		             if (line.size () > 0 && line[0] == '{') { context.isInComment = true; }
		             if (context.isInComment)
		             {
			             int i = 0;
			             while (i < line.size () && line[i] != '}')
				             i++;
			             if (i < line.size () && line[i] == '}')
			             {
				             context.isInComment = false;
				             i++;
				             // if (i > line.size ()) i = line.size () - 1; // overflow?
			             }
			             return LexerMachineReturn (i);
		             }
		             return {};
	             } });

	AddMachine (
	{ "String-Literal", 95, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.size () > 0 && line[0] == '\'')
		 {
			 int i = 1;

			 while (i < line.size () && line[i] != '\'')
			 {
				 i++;
			 }
			 i++; // needs to include the extra tick mark
			 if (i < line.size ())
			 {
				 std::string s = Str_ProgramLine (Sub_ProgramLine (line, i - 2, 1));

				 if (i - 2 <= string_literal_length)
				 {
					 int loc = context.literalTable.AddSymbol (std::move (s));
					 return LexerMachineReturn (i, TokenInfo (TokenType::STR_LIT, StringLiteral (loc)));
				 }
				 else
				 {
					 return LexerMachineReturn (i, TokenInfo (TokenType::LEXERR,
					                                          LexerError (LexerErrorEnum::StrLit_TooLong,
					                                                      Sub_ProgramLine (line, i))));
				 }
				 return LexerMachineReturn (i, TokenInfo (TokenType::LEXERR,
				                                          LexerError (LexerErrorEnum::StrLit_TooLong,
				                                                      Sub_ProgramLine (line, i))));
			 }
			 return LexerMachineReturn (i, TokenInfo (TokenType::LEXERR,
			                                          LexerError (LexerErrorEnum::StrLit_NotTerminated,
			                                                      Sub_ProgramLine (line, i))));
		 }
		 return {};
	 } });

	AddMachine ({ "Whitespace", 100, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		             int i = 0;
		             while (line[i] == ' ' || line[i] == '\t' || line[i] == '\n')
		             {
			             i++;
		             }

		             if (i > 0) return LexerMachineReturn (i);
		             return {};
	             } });

	AddMachine (
	{ "IdRes", 90, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.size () >= 1 && std::isalpha (line[0]))
		 {
			 int index = 0;
			 while (index < line.size () && std::isalnum (line[index]))
			 {
				 index++;
			 }
			 if (index <= identifier_length)
			 {
				 auto res_word = CheckReseredWords (context.reservedWords,
				                                    Str_ProgramLine (Sub_ProgramLine (line, index)));
				 if (res_word.has_value ())
				 { return LexerMachineReturn (index, res_word.value ()); }
				 int loc = context.symbolTable.AddSymbol (Str_ProgramLine (Sub_ProgramLine (line, index)));
				 return LexerMachineReturn (index, TokenInfo (TokenType::ID, SymbolType (loc)));
			 }
			 return LexerMachineReturn (index, TokenInfo (TokenType::LEXERR,
			                                              LexerError (LexerErrorEnum::Id_TooLong,
			                                                          Sub_ProgramLine (line, index))));
		 }
		 return {};
	 } });

	AddMachine (
	{ "Catch-all", 80, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.size () >= 2)
		 {
			 if (line[0] == ':' && line[1] == '=')
				 return LexerMachineReturn (2, TokenInfo (TokenType::ASSIGNOP, NoAttrib ()));
			 if (line[0] == '.' && line[1] == '.')
				 return LexerMachineReturn (2, TokenInfo (TokenType::DOT_DOT, NoAttrib ()));
		 }
		 if (line.size () >= 1)
		 {
			 if (line[0] == '\0')
				 return LexerMachineReturn (1, TokenInfo (TokenType::END_FILE, NoAttrib ()));
			 if (line[0] == '(')
				 return LexerMachineReturn (1, TokenInfo (TokenType::PAREN_OPEN, NoAttrib ()));
			 if (line[0] == ')')
				 return LexerMachineReturn (1, TokenInfo (TokenType::PAREN_CLOSE, NoAttrib ()));
			 if (line[0] == ';')
				 return LexerMachineReturn (1, TokenInfo (TokenType::SEMICOLON, NoAttrib ()));
			 if (line[0] == '.')
				 return LexerMachineReturn (1, TokenInfo (TokenType::DOT, NoAttrib ()));
			 if (line[0] == ',')
				 return LexerMachineReturn (1, TokenInfo (TokenType::COMMA, NoAttrib ()));
			 if (line[0] == ':')
				 return LexerMachineReturn (1, TokenInfo (TokenType::COLON, NoAttrib ()));
			 if (line[0] == '[')
				 return LexerMachineReturn (1, TokenInfo (TokenType::BRACKET_OPEN, NoAttrib ()));
			 if (line[0] == ']')
				 return LexerMachineReturn (1, TokenInfo (TokenType::BRACKET_CLOSE, NoAttrib ()));
			 if (line[0] == '+')
				 return LexerMachineReturn (1, TokenInfo (TokenType::SIGN, SignOpEnum::plus));
			 if (line[0] == '-')
				 return LexerMachineReturn (1, TokenInfo (TokenType::SIGN, SignOpEnum::minus));
			 if (line[0] == EOF)
				 return LexerMachineReturn (1, TokenInfo (TokenType::END_FILE, NoAttrib{}));
		 }
		 return {};
	 } });

	AddMachine (
	{ "Real", 60, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
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
				 {
					 auto sub = Sub_ProgramLine (line, i);
					 float val;
					 try
					 {
						 val = std::stof (Str_ProgramLine (sub));
					 }
					 catch (std::invalid_argument &e)
					 {
					 }
					 catch (std::out_of_range &e)
					 {
					 }
					 return LexerMachineReturn (i, TokenInfo (TokenType::REAL, FloatType (val)));
				 }
				 else
				 {
					 return LexerMachineReturn (i, TokenInfo (TokenType::LEXERR,
					                                          LexerError (LexerErrorEnum::LReal1_TooLong,
					                                                      Sub_ProgramLine (line, i))));
				 }
			 }
			 else
			 {
				 if (base_size < real_base_length && decimal_size < real_decimal_length)
				 {
					 auto sub = Sub_ProgramLine (line, i);
					 float val;
					 try
					 {
						 val = std::stof (Str_ProgramLine (sub));
					 }
					 catch (std::invalid_argument &e)
					 {
					 }
					 catch (std::out_of_range &e)
					 {
					 }
					 return LexerMachineReturn (i, TokenInfo (TokenType::REAL, FloatType (val)));
				 }
				 else
				 {
					 return LexerMachineReturn (i, TokenInfo (TokenType::LEXERR,
					                                          LexerError (LexerErrorEnum::SReal1_TooLong,
					                                                      Sub_ProgramLine (line, i))));
				 }
			 }
		 }
		 return {};
	 } });

	AddMachine (
	{ "Integer", 50, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
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

				 if (i <= integer_digit_length)
				 {
					 if (i > 1 && line[0] == '0')
					 {
						 return LexerMachineReturn (i, TokenInfo (TokenType::LEXERR,
						                                          LexerError (LexerErrorEnum::Int_LeadingZero, seq)));
					 }
					 else
					 {
						 return LexerMachineReturn (i, TokenInfo (TokenType::INTEGER, IntType (val)));
					 }
				 }
				 else
				 {
					 return LexerMachineReturn (i, TokenInfo (TokenType::LEXERR,
					                                          LexerError (LexerErrorEnum::Int_TooLong, seq)));
				 }
			 }
			 catch (std::out_of_range e)
			 {
				 return LexerMachineReturn (i, TokenInfo (TokenType::LEXERR,
				                                          LexerError (LexerErrorEnum::Int_TooLong, seq)));
			 }
			 catch (std::invalid_argument e)
			 {
				 return LexerMachineReturn (i, TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::Int_InvalidNumericLiteral,
				                                                                         seq)));
			 }
		 }
		 return {};
	 } });

	AddMachine (
	{ "Relop", 70, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.size () >= 2)
		 {
			 if (line[0] == '>' && line[1] == '=')
				 return LexerMachineReturn (2, TokenInfo (TokenType::RELOP, RelOpEnum::greater_than_or_equal));
			 if (line[0] == '<' && line[1] == '=')
				 return LexerMachineReturn (2, TokenInfo (TokenType::RELOP, RelOpEnum::less_than_or_equal));
			 if (line[0] == '<' && line[1] == '>')
				 return LexerMachineReturn (2, TokenInfo (TokenType::RELOP, RelOpEnum::not_equal));
		 }
		 if (line[0] == '>')
			 return LexerMachineReturn (1, TokenInfo (TokenType::RELOP, RelOpEnum::greater_than));
		 if (line[0] == '<')
			 return LexerMachineReturn (1, TokenInfo (TokenType::RELOP, RelOpEnum::less_than));
		 if (line[0] == '=')
			 return LexerMachineReturn (1, TokenInfo (TokenType::RELOP, RelOpEnum::equal));
		 return {};
	 } });
}

int main (int argc, char *argv[])
{
	auto reserved_words = ReadReservedWordsFile ();

	std::vector<std::string> file_list;
	file_list.push_back ("test_input/test_error.txt");
	// file_list.push_back("test_input/test_error.txt");

	// if (argc == 2) { inFileName = std::string (argv[1]); }

	Lexer lexer;
	// lexer.LoadReservedWords (reserved_words);


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

				std::getline (inFile, line, '\n');
				lines.push_back (line);

				cur_line_number++;
			}
			lines.back ().push_back (EOF);
			lexer.GetTokens (reserved_words, lines);
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
