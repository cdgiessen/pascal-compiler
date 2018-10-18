#include "lexer.h"

#include <cctype>
#include <cstring>

template <>
char const *enumStrings<TokenType>::data[] = {
	"PROGRAM",
	"ID",
	"PAREN_OPEN",
	"PAREN_CLOSE",
	"SEMICOLON",
	"DOT",
	"VARIABLE",
	"COLON",
	"ARRAY",
	"BRACKET_OPEN",
	"BRACKET_CLOSE",
	"NUM",
	"OF",
	"STANDARD_TYPE",
	"INTEGER",
	"REAL",
	"PROCEDURE",
	"BEGIN",
	"END",
	"CALL",
	"COMMA",
	"RELOP",
	"ADDOP",
	"ASSIGNOP",
	"MULOP",
	"NOT",
	"SIGN",
	"IF",
	"THEN",
	"ELSE",
	"WHILE",
	"DO",
	"DOT_DOT",
	"END_FILE",
	"LEXERR",
};

std::string operator+ (const std::string &out, TokenType tt)
{
	std::ostringstream ss;
	ss << enumToString (tt);
	return out + ss.str ();
}

template <> char const *enumStrings<AddOpEnum>::data[] = { "PLUS", "MINUS", "OR" };

template <> char const *enumStrings<MulOpEnum>::data[] = { "MUL", "DIV", "MOD", "AND" };

template <> char const *enumStrings<SignOpEnum>::data[] = { "PLUS", "MINUS" };

template <> char const *enumStrings<RelOpEnum>::data[] = { "EQ", "NEQ", "LT", "LEQ", "GT", "GEQ" };

template <> char const *enumStrings<StandardTypeEnum>::data[] = { "INTEGER", "REAL" };
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
	"SReal2_TooShort",
	"LReal_InvalidNumericLiteral",
	"LReal1_LeadingZero",
	"LReal2_TrailingZero",
	"LReal3_LeadingZero",
	"LReal1_TooLong",
	"LReal2_TooLong",
	"LReal2_TooShort",
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
	// if (t.index () == 10) return os << std::get<10> (t);
	return os << "ATTRIB OSTREAM NOT UPDATED!";
}

std::optional<ReservedWord> Lexer::GetReservedWord (std::string s)
{
	if (s == "program") return ReservedWord (s, TokenType::PROGRAM);
	if (s == "var") return ReservedWord (s, TokenType::VARIABLE);
	if (s == "array") return ReservedWord (s, TokenType::ARRAY);
	if (s == "of") return ReservedWord (s, TokenType::OF);
	if (s == "integer")
		return ReservedWord (s, TokenType::STANDARD_TYPE, StandardTypeEnum::integer);
	if (s == "real") return ReservedWord (s, TokenType::STANDARD_TYPE, StandardTypeEnum::real);
	// if (s == "function") return ReservedWord (s, TokenType::FUNCTION);
	if (s == "procedure") return ReservedWord (s, TokenType::PROCEDURE);
	if (s == "begin") return ReservedWord (s, TokenType::BEGIN);
	if (s == "end") return ReservedWord (s, TokenType::END);
	if (s == "if") return ReservedWord (s, TokenType::IF);
	if (s == "then") return ReservedWord (s, TokenType::THEN);
	if (s == "else") return ReservedWord (s, TokenType::ELSE);
	if (s == "while") return ReservedWord (s, TokenType::WHILE);
	if (s == "do") return ReservedWord (s, TokenType::DO);
	if (s == "not") return ReservedWord (s, TokenType::NOT);
	if (s == "or") return ReservedWord (s, TokenType::ADDOP, AddOpEnum::t_or);
	if (s == "and") return ReservedWord (s, TokenType::MULOP, MulOpEnum::t_and);
	if (s == "div") return ReservedWord (s, TokenType::MULOP, MulOpEnum::div);
	if (s == "mod") return ReservedWord (s, TokenType::MULOP, MulOpEnum::mod);
	if (s == "call") return ReservedWord (s, TokenType::CALL);
	// if (s == "read") return ReservedWord(s, TokenType::ID, Symbo)
	return {};
}

void Lexer::ReadReservedWordsFile ()
{
	std::ifstream reserved_word_file (std::string ("test_input/reserved_words.txt"));

	if (reserved_word_file)
	{

		while (reserved_word_file.good ())
		{
			std::string word, token, attribute;
			reserved_word_file >> word >> token >> attribute;
			auto res_word = GetReservedWord (word);
			if (res_word.has_value ())
				reservedWords.insert (res_word.value ());
			else
				fmt::print ("Reserved word not found! {}\n", word);
		}

		fmt::print ("Res Word size = {}\n", reservedWords.size ());
		fmt::print ("Reserved Words:\n");
		for (auto &item : reservedWords)
		{
			fmt::print ("Word: {} \t {}\n", item.word, enumToString (item.type));
		}
	}
	else
	{
		fmt::print ("Reserved Word List not found!\n");
	}
}

std::optional<TokenInfo> Lexer::CheckReseredWords (std::string_view s)
{
	for (auto &item : reservedWords)
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
		if (lexeme[0] == '$') // EOF doesn't play nice in the output
			lexeme = "EOF";
		fmt::print (logger.token_file.FP (),
		"{:^14}{:<14}{:<4}{:<12}{:<4}{:<4}\n",
		line_num,
		lexeme,
		static_cast<int> (content->type),
		enumToString ((content)->type),
		(content)->attrib.index (),
		(content)->attrib);
		if ((content)->attrib.index () == 9) // lexer error
		{ fmt::print (logger.listing_file.FP (), "{:<8}{}\n", "LEXERR:", content->attrib); } }
	else
	{
		// fmt::print (token_file.FP (), "{:^14}{:<14} {:<14}{:<4} (Unrecog Symbol)\n", line_num,
		//            lexeme, "99 (LEXERR)", "1");
	}
}

std::vector<TokenInfo> Lexer::GetTokens (CompilationContext &compContext)
{
	std::vector<TokenInfo> tokens;
	auto lines = compContext.dataSource.Read ();
	if (lines.has_value ())
	{
		LexerContext context (compContext);


		int backward_index = 0;
		int cur_line_number = 0;


		for (auto &s_line : lines.value ())
		{
			fmt::print (logger.listing_file.FP (), "{:<8}{}\n", cur_line_number, s_line);
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
							TokenFilePrinter (cur_line_number,
							buffer.substr (0, machine_ret->chars_to_eat),
							machine_ret->content);

							machine_ret->content->line_location = cur_line_number;
							machine_ret->content->column_location = backward_index - machine_ret->chars_to_eat;

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

					if (buffer[0] == '$')
						fmt::print (logger.listing_file.FP (), "{:<8}{}\t\n", "LEXERR:", "Unrecognized Symbol: EOF");
					// else
					//	fmt::print (
					//	listing_file.FP (), "{:<8}{}\t{}\n", "LEXERR:", "Unrecognized Symbol: ", buffer[0]);

					TokenFilePrinter (cur_line_number, bad_symbol, machine_ret->content);
					tokens.push_back (*machine_ret->content);
					backward_index++;
				}
				if (machine_ret.has_value () && machine_ret->content->type == TokenType::END_FILE)
				{ break; } }
			cur_line_number++;
			backward_index = 0;
		}
	}
	return tokens;
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
	/*
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
	*/
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
	{ "IdRes", 90, [&](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 if (line.size () < 1 || !std::isalpha (line[0])) return {};

		 int index = 0;
		 while (index < line.size () && std::isalnum (line[index]))
			 index++;

		 auto sv = line.substr (0, index);

		 if (index > identifier_length)
			 return LexerMachineReturn (index,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::Id_TooLong, std::string (sv))));

		 auto res_word = CheckReseredWords (sv);
		 if (res_word.has_value ()) { return LexerMachineReturn (index, res_word.value ()); }
		 int loc = context.compContext.symbolTable.AddSymbol (sv);
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
			 if (line[0] == '$')
				 return LexerMachineReturn (1, TokenInfo (TokenType::END_FILE, NoAttrib{}));
		 }
		 return {};
	 } });

	AddMachine (
	{ "LReal", 60, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 int i = 0;
		 int base_size = 0;
		 while (i < line.size () && std::isdigit (line[i]))
		 {
			 i++;
			 base_size++;
		 }
		 if (i == 0) return {};

		 if (i >= line.size () || line[i] != '.') return {};

		 i++;

		 if (i >= line.size () || !std::isdigit (line[i])) return {};
		 i++;

		 int decimal_size = 1;
		 while (i < line.size () && std::isdigit (line[i]))
		 {
			 i++;
			 decimal_size++;
		 }

		 if (i >= line.size () || line[i] != 'E') return {};

		 // LReal
		 i++;
		 bool hasSign = false;
		 if (i < line.size () && (line[i] == '+' || line[i] == '-'))
		 {
			 i++;
			 hasSign = true;
		 }
		 if (i >= line.size () || !std::isdigit (line[i])) return {};
		 i++;

		 int pow_size = 1;
		 while (i < line.size () && std::isdigit (line[i]))
		 {
			 i++;
			 pow_size++;
		 }


		 if (pow_size == 0)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::LReal3_TooShort, line.substr (0, i))));

		 // error checking

		 if (base_size > real_base_length)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::LReal1_TooLong, line.substr (0, i))));

		 if (decimal_size > real_decimal_length)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::LReal2_TooLong, line.substr (0, i))));

		 if (pow_size > real_exponent_length)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::LReal3_TooLong, line.substr (0, i))));

		 if (i > 1 && line[0] == '0')
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR,
			 LexerError (LexerErrorEnum::LReal1_LeadingZero, line.substr (0, i))));

		 if (line[base_size + decimal_size + 2 + hasSign ? 1 : 0] == '0')
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR,
			 LexerError (LexerErrorEnum::LReal3_LeadingZero, line.substr (0, i))));

		 if (decimal_size == 0)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::LReal2_TooShort, line.substr (0, i))));

		 if (pow_size == 0)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::LReal3_TooShort, line.substr (0, i))));

		 auto sub = line.substr (0, i);
		 float fval;
		 try
		 {
			 fval = std::stof (std::string (sub));
		 }
		 catch (std::exception &e)
		 {
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR,
			 LexerError (LexerErrorEnum::LReal_InvalidNumericLiteral, line.substr (0, i))));
		 }

		 return LexerMachineReturn (i, TokenInfo (TokenType::NUM, NumType (fval)));
	 } });

	AddMachine (
	{ "SReal", 55, [](LexerContext &context, ProgramLine &line) -> std::optional<LexerMachineReturn> {
		 int i = 0;
		 int base_size = 0;
		 while (i < line.size () && std::isdigit (line[i]))
		 {
			 i++;
			 base_size++;
		 }
		 if (i == 0) return {};

		 if (i >= line.size () || line[i] != '.') return {};

		 i++;

		 if (i >= line.size () || !std::isdigit (line[i])) return {};
		 i++;

		 int decimal_size = 1;
		 while (i < line.size () && std::isdigit (line[i]))
		 {
			 i++;
			 decimal_size++;
		 }

		 // error checking

		 if (base_size > real_base_length)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::SReal1_TooLong, line.substr (0, i))));

		 if (decimal_size > real_decimal_length)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::SReal2_TooLong, line.substr (0, i))));

		 if (decimal_size == 0)
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR, LexerError (LexerErrorEnum::SReal2_TooShort, line.substr (0, i))));
		 if (i > 1 && line[0] == '0')
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR,
			 LexerError (LexerErrorEnum::SReal1_LeadingZero, line.substr (0, i))));


		 auto sub = line.substr (0, i);
		 float fval;
		 try
		 {
			 fval = std::stof (std::string (sub));
		 }
		 catch (std::exception &e)
		 {
			 return LexerMachineReturn (i,
			 TokenInfo (TokenType::LEXERR,
			 LexerError (LexerErrorEnum::SReal_InvalidNumericLiteral, line.substr (0, i))));
		 }

		 return LexerMachineReturn (i, TokenInfo (TokenType::NUM, NumType (fval)));
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
		 return LexerMachineReturn (i, TokenInfo (TokenType::NUM, NumType (val)));
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

TokenStream::TokenStream (Lexer &lexer, CompilationContext &compilationContext)
: compilationContext (compilationContext), lexer (lexer)
{
	auto &new_tokens = lexer.GetTokens (compilationContext);
	tokens.insert (std::end (tokens), std::begin (new_tokens), std::end (new_tokens));
}

TokenInfo TokenStream::Current () const { return tokens.at (index); }
TokenInfo TokenStream::Advance ()
{
	if (index + 1 >= tokens.size ())
	{
		auto new_tokens = lexer.GetTokens (compilationContext);
		if (new_tokens.size () > 0)
			tokens.insert (std::end (tokens), std::begin (new_tokens), std::end (new_tokens));
	}
	if (index + 1 < tokens.size ()) index++;
	return Current ();
}