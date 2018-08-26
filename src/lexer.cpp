#include "lexer.h"

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

void Lexer::TokenFilePrinter (int line_num, std::string lexeme, LexerMachineReturn::ReturnVariant content)
{
	if (content.index () == 1)
	{
		fmt::print (token_file.FP (), "{:^14}{:<14}{:<14}{:<14}{:<14}\n", line_num, lexeme,
		            enumToString (std::get<1> (content).type),
		            std::get<1> (content).attrib.index (), std::get<1> (content).attrib);
	}
	else if (content.index () == 2)
	{
		fmt::print (token_file.FP (), "{:^14}{:<14}{:<14} ({} {})\n", line_num, lexeme, "99 (LEXERR)",
		            enumToString (std::get<2> (content).type), enumToString (std::get<2> (content).subType));
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
				             // if (i > line.size ()) i = line.size () - 1; // overflow?
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
				 {
					 auto sub = Sub_ProgramLine (line, i);
					 float val;
					 try
					 {
						 val = std::stof (Str_ProgramLine (sub));
					 }
					 catch (std::invalid_argument& e)
					 {
					 }
					 catch (std::out_of_range& e) {
					 
					 }
					 return LexerMachineReturn (i, TokenInfo (TokenType::real, FloatType (val)));
				 }
				 else
				 {
					 return LexerMachineReturn (i, LexerError (LexerErrorType::LReal, LexerErrorSubType::TooLong,
					                                           Sub_ProgramLine (line, i)));
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
				     return LexerMachineReturn (i, TokenInfo (TokenType::real, FloatType (val)));
				 }
				 else
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
	file_list.push_back ("test_input/test_error.txt");
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
