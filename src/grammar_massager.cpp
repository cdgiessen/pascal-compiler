
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
	int start_symbol = 0; // should generally be the first production...
};

void PrintGrammar (Grammar &grammar, OutputFileHandle &ofh)
{
	fmt::print (ofh.FP (), "TOKENS ");
	for (auto & [key, term] : grammar.terminals)
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

bool isEProd (Rule &rule, Grammar &grammar)
{
	for (auto &token : rule)
	{
		if (token.isTerm && grammar.terminals[token.index].size () == 1 &&
		    grammar.terminals[token.index][0] == 'e')
		{
			return true;
		}
		else if (!token.isTerm && grammar.variables[token.index].size () == 1 &&
		         grammar.variables[token.index][0] == 'e')
		{
			return true;
		}
	}
	return false;
}

std::vector<Rule> PermuteRule (Rule original_rule, std::vector<bool> &found_e_vars, int iteration)
{
	if (iteration == found_e_vars.size () - 1) {
		std::vector<Rule> new_rules;
		Rule r;
		r.insert (std::begin (r), original_rule[iteration]);
		new_rules.push_back (r);
		// if(found_e_vars[iteration])
		return new_rules;
	} // base

	// get all rule combos with (iteration + 1)
	auto rules = PermuteRule (original_rule, found_e_vars, iteration + 1);

	// append current token character to rules into new_rules
	std::vector<Rule> new_rules;
	for (auto &rule : rules)
	{
		Rule r = rule;
		r.insert (std::begin (r), original_rule[iteration]);
		new_rules.push_back (r);
	}
	// if its a varable with e_prod, also add the rules without it being added
	if (found_e_vars[iteration]) {
		new_rules.insert (std::end (new_rules), std::begin (rules), std::end (rules));
	}
	return new_rules;
}

std::vector<Production> PermuteProductionOnERemoval (Production prod, std::unordered_set<Variable> &e_vars)
{
	std::vector<Production> out_productions;

	// find all e_prod variables in the rule
	std::vector<bool> found_e_vars;
	found_e_vars.resize (prod.rule.size ());

	for (int i = 0; i < prod.rule.size (); i++)
	{
		if (!prod.rule[i].isTerm) {
			auto it = std::find (std::begin (e_vars), std::end (e_vars), prod.rule[i].index);
			if (it != std::end (e_vars)) {
				found_e_vars[i] = true;
			}
		}
	}

	auto rules = PermuteRule (prod.rule, found_e_vars, 0);
	for (auto &r : rules)
	{
		out_productions.push_back (Production (prod.var, r));
	}

	return out_productions;
}

Grammar RemoveEProds (Grammar &grammar)
{

	std::unordered_set<Variable> e_vars;

	// finds direct e prods
	for (auto &prod : grammar.productions)
	{
		if (isEProd (prod.rule, grammar)) e_vars.insert (prod.var);
	}


	Grammar res_grammar;
	res_grammar.terminals = grammar.terminals;
	res_grammar.variables = grammar.variables;


	for (auto &prod : grammar.productions)
	{
		if (!isEProd (prod.rule, grammar)) {
			auto new_prods = PermuteProductionOnERemoval (prod, e_vars);
			if (new_prods.size () > 0)
				res_grammar.productions.insert (
				std::end (res_grammar.productions), std::begin (new_prods), std::end (new_prods));
			else
				res_grammar.productions.push_back (prod);
		}
	}
	return res_grammar;
}

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
		if (line.size () > 2 && line[0] == '/' && line[1] == '/') { // make sure its not a comment
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
			if (var_name.size () > 0) {
				bool isNew = true;
				for (auto[key, val] : grammar.variables)
				{
					if (var_name == val) {
						var_index = key;
						isNew = false;
					}
				}
				if (isNew) {
					var_index = index;
					grammar.variables[index++] = var_name;
				}
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
				for (auto[key, val] : grammar.terminals)
				{
					if (str == val) {
						rule.push_back (Token (true, key));
						isTerm = true;
					}
				}
				if (isTerm == false) {
					bool isNew = true;
					for (auto[key, val] : grammar.variables)
					{
						if (str == val) {
							rule.push_back (Token (false, key));
							isNew = false;
						}
					}
					if (isNew) {
						grammar.variables[index] = str;

						rule.push_back (Token (false, index));
						index++;
					}
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
	if (!in.is_open ()) {
		fmt::print ("failed to open {}\n", grammar_fileName);
	}
	else
	{
		auto grammar = ReadGrammar (in);
		OutputFileHandle ofh_orig ("massaged.txt");
		PrintGrammar (grammar, ofh_orig);

		auto eLess_Grammar = RemoveEProds (grammar);
		OutputFileHandle ofh_e_less ("e_less.txt");
		PrintGrammar (eLess_Grammar, ofh_e_less);
	}
}

int main ()
{
	MassageGrammar ("grammars/grammar_modified.txt");
	return 0;
}