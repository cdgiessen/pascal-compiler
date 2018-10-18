#pragma once

#include <string_view>
#include <unordered_set>
#include <vector>
#include <optional>
#include <fstream>

#include <cstdio>
#include <fmt/format.h>
#include <fmt/ostream.h>

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
		symbols.push_back (std::string (symbol));
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


class Logger
{
	public:
	Logger () : listing_file ("listing_file.txt"), token_file ("token_file.txt")
	{
		fmt::print (token_file.FP (), "{:^14}{:<14}{:<16}{:<14}\n", "Line No.", "Lexeme", "TOKEN-TYPE", "ATTRIBUTE");
	}

	OutputFileHandle listing_file;
	OutputFileHandle token_file;
};

class FileReader
{
	public:
	FileReader (Logger& logger, std::vector<std::string> file_list) : logger(logger), file_list (file_list) {}

	std::optional<std::vector<std::string>> Read ()
	{
		try
		{
			std::vector<std::string> lines;
			std::fstream inFile (file_list.at(index++), std::ios::in);
			if (inFile)
			{
				int cur_line_number = 1;
				while (inFile.good ())
				{
					std::string line;

					std::getline (inFile, line, '\n');
					lines.push_back (line);

					cur_line_number++;
				}
				lines.back ().push_back ('$');
			}
			else
			{
				fmt::print ("File not read, was there an error?");
			}
			return lines;
		}
		catch (const std::exception &e)
		{
			fmt::print ("Exception {}", e.what ());
		}
		return std::nullopt;
	}
	private:
		Logger& logger;

	int index = 0;
	std::vector<std::string> file_list;
};

class CompilationContext
{
	public:
		CompilationContext(FileReader& dataSource): dataSource(dataSource){}

	SymbolTable symbolTable;
	SymbolTable literalTable;

	FileReader& dataSource;
};
