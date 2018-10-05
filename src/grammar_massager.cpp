
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include <map>
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

	bool operator== (const Production &rhs) const { return rhs.var == var && rhs.rule == rule; }
};

struct Grammar
{
	std::map<int, std::string> terminals;
	std::map<int, std::string> variables;
	std::vector<Production> productions;
	int start_symbol = 0; // should generally be the first production...
	int index = 0;
};

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

int find_epsilon_index (Grammar &grammar)
{
	int e_index = -1;
	for (auto [key, value] : grammar.variables)
	{
		if (value == "e") e_index = key;
	}
	if (e_index == -1) { grammar.variables[grammar.index++] = "e"; }
	return e_index;
}

std::vector<Terminal> find_firsts_of_production (Grammar &grammar, Production &prod)
{


	std::vector<Terminal> possible_prods;
	int e_index = find_epsilon_index (grammar);
	for (auto &token : prod.rule)
	{

		bool prod_has_epsilon = false;

		// find if the var has an e prod and also find all prods that start with var
		std::vector<Production> token_prods;
		for (auto &possible : grammar.productions)
		{
			if (token.index == possible.var)
			{
				if (possible.rule.at (0).index == e_index) prod_has_epsilon = true;
				token_prods.push_back (possible);
			}
		}

		for (auto &tok_prod : token_prods)
		{
			//
			if (tok_prod.rule.at (0).isTerm)
			{ possible_prods.push_back (tok_prod.rule.at (0).index); } else
			{
				auto vec = find_firsts_of_production (grammar, tok_prod);
				possible_prods.insert (std::end (possible_prods), std::begin (vec), std::end (vec));
			}
		}

		if (prod_has_epsilon) {}
		else
		{
			break;
		}
	}
	return possible_prods;
}

bool FindFirst (Variable key,
std::map<int, std::vector<Production>> &var_productions,
int e_index,
std::map<int, bool> &e_prods,
std::map<int, std::vector<int>> &firsts)
{
	bool isChanged = false;
	if (e_prods.at (key) == true)
	{
		auto it = std::find (std::begin (firsts.at (key)), std::end (firsts.at (key)), e_index);
		if (it != std::end (firsts.at (key)))
		{
			isChanged = true;
			firsts.at (key).push_back (e_index);
		}
	}

	for (auto &prod : var_productions.at (key))
	{
		for (int i = 0; i < prod.rule.size (); i++)
		{
			// if on last rule and its an e prod, we made it to the end and so add e to Var
			if (i == prod.rule.size () - 1 && e_prods.at (prod.rule.at (i).index) == true)
			{
				auto it = std::find (std::begin (firsts.at (key)), std::end (firsts.at (key)), e_index);
				if (it != std::end (firsts.at (key)))
				{
					isChanged = true;
					firsts.at (key).push_back (e_index);
				}
			}
			if (prod.rule.at (i).isTerm)
			{
				auto it = std::find (
				std::begin (firsts.at (key)), std::end (firsts.at (key)), prod.rule.at (i).index);
				if (it != std::end (firsts.at (key)))
				{
					isChanged = true;
					firsts.at (key).push_back (e_index);
				}
				break;
			}
			else
			{
				if (firsts.at (prod.rule.at (i).index).size () > 0)
				{
					// make sure something happened
					isChanged = true;
					firsts.at (key).insert (std::end (firsts.at (key)),
					std::begin (firsts.at (prod.rule.at (i).index)),
					std::end (firsts.at (prod.rule.at (i).index)));
				}


				if (e_prods.at (prod.rule.at (i).index) == true) break;
			}
		}
	}
	return isChanged;
}
void FindFollow (Variable key,
std::map<int, std::vector<Production>> &var_productions,
std::map<int, bool> &e_prods,
std::map<int, std::vector<int>> &follows)
{
}

struct ParseTable
{
	Grammar grammar;
	std::map<int, int> var_key_to_index;

	// row-var, col-terminal, inner is for possible multiple entries
	std::vector<std::vector<std::vector<int>>> table;

	ParseTable (Grammar &grammar) : grammar (grammar)
	{
		table = std::vector<std::vector<std::vector<int>>> (
		grammar.variables.size (), std::vector<std::vector<int>> (grammar.terminals.size ()));

		int e_index = find_epsilon_index (grammar);

		// map var keys to 0 based index. not sure if necessary if I just make everything maps...
		int index = 0;
		for (auto &[key, value] : grammar.variables)

			var_key_to_index[key] = index++;

		// pre-bucket all the productions per key. Just removes needless searching in first
		// calculation which could be heavily recursive...
		std::map<int, std::vector<Production>> var_productions;
		for (auto &prod : grammar.productions)
		{
			if (prod.var != e_index) // stupid special cases
				var_productions[prod.var].push_back (prod);
		}

		std::map<int, std::vector<int>> firsts;
		std::map<int, std::vector<int>> follows;



		// find symbols with e prods - first need to fill out map with false values
		std::map<int, bool> e_prods;
		for (auto [key, value] : grammar.variables)
		{
			if (key == e_index)
			{
				e_prods[key] = true; // special case gone wrong....
			}
			else
			{

				firsts[key] = std::vector<int> (); // should be enough to initialize them right?
				follows[key] = std::vector<int> ();
				e_prods[key] = false;
			}
		}
		for (auto [key, value] : grammar.terminals)
		{
			e_prods[key] = false;
		}

		// find all epsilon production variables
		for (auto &prod : grammar.productions)
		{
			if (prod.rule.size () > 0 && prod.rule.at (0).index == e_index)
			{ e_prods[prod.var] = true; } }


		// find firsts
		bool isChanged = true;
		// while (isChanged)
		// {
		// 	isChanged = false;
		// 	for (auto [key, value] : grammar.variables)
		// 	{
		// 		if (key != e_index)
		// 		{
		// 			bool ret = FindFirst (key, var_productions, e_index, e_prods, firsts);
		// 			if (ret == true) { isChanged = true; }
		// 		}
		// 	}
		// }


		// find follows
		follows.at (grammar.start_symbol).push_back (-1); // terminal symbol is -1 possibly?

		for (auto [key, value] : grammar.variables)
		{
			FindFollow (key, var_productions, e_prods, follows);
		}




		index = 0;
		for (auto &prod : grammar.productions)
		{
			if (prod.rule.at (0).isTerm)
			{

				int first_token_index = prod.rule.at (0).index;
				int var_key = var_key_to_index.at (prod.var);

				table.at (var_key).at (first_token_index).push_back (index);
			}
			else
			{
				auto vec_indices = find_firsts_of_production (grammar, prod);
				for (auto &ind : vec_indices)
				{
					int token_index = ind;
					int var_key = var_key_to_index.at (prod.var);
					table.at (var_key).at (token_index).push_back (index);
				}

				// int e_index = find_epsilon_index (grammar);
				// for (auto &token : prod.rule)
				// {

				// 	bool prod_has_epsilon = false;

				// 	std::vector<Production> token_prods;
				// 	for (auto &possible : grammar.productions)
				// 	{
				// 		if (token.index == possible.var)
				// 		{
				// 			if (possible.rule.at (0).index == e_index) prod_has_epsilon = true;
				// 			token_prods.push_back (possible);
				// 		}
				// 	}

				// 	for (auto &tok_prod : token_prods)
				// 	{
				// 		if (tok_prod.rule.at (0).isTerm)
				// 		{
				// 			int token_index = tok_prod.rule.at (0).index;
				// 			int var_key = var_key_to_index.at (prod.var);
				// 			table.at (var_key).at (token_index).push_back (index);
				// 		}
				// 		else
				// 		{
				// 			// find sub vars start values and add it to this
				// 		}
				// 	}

				// 	if (prod_has_epsilon) {}
				// 	else
				// 	{
				// 		break;
				// 	}
			}
			index++;

			// auto firsts = find_firsts_of_production(grammar, prod);

			// int first_token_index = prod.rule.at(0).index;
			// int var_key = var_key_to_index.at(prod.var);


			// table.at(var_key).at(first_token_index).insert(std::end(table.at(var_key).at(first_token_index)), std::begin(firsts), std::end(firsts));
		}
		// TODO logic for following variables
	}
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

void PrintParseTable (ParseTable &parse, OutputFileHandle &ofh)
{
	fmt::print (ofh.FP (), "TOKENS, ");
	for (auto [key, value] : parse.grammar.terminals)
	{
		fmt::print (ofh.FP (), "\'{}\', ", value);
	}
	fmt::print (ofh.FP (), "\n");


	int i = 0;
	for (auto [key, value] : parse.grammar.variables)
	{
		fmt::print (ofh.FP (), "{}, ", value);
		for (auto &terms : parse.table.at (i))
		{
			for (auto &index : terms)
			{
				fmt::print (
				ofh.FP (), "{} -> ", parse.grammar.variables.at (parse.grammar.productions[index].var));
				for (auto &token : parse.grammar.productions[index].rule)
				{
					if (token.isTerm)
					{
						if (parse.grammar.terminals.at (token.index) == ",")
							fmt::print (ofh.FP (), "\'comma\' ");
						else
							fmt::print (ofh.FP (), "\'{}\' ", parse.grammar.terminals[token.index]);
					}
					else
						fmt::print (ofh.FP (), "{} ", parse.grammar.variables[token.index]);
				}
				if (terms.size () > 1) fmt::print (ofh.FP (), "| ");
			}
			fmt::print (ofh.FP (), ", ");
		}
		fmt::print (ofh.FP (), "\n");
		i++;
	}
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


	Grammar res_grammar = grammar;


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
		for (auto &prod : grammar.productions)
		{
			if (prod.var == var && prod.rule.size () > 0) //&& !(*it).rule[0].isTerm && (*it).rule[0].index == var)
			{ prods.push_back (prod); } }

		for (auto &prod : prods)
		{
			auto it = std::find (std::begin (grammar.productions), std::end (grammar.productions), prod);
			if (it != std::end (grammar.productions)) grammar.productions.erase (it);
		}

		fmt::print ("Immediately recursive {}\n", prods.size ());

		if (prods.size () > 0)
		{

			// create a new variable
			std::string new_prod_name = grammar.variables[var] + std::string ("_prime");
			int new_index = grammar.index;
			grammar.variables[grammar.index++] = new_prod_name;

			// find the 'e' variable and add a new production for var_prime with e as its rule
			int e_index = find_epsilon_index (grammar);

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
	Grammar res_grammar = eLess_Grammar;

	// get a map of all variables with their relative priority (the order they appear in a left
	// side variable)
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
		bool keepSorting = true;
		while (keepSorting)
		{
			bool hasChanged = false;

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
					auto it = std::find (
					std::begin (res_grammar.productions), std::end (res_grammar.productions), prod);
					if (it != std::end (res_grammar.productions))
						res_grammar.productions.erase (it);

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
							hasChanged = true;
						}
					}
				}
			}
			if (hasChanged)
				keepSorting = true;
			else
				keepSorting = false;
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

Grammar RemoveXLeftFactoring (Grammar &in_grammar)
{
	Grammar res_grammar = in_grammar;

	std::map<int, std::vector<Production>> num_variable_occurances;
	for (auto &prod : res_grammar.productions)
	{
		int hash = prod.var * 10000 + prod.rule[0].index; // shouldn't be any collusions for small grammar sizes...
		num_variable_occurances[hash].push_back (prod);
	}
	for (auto &[key, prods] : num_variable_occurances)
	{
		if (prods.size () > 1)
		{
			// make new variable
			std::string new_prod_name =
			res_grammar.variables.at (prods[0].var) + std::string ("_factored");
			int new_index = res_grammar.index;
			res_grammar.variables[res_grammar.index++] = new_prod_name;

			// remove original production
			for (auto &prod : prods)
			{
				auto it =
				std::find (std::begin (res_grammar.productions), std::end (res_grammar.productions), prod);
				if (it != std::end (res_grammar.productions)) res_grammar.productions.erase (it);
			}

			// count number of repeated characters are at the front
			int num_repeated = 0;
			for (auto &token : prods[0].rule)
			{
				bool all_the_same = true;
				for (auto &prod : prods)
				{
					if (prod.rule.size () <= num_repeated || !(prod.rule[num_repeated] == token))
						all_the_same = false;
				}
				if (all_the_same)
					num_repeated++;
				else
					break;
			}

			// add replacement production with only the first token from rule
			Rule rule;
			rule.insert (std::end (rule), std::begin (prods[0].rule), std::begin (prods[0].rule) + num_repeated);
			rule.push_back (Token (false, new_index));
			res_grammar.productions.push_back (Production (prods[0].var, rule));


			// Add new productions with
			for (auto &prod : prods)
			{
				Rule new_r;
				new_r.insert (
				std::end (new_r), std::begin (prod.rule) + num_repeated, std::end (prod.rule));
				if (new_r.size () == 0)
				{
					new_r.push_back (Token (false, find_epsilon_index (res_grammar))); // insert epsilon
				}
				res_grammar.productions.push_back (Production (new_index, new_r));
			}
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

	bool hasFoundFirstVariable = false; // start symbol?
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
				if (str.length () > 0) grammar.terminals[grammar.index++] = str;
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
					if (hasFoundFirstVariable == false)
					{
						hasFoundFirstVariable = true;
						grammar.start_symbol = grammar.index;
					}
					var_index = grammar.index;
					grammar.variables[grammar.index++] = var_name;
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
						grammar.variables[grammar.index] = str;

						rule.push_back (Token (false, grammar.index));
						grammar.index++;
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

	find_epsilon_index (grammar);

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

		auto factored_Grammar = RemoveXLeftFactoring (ll_less_Grammar);
		OutputFileHandle ofh_factoring ("factored_full.txt");
		PrintGrammar (factored_Grammar, ofh_factoring);

		auto parse_table = ParseTable (factored_Grammar);
		OutputFileHandle ofh_parse_table ("parse_table.txt");
		PrintParseTable (parse_table, ofh_parse_table);
	}
}

int main ()
{
	MassageGrammar ("grammars/grammar_modified.txt");
	return 0;
}