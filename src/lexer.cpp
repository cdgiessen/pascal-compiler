#include "lexer.h"

template <>
char const *enumStrings<TokenType>::data[] = { "PROG",
	"ID",
	"STD_TYPE",
	"INT",
	"REAL",
	"FUNC",
	"PROC",
	"ASSIGN",
	"VAR",
	"ARRAY",
	"RELOP",
	"ADDOP",
	"MULOP",
	"SIGN",
	"BEGIN",
	"END",
	"NOT",
	"OF",
	"IF",
	"THEN",
	"ELSE",
	"WHILE",
	"DO",
	"PAREN_O",
	"PAREN_C",
	"SEMIC",
	"DOT",
	"COMMA",
	"COLON",
	"BRKT_O",
	"BRKT_C",
	"DOT_DOT",
	"STR_LIT",
	"EOF",
	"LEXERR" };

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
	"SReal1_LeadingZero",
	"SReal1_TooLong",
	"SReal2_TooLong",
	"LReal_InvalidNumericLiteral",
	"LReal1_LeadingZero",
	"LReal2_TrailingZero",
	"LReal3_LeadingZero",
	"LReal1_TooLong",
	"LReal2_TooLong",
	"LReal3_TooLong",
	"LReal3_TooShort",
	"CommentContains2ndLeftCurlyBrace",
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
			std::string word, token, attribute;
			reserved_word_file >> word >> token >> attribute;
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

std::optional<TokenInfo> CheckReseredWords (ReservedWordList &list, std::string_view s)
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
	std::sort (std::begin (machines), std::end (machines), [](LexerMachine a, LexerMachine b) {
		return a.precedence > b.precedence;
	});
}

void Lexer::TokenFilePrinter (int line_num, std::string_view lexeme, LexerMachineReturn::OptionalToken content)
{
	if (content.has_value ())
	{
		if (lexeme[0] == EOF) // EOF doesn't play nice in the output
			lexeme = "EOF";
		fmt::print (token_file.FP (),
		"{:^14}{:<14}{:<4}{:<12}{:<4}{:<4}\n",
		line_num,
		lexeme,
		static_cast<int> (content->type),
		enumToString ((content)->type),
		(content)->attrib.index (),
		(content)->attrib);
		if ((content)->attrib.index () == 10) // lexer error
		{ fmt::print (listing_file.FP (), "{:<8}{}\n", "LEXERR:", content->attrib); } }
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


	for (auto& s_line : lines)
	{
		fmt::print (listing_file.FP (), "{:<8}{}\n", cur_line_number, s_line);
		while (backward_index < s_line.size ())
		{
			std::string_view buffer = std::string_view (s_line).substr (backward_index, s_line.size ());

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
						TokenFilePrinter (
						cur_line_number, buffer.substr (0, machine_ret->chars_to_eat), machine_ret->content);

						machine_ret->content->line_location = cur_line_number;
						machine_ret->content->column_location = backward_index;

						tokens.push_back (*machine_ret->content);
					}
				}
			}
			else
			{
				std::string bad_symbol;
				bad_symbol.push_back (buffer[0]);

				machine_ret = LexerMachineReturn (1,
				TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::Unrecognized_Symbol, bad_symbol)));

				if (buffer[0] == EOF)
					fmt::print (listing_file.FP (), "{:<8}{}\t\n", "LEXERR:", "Unrecognized Symbol: EOF");
				else
					fmt::print (
					listing_file.FP (), "{:<8}{}\t{}\n", "LEXERR:", "Unrecognized Symbol: ", buffer[0]);

				TokenFilePrinter (cur_line_number, bad_symbol, machine_ret->content);
				tokens.push_back (*machine_ret->content);
				backward_index++;
			}
			if (machine_ret.has_value() && machine_ret->content->type == TokenType::END_FILE)
			{
				break;
			}
		}
		cur_line_number++;
		backward_index = 0;
	}
	return TokenStream (tokens, context.symbolTable);
}

void Lexer::CreateMachines ()
{
	AddMachine (
	{ "Comment", 110, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 int i = 0;
		 if (line.length () > 0 && line[0] == '{')
		 {
			 context.isInComment = true;
			 i++;
		 }
		 if (context.isInComment)
		 {
			 if (i < line.length ())
			 {
				 while (i < line.length () && line[i] != '}')
				 {
					 if (line[i] == '{')
					 {
						 return LexerMachineReturn (i + 1,
						 TokenInfo (TokenType::LEXERR,
						 LexerError (LexerErrorEnum::CommentContains2ndLeftCurlyBrace, line.substr (0, i + 1))));
					 }
					 i++;
				 }
				 if (i < line.length () && line[i] == '}')
				 {
					 context.isInComment = false;
					 i++;
				 }
				 return LexerMachineReturn (i);
			 }
			 return LexerMachineReturn (i);
		 }
		 return {};
	 } });

	AddMachine (
	{ "String-Literal", 95, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.length () == 0 || line[0] != '\'') return {};


		 int i = 1;
		 while (i < line.length () && line[i] != '\'')

			 i++;

		 if (i >= line.length () || line[i] != '\'')
		 {
			 fmt::print ("{}\n", line);
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::StrLit_NotTerminated, line)));
		 }
		 i++; // needs to include the extra tick mark
		 auto full_s = line.substr (0, i);

		 std::string_view tickless_s = std::string_view (line.data () + 1, i - 1);
		 if (i - 2 <= string_literal_length)
		 {
			 int loc = context.literalTable.AddSymbol (tickless_s);
			 return LexerMachineReturn (i, TokenInfo (TokenType::STR_LIT, StringLiteral (loc)));
		 }
		 else
		 {
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::StrLit_TooLong, std::string (full_s))));
		 }
		 return LexerMachineReturn (i,
		 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::StrLit_TooLong, std::string (full_s))));
	 } });

	AddMachine ({ "Whitespace", 100, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		             int i = 0;
		             while (i < line.length () && (line[i] == ' ' || line[i] == '\t' || line[i] == '\n'))
		             {
			             i++;
		             }
		             if (i > 0) { return LexerMachineReturn (i); }
		             return {};
	             } });

	AddMachine (
	{ "IdRes", 90, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.size () < 1 || !std::isalpha (line[0])) return {};

		 int index = 0;
		 while (index < line.size () && std::isalnum (line[index]))
			 index++;

		 auto sv = line.substr (0, index);

		 if (index > identifier_length)
			 return LexerMachineReturn (index,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::Id_TooLong, std::string (sv))));

		 auto res_word = CheckReseredWords (context.reservedWords, sv);
		 if (res_word.has_value ()) { return LexerMachineReturn (index, res_word.value ()); }
		 int loc = context.symbolTable.AddSymbol (sv);
		 return LexerMachineReturn (index, TokenInfo (TokenType::ID, SymbolType (loc)));
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
		 if (i >= line.size () || line[i] != '.') return {};
		 i++;
		 while (i < line.size () && std::isdigit (line[i]))
		 {
			 i++;
			 decimal_size++;
		 }
		 if (i >= line.size ()) return {};

		 if (line[i] == 'E')
		 {
			 i++;
			 bool hasSign = false;
			 bool signVal = false;
			 if (line[i] == '+') {
				 hasSign = true; 
				 signVal = true; 
				 i++;
			 }
			 else if (line[i] == '-') {
				 hasSign = true; //signVal already false
				 i++;
			 }


			 while (i < line.size () && std::isdigit (line[i]))
			 {
				 i++;
				 pow_size++;
			 }
			 //error checking enum. Really just to reduce # of return statements...
			 LexerErrorEnum errorType = LexerErrorEnum::Unrecognized_Symbol;
			 if (i > 1 && line[0] == '0')			 errorType = LexerErrorEnum::LReal1_LeadingZero;
			 if (base_size > real_base_length)		 errorType = LexerErrorEnum::LReal1_TooLong;
			 if (decimal_size > real_decimal_length) errorType = LexerErrorEnum::LReal2_TooLong;
			 if (line[base_size + decimal_size] == '0')	 errorType = LexerErrorEnum::LReal2_TrailingZero;
			 if (pow_size > real_exponent_length)	 errorType = LexerErrorEnum::LReal3_TooLong;
			 if (pow_size == 0)                      errorType = LexerErrorEnum::LReal3_TooShort;
			 if (line[base_size + decimal_size + 2 + hasSign ? signVal : 0] == '0')
				 errorType = LexerErrorEnum::LReal3_LeadingZero;

			 if (errorType != LexerErrorEnum::Unrecognized_Symbol) {
				 return LexerMachineReturn(
					 i, TokenInfo(TokenType::LEXERR, LexerError(errorType, line.substr(0, i))));
			 }

			 auto sub = line.substr (0, i);
			 float val;
			 try
			 {
				 val = std::stof (std::string (sub));
			 }
			 catch (std::exception &e)
			 {
				 return LexerMachineReturn (i,
				 TokenInfo (TokenType::LEXERR,
				 LexerError (LexerErrorEnum::LReal_InvalidNumericLiteral, line.substr (0, i))));
			 }
			 return LexerMachineReturn (i, TokenInfo (TokenType::REAL, FloatType (val)));
		 }

		 if (i > 1 && line[0] == '0')
			 return LexerMachineReturn(
				 i, TokenInfo(TokenType::LEXERR, LexerError(LexerErrorEnum::SReal1_LeadingZero, line.substr(0, i))));
		 if (base_size > real_base_length)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::SReal1_TooLong, line.substr (0, i))));
		 if (decimal_size > real_decimal_length)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::SReal2_TooLong, line.substr (0, i))));
		 

		 auto sub = line.substr (0, i);
		 float val;
		 try
		 {
			 val = std::stof (std::string (sub));
		 }
		 catch (std::exception &e)
		 {
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR,
			 LexerError (LexerErrorEnum::SReal_InvalidNumericLiteral, line.substr (0, i))));
		 }
		 return LexerMachineReturn (i, TokenInfo (TokenType::REAL, FloatType (val)));
	 } });

	AddMachine (
	{ "Integer", 50, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 int i = 0;
		 while (i < line.size () && std::isdigit (line[i]))
		 {
			 i++;
		 }
		 if (i == 0) return {};

		 auto seq = line.substr (0, i);
		 int val = 0;

		 if (i > integer_digit_length)

			 return LexerMachineReturn (
			 i, TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::Int_TooLong, seq)));

		 if (i > 1 && line[0] == '0')
			 return LexerMachineReturn (
			 i, TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::Int_LeadingZero, seq)));


		 try
		 {
			 val = std::stoi (std::string (seq));
		 }
		 catch (std::out_of_range e)
		 {
			 return LexerMachineReturn (
			 i, TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::Int_TooLong, seq)));
		 }
		 catch (std::invalid_argument e)
		 {
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::Int_InvalidNumericLiteral, seq)));
		 }
		 return LexerMachineReturn (i, TokenInfo (TokenType::INTEGER, IntType (val)));
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
