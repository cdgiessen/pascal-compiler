#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
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
	bool operator== (const Token &rhs) const { return rhs.isTerm == isTerm && rhs.index == index; }
};

using Rule = std::vector<Token>;

struct Production
{
	Variable var;
	Rule rule;
	Production (Variable var, Rule rule) : var (var), rule (rule) {}

	bool operator== (const Production &rhs) const
	{
		if ((rhs.var == var) && (rhs.rule.size () == rule.size ()))
		{
			for (int i = 0; i < rhs.rule.size (); i++)
			{

				if (rhs.rule[i].index != rule[i].index) { return false; }
			}
			return true;
		}
		else
		{
			return false;
		}
	}
};

class Grammar
{
	public:
	std::map<int, std::string> terminals;
	std::map<int, std::string> variables;
	std::vector<Production> productions;
	int start_symbol = 0; // should generally be the first production...
	int index = 0;

	void PrintGrammar (std::string out_file_name);

	int DeriveNewVariable (Variable var, std::string str);
	bool isEProd (Rule &rule) const;
	int find_epsilon_index () const;
	std::vector<Terminal> find_firsts_of_production (Production &prod);
	bool ProductionExists (Production &p) const;
	void ReorderProductionsByVariable ();
	void AddProduction (Production p);
	void EraseProduction (Production &p);
	std::vector<Production> ProductionsOfVariable (Variable var) const;
	void RemoveDuplicateProductions ();
};


struct ParseTable
{
	Grammar grammar;
	std::map<int, int> var_key_to_index;

	// row-var, col-terminal, inner is for possible multiple entries
	std::vector<std::vector<std::set<int>>> table;

	void PrintParseTable (std::string out_file_name);

	ParseTable (Grammar &grammar);
};