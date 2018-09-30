
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <unordered_set>

#include <unordered_map>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "lexer.h"

using Variable = int;
using Terminal = int;

struct Token
{
	bool isTerm;
	int index;
	Token () {}
	Token (bool isTerm, int index) : isTerm (isTerm), index (index) {}
};

using Rule = std::vector<Token>;

struct Production
{
	Variable var;
	Rule rule;
	Production (Variable var, Rule rule) : var (var), rule (rule) {}
};

struct Grammar
{
	std::unordered_map<int, std::string> terminals;
	std::unordered_map<int, std::string> variables;
	std::vector<Production> productions;
	int start_symbol;
};

void PrintGrammar (Grammar &grammar)
{
	OutputFileHandle ofh ("massaged.txt");

	fmt::print (ofh.FP (), "TOKENS ");
	for (auto &[key, term] : grammar.terminals)
	{
		fmt::print (ofh.FP (), "{} ", term);
	}
	fmt::print (ofh.FP (), "\n");


	for (auto &prod : grammar.productions)
	{
		fmt::print (ofh.FP (), "{} ->\n\t", grammar.variables[prod.var]);
		for (auto &token : prod.rule)
		{
			if (token.isTerm)
				fmt::print (ofh.FP (), "{} ", grammar.terminals[token.index]);
			else
				fmt::print (ofh.FP (), "{} ", grammar.variables[token.index]);
		}
		fmt::print (ofh.FP (), "\n\n");
	}
}

// bool hasEProd (Rule &rule)
// {
// 	for (auto &token : rule)
// 	{
// 		if (token.isTerm && token.size () == 1 && token[0] == 'e') { return true; }
// 	}
// 	return false;
// }

std::vector<Production> PermuteProduction (Production prod, std::vector<Variable> e_vars) {}

// std::vector<Production> RemoveEProds (std::vector<Production> &productions)
// {
// 	std::unordered_set<Variable> e_vars;
// 	for (auto &prod : productions)
// 	{
// 		if (hasEProd (prod.rule)) e_vars.insert (prod.var);
// 	}

// 	for (auto &prod : productions)
// 	{
// 		for (auto &token : prod.rule)
// 		{
// 			if (token.index () == 0)
// 			{
// 				if (e_vars.count (std::get<0> (token)) > 0) {}
// 			}
// 			else
// 			{
// 			}
// 		}

// 		std::vector<Production> newProds;
// 		return newProds;
// 	}
// }

Grammar ReadGrammar (std::ifstream &in)
{
	std::vector<std::string> lines;
	std::string line;
	while (getline (in, line))
	{
		lines.push_back (line);
	}

	Grammar grammar;
	int index = 0;

	bool isReadingProductionList = false;
	std::string var_name;
	int var_index;
	for (auto &line : lines)
	{
		if (line.size () > 2 && line[0] == '/' && line[1] == '/')
		{ // make sure its not a comment
		}
		else if (line.find ("TOKENS") == 0)
		{

			std::stringstream s;
			std::string str;
			s << line;
			s >> str; // remove TOKENS
			while (!s.eof ())
			{
				std::string str;
				s >> str;
				if (str.length () > 0) grammar.terminals[index++] = str;
			}
		}
		else if (!isReadingProductionList)
		{
			isReadingProductionList = true;
			std::stringstream s;
			s << line;
			s >> var_name;
			if (var_name.size () > 0)
			{

				var_index = index;
				grammar.variables[index++] = var_name;
			}
		}
		else if (isReadingProductionList && line.size () > 0)
		{
			std::stringstream s;
			s << line;
			Rule rule;
			while (!s.eof ())
			{
				std::string str;
				s >> str;

				bool isTerm = false;
				for (auto [key, val] : grammar.terminals)
				{
					if (str == val)
					{
						rule.push_back (Token (true, key));
						isTerm = true;
						break;
					}
				}
				if (isTerm == false)
				{
					grammar.variables[index] = str;

					rule.push_back (Token (false, index));
					index++;
				}
			}

			grammar.productions.push_back (Production (Variable (var_index), rule));
		}
		else if (line.size () == 0)
		{
			isReadingProductionList = false;
		}
	}
	return grammar;
}

void MassageGrammar (std::string grammar_fileName)
{

	std::ifstream in (grammar_fileName, std::ios::in);
	if (!in.is_open ()) { fmt::print ("failed to open {}\n", grammar_fileName); }
	else
	{
		auto grammar = ReadGrammar (in);
		PrintGrammar (grammar);
		// auto eLess_prods = RemoveEProds (prods);
	}
}

int main ()
{
	MassageGrammar ("grammars/grammar_modified.txt");
	return 0;
}