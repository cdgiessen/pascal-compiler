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

#include "common.h"

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

	void PrintTokenList(FILE *fp);

	void PrintGrammar (std::string out_file_name);
	void PrintGrammar(std::string out_file_name, std::function<void(FILE* fp, int var)> variable_decorator);

	int DeriveNewVariable (Variable var, std::string str);
	int CreateNewVariable (Variable var, std::string str);
	bool isEProd (Rule &rule) const;

	int find_epsilon_index () const;
	int find_eof_index () const;

	std::vector<Terminal> find_firsts_of_production (Production &prod);

	bool ProductionExists (Production &p) const;

	void AddProduction (Production p);

	void EraseProduction (Production &p);

	void ReorderProductionsByVariable ();
	void RemoveDuplicateProductions ();

	std::vector<Production> ProductionsOfVariable (Variable var) const;

	std::map<int, std::string> ProperIndexes();
};

class FirstsAndFollows
{
	public:
	FirstsAndFollows (Grammar &grammar);

	void FindFirsts ();
	void FindFollows ();

	std::set<int> GetFirstsOfRule(Rule& rule, int epsilon_index);

	void PrintFirst(FILE *fp, int key);
	void PrintFollow(FILE *fp, int key);

	void Print (std::string outFileName);
	void PrintWithGrammar(std::string outFileName);

	std::map<int, std::set<int>> firsts;
	std::map<int, std::set<int>> follows;

	Grammar &grammar;
};

struct ParseTable
{
	ParseTable (Grammar &grammar);

	void PrettyPrintParseTableCSV (std::string out_file_name);
	void PrintParseTableCSV (std::string out_file_name);

	Grammar grammar;

	FirstsAndFollows firstAndFollows;
	std::map<int, int> var_key_to_index;
	std::map<int, int> term_key_to_index;

	// row-var, col-terminal, inner is for possible multiple entries
	std::vector<std::vector<std::set<int>>> table;
};