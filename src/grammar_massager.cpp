
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
	bool operator== (const Token &rhs) const { return rhs.isTerm == isTerm && rhs.index == index; }
};

using Rule = std::vector<Token>;

struct Production
{
	Variable var;
	Rule rule;
	Production (Variable var, Rule rule) : var (var), rule (rule) {}

	bool operator== (const Production &rhs) const { return rhs.var == var && rhs.rule == rule; }
};

struct Grammar
{
	std::unordered_map<int, std::string> terminals;
	std::unordered_map<int, std::string> variables;
	std::vector<Production> productions;
	int start_symbol = 0; // should generally be the first production...
	int index = 0;
};

void PrintGrammar (Grammar &grammar, OutputFileHandle &ofh)
{
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

bool isEProd (Rule &rule, Grammar &grammar)
{
	for (auto &token : rule)
	{
		if (token.isTerm && grammar.terminals[token.index].size () == 1 &&
		    grammar.terminals[token.index][0] == 'e')
		{ return true; } else if (!token.isTerm && grammar.variables[token.index].size () == 1 &&
		                          grammar.variables[token.index][0] == 'e')
		{
			return true;
		}
	}
	return false;
}

std::vector<Rule> PermuteRule (Rule original_rule, std::vector<bool> &found_e_vars, int iteration)
{
	if (iteration == found_e_vars.size () - 1)
	{
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
	if (found_e_vars[iteration])
	{ new_rules.insert (std::end (new_rules), std::begin (rules), std::end (rules)); } return new_rules;
}

std::vector<Production> PermuteProductionOnERemoval (Production prod, std::unordered_set<Variable> &e_vars)
{
	std::vector<Production> out_productions;

	// find all e_prod variables in the rule
	std::vector<bool> found_e_vars;
	found_e_vars.resize (prod.rule.size ());

	for (int i = 0; i < prod.rule.size (); i++)
	{
		if (!prod.rule[i].isTerm)
		{
			auto it = std::find (std::begin (e_vars), std::end (e_vars), prod.rule[i].index);
			if (it != std::end (e_vars)) { found_e_vars[i] = true; }
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
	res_grammar.index = grammar.index;


	for (auto &prod : grammar.productions)
	{
		if (!isEProd (prod.rule, grammar))
		{
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

void RemoveImmediateLeftRecursion (Variable var, Grammar &grammar)
{
	bool hasILR = false;
	for (auto &prod : grammar.productions)
	{
		if (prod.var == var && prod.rule.size () > 0 && !prod.rule[0].isTerm && prod.rule[0].index == var)
		{ hasILR = true; } }

	if (hasILR)
	{

		// find all productions with this variable as its left side
		std::vector<Production> prods;
		for (auto it = std::begin (grammar.productions); it != std::end (grammar.productions);)
		{
			if ((*it).var == var && (*it).rule.size () > 0) //&& !(*it).rule[0].isTerm && (*it).rule[0].index == var)
			{
				prods.push_back ((*it));
				grammar.productions.erase (it);
			}
			else
			{
				it++;
			}
		}

		fmt::print ("Immediately recursive {}\n", prods.size ());

		if (prods.size () > 0)
		{

			// create a new variable
			std::string new_prod_name = grammar.variables[var] + std::string ("_prime");
			int new_index = grammar.index;
			grammar.variables[grammar.index++] = new_prod_name;

			// find the 'e' variable and add a new production for var_prime with e as its rule
			int e_index = -1;
			for (auto [key, value] : grammar.variables)
			{
				if (value == "e") e_index = key;
			}

			Rule r;
			r.push_back (Token (false, e_index));
			grammar.productions.push_back (Production (new_index, r));

			std::vector<Production> new_prods;
			for (auto &prod : prods)
			{
				// For productions starting with var, make a new prod for var_prime
				if (prod.rule.size () > 0

				    && prod.rule[0].index == var)
				{
					Rule r = prod.rule;
					r.push_back (Token (false, new_index));
					r.erase (std::begin (r));
					new_prods.push_back (Production (new_index, r));
				}
				else // otherwise just append var_prime onto the original prod's rule
				{
					Rule r = prod.rule;
					r.push_back (Token (false, new_index));
					new_prods.push_back (Production (prod.var, r));
				}
			}

			grammar.productions.insert (
			std::end (grammar.productions), std::begin (new_prods), std::end (new_prods));
			//}
		}
	}
}

Grammar RemoveLeftRecursion (Grammar &eLess_Grammar)
{
	Grammar res_grammar;
	res_grammar.terminals = eLess_Grammar.terminals;
	res_grammar.variables = eLess_Grammar.variables;
	res_grammar.productions = eLess_Grammar.productions;
	res_grammar.index = eLess_Grammar.index;

	// get a map of all variables with their relative priority (the order they appear in a left side
	// variable)
	std::unordered_map<int, int> var_index_priority;
	int priority = 0;
	for (auto &prod : res_grammar.productions)
	{
		// std::find (std::begin (var_index_priority), std::end (var_index_priority), prod.var);
		auto it = var_index_priority.find (prod.var);
		if (it != std::end (var_index_priority)) {}
		else
		{
			var_index_priority[prod.var] = priority++;
		}
	}

	// remove deep left recursion
	for (int priority = 0; priority < var_index_priority.size (); priority++)
	{
		int index = -1;
		for (auto [cur_key, cur_value] : var_index_priority)
		{
			if (priority == cur_value) index = cur_key;
		}
		fmt::print ("On iteration {}\n", priority);

		// find prods with key as their starting variable;
		std::vector<Production> var_productions;
		for (auto &prod : res_grammar.productions)
		{
			if (prod.var == index) var_productions.push_back (prod);
		}

		// for each rule with Ai→αi
		for (auto &prod : var_productions)
		{
			// if αi begins with nonterminal Aj and j<i
			if (prod.rule.size () > 0 && !prod.rule[0].isTerm &&
			    var_index_priority[prod.rule[0].index] < var_index_priority[index])
			{
				// Let βi be αi without its leading Aj.
				Variable Aj = prod.rule[0].index;
				Rule r = prod.rule;
				r.erase (std::begin (r));

				// Remove the rule Ai→αi.
				auto it =
				std::find (std::begin (res_grammar.productions), std::end (res_grammar.productions), prod);
				if (it != std::end (res_grammar.productions)) res_grammar.productions.erase (it);

				// Find Aj productions
				std::vector<Production> Aj_prods;
				for (auto &prod : res_grammar.productions)
				{
					if (prod.var == Aj) Aj_prods.push_back (prod);
				}

				// For each rule Aj→αj:
				for (auto &j_prod : Aj_prods)
				{
					// Add the rule Ai→αjβi .
					if (j_prod.rule.size () > 0)
					{ //&& prod.rule[0].index == j_prod.var) {
						Rule new_r = j_prod.rule;
						new_r.insert (std::end (new_r), std::begin (r), std::end (r));
						res_grammar.productions.push_back (Production (prod.var, new_r));
					}
				}
			}
		}
		fmt::print ("Removing Left Recusion of {}\n", priority);
		RemoveImmediateLeftRecursion (index, res_grammar);
	}
	return res_grammar;

	/*
	For each nonterminal Ai :
	    Repeat until an iteration leaves the grammar unchanged:
	        For each rule Ai→αi, αi  being a sequence of terminals and nonterminals:
	            If αi begins with a nonterminal Aj and j<i:
	                Let βi be αi without its leading Aj.
	                Remove the rule Ai→αi.
	                For each rule Aj→αj:
	                    Add the rule Ai→αjβi .
	    Remove direct left recursion for Ai as described above
	*/
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
		else if (line.size () > 0 && !isReadingProductionList)
		{
			isReadingProductionList = true;
			std::stringstream s;
			s << line;
			s >> var_name;
			if (var_name.size () > 0)
			{
				bool isNew = true;
				for (auto [key, val] : grammar.variables)
				{
					if (var_name == val)
					{
						var_index = key;
						isNew = false;
					}
				}
				if (isNew)
				{
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
				for (auto [key, val] : grammar.terminals)
				{
					if (str == val)
					{
						rule.push_back (Token (true, key));
						isTerm = true;
					}
				}
				if (isTerm == false)
				{
					bool isNew = true;
					for (auto [key, val] : grammar.variables)
					{
						if (str == val)
						{
							rule.push_back (Token (false, key));
							isNew = false;
						}
					}
					if (isNew)
					{
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

	int e_index = -1;
	for (auto [key, value] : grammar.variables)
	{
		if (value == "e") e_index = key;
	}
	if (e_index == -1) { grammar.variables[grammar.index++] = "e"; }

	grammar.index = index;
	return grammar;
}

void MassageGrammar (std::string grammar_fileName)
{
	std::ifstream in2 ("simple.txt", std::ios::in);
	if (!in2.is_open ()) { fmt::print ("failed to open {}\n", "grammars/simple.txt"); }
	else
	{
		auto in_gram = ReadGrammar (in2);
		auto e_less = RemoveEProds (in_gram);
		auto ll_less_Grammar = RemoveLeftRecursion (e_less);
		OutputFileHandle ofh_lr_less ("left_recursive_less_simple.txt");
		PrintGrammar (ll_less_Grammar, ofh_lr_less);
	}

	std::ifstream in (grammar_fileName, std::ios::in);
	if (!in.is_open ()) { fmt::print ("failed to open {}\n", grammar_fileName); }
	else
	{
		auto grammar = ReadGrammar (in);
		OutputFileHandle ofh_orig ("massaged.txt");
		PrintGrammar (grammar, ofh_orig);

		auto eLess_Grammar = RemoveEProds (grammar);
		OutputFileHandle ofh_e_less ("e_less.txt");
		PrintGrammar (eLess_Grammar, ofh_e_less);

		auto ll_less_Grammar = RemoveLeftRecursion (eLess_Grammar);
		OutputFileHandle ofh_lr_less ("left_recursive_less_full.txt");
		PrintGrammar (ll_less_Grammar, ofh_lr_less);
	}
}

int main ()
{
	MassageGrammar ("grammars/grammar_modified.txt");
	return 0;
}