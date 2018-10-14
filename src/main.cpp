
#include "lexer.h"

#include "parser.h"




int main(int argc, char *argv[])
{
	auto reserved_words = ReadReservedWordsFile();

	std::vector<std::string> file_list;
	file_list.push_back("test_input/test_error.txt");
	// file_list.push_back("test_input/test_error.txt");

	// if (argc == 2) { inFileName = std::string (argv[1]); }

	Lexer lexer;
	// lexer.LoadReservedWords (reserved_words);


	try
	{

		std::fstream inFile(file_list.at(0), std::ios::in);
		if (inFile)
		{
			std::vector<std::string> lines;
			int cur_line_number = 1;
			while (inFile.good())
			{
				std::string line;

				std::getline(inFile, line, '\n');
				lines.push_back(line);

				cur_line_number++;
			}
			lines.back().push_back(EOF);
			lexer.GetTokens(reserved_words, lines);
		}
		else
		{
			fmt::print("File not read, was there an error?");
		}
	}
	catch (const std::exception &e)
	{
		fmt::print("Exception {}", e.what());
	}
	return 0;
}
