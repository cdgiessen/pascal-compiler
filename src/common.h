#pragma once

#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <vector>

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

	void AddListPrint (int index, std::function<void(FILE *fp)> func)
	{
		listing[index].push_back (func);
	}
	void AddLexErrPrint (int index, std::function<void(FILE *fp)> func)
	{
		lex_errors[index].push_back (func);
	}
	void AddSynErrPrint (int index, std::function<void(FILE *fp)> func)
	{
		syn_errors[index].push_back (func);
	}

	void LogErrors ()
	{
		for (auto [key, vec] : listing)
		{
			for (auto &func : vec)
				func (listing_file.FP ());
			// for (auto [key, vec] : lex_errors)
			if (lex_errors.count (key))
				for (auto &func : lex_errors.at (key))
					func (listing_file.FP ());
			// for (auto [key, vec] : syn_errors)
			if (syn_errors.count (key))
				for (auto &func : syn_errors.at (key))
					func (listing_file.FP ());
		}
	}
	std::map<int, std::vector<std::function<void(FILE *fp)>>> listing;
	std::map<int, std::vector<std::function<void(FILE *fp)>>> lex_errors;
	std::map<int, std::vector<std::function<void(FILE *fp)>>> syn_errors;
};

class FileReader
{
	public:
	FileReader (Logger &logger, std::vector<std::string> file_list)
	: logger (logger), file_list (file_list)
	{
	}

	std::optional<std::vector<std::string>> Read ()
	{
		try
		{
			std::vector<std::string> lines;
			std::fstream inFile (file_list.at (index++), std::ios::in);
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
	Logger &logger;

	int index = 0;
	std::vector<std::string> file_list;
};

class CompilationContext
{
	public:
	CompilationContext (FileReader &dataSource) : dataSource (dataSource) {}

	SymbolTable symbolTable;
	SymbolTable literalTable;

	FileReader &dataSource;
};