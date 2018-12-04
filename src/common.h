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

	std::string SymbolView (int loc)
	{
		if (loc > 0 && loc < symbols.size ())
			return symbols.at (loc);
		else
			throw "Symbol not found!";
	}

	void Print (OutputFileHandle &out)
	{
		fmt::print (out.FP (), "Symbol Table\n");
		int i = 0;
		for (auto &s : symbols)
		{
			fmt::print (out.FP (), "{:<6}{}\n", std::to_string (i++), s);
		}
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
	void AddSemErrPrint (int index, std::function<void(FILE *fp)> func)
	{
		sem_errors[index].push_back (func);
	}

	void LogErrors ()
	{
		for (auto [key, vec] : listing)
		{
			for (auto &func : vec)
				func (listing_file.FP ());
			if (lex_errors.count (key))
				for (auto &func : lex_errors.at (key))
					func (listing_file.FP ());
			if (syn_errors.count (key))
				for (auto &func : syn_errors.at (key))
					func (listing_file.FP ());
			if (sem_errors.count (key))
				for (auto &func : sem_errors.at (key))
					func (listing_file.FP ());
		}
	}
	std::map<int, std::vector<std::function<void(FILE *fp)>>> listing;
	std::map<int, std::vector<std::function<void(FILE *fp)>>> lex_errors;
	std::map<int, std::vector<std::function<void(FILE *fp)>>> syn_errors;
	std::map<int, std::vector<std::function<void(FILE *fp)>>> sem_errors;
};

using CodeSource = std::vector<std::string>;

class FileReader
{
	public:
	FileReader (std::vector<std::string> file_list) : file_list (file_list) {}

	std::optional<CodeSource> Read ()
	{
		try
		{
			if (index >= file_list.size ())
				return {

				};
			CodeSource lines;
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
	int index = 0;
	std::vector<std::string> file_list;
};

class CompilationContext
{
	public:
	SymbolTable symbolTable;
	SymbolTable literalTable;
};
