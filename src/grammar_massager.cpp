#include "grammar_massager.h"

void Grammar::AddProduction (Production p)
{
	auto it = std::find (std::begin (productions), std::end (productions), p);
	// if not found, add it
	if (it == std::end (productions)) { productions.push_back (p); }
}
void Grammar::EraseProduction (Production &p)
{
	auto it = std::find (std::begin (productions), std::end (productions), p);
	if (it != std::end (productions)) productions.erase (it);
}

int Grammar::DeriveNewVariable (Variable var, std::string str)
{
	// check if the variable name already exists, return it if it does
	std::string new_prod_name = variables.at (var) + str;
	for (auto [key, value] : variables)
	{
		if (value == new_prod_name) return key;
	}
	// if it doesn't exist, make a new one
	int new_index = index;
	variables[index++] = new_prod_name;

	return new_index;
}

bool Grammar::isEProd (Rule &rule) const
{
	for (auto &token : rule)
	{
		if (token.isTerm && terminals.at (token.index).size () == 1 && terminals.at (token.index) == "e")
		{ return true; }
		else if (!token.isTerm && variables.at (token.index).size () == 1 && variables.at (token.index) == "e")
		{
			return true;
		}
	}
	return false;
}

int Grammar::find_epsilon_index () const
{
	int e_index = -1;
	for (auto [key, value] : variables)
	{
		if (value == "e") e_index = key;
	}
	// if (e_index == -1) { variables[index++] = "e"; }
	return e_index;
}

void Grammar::ReorderProductionsByVariable ()
{
	auto old_prods = productions;
	productions.clear ();
	for (auto [key, value] : variables)
	{
		for (auto &prod : old_prods)
		{
			if (prod.var == key) productions.push_back (prod);
		}
	}
}

std::vector<Terminal> Grammar::find_firsts_of_production (Production &prod)
{
	std::vector<Terminal> possible_prods;
	int e_index = find_epsilon_index ();
	for (auto &token : prod.rule)
	{

		bool prod_has_epsilon = false;

		// find if the var has an e prod and find all prods that start with var
		std::vector<Production> token_prods;
		for (auto &possible : productions)
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
				if (true) // tok_prod.rule[0].index < prod.var)
				{
					auto vec = find_firsts_of_production (tok_prod);
					possible_prods.insert (std::end (possible_prods), std::begin (vec), std::end (vec));
				}
			}
		}

		if (!prod_has_epsilon) { break; }
	}
	return possible_prods;
}

bool Grammar::ProductionExists (Production &p) const
{
	auto it = std::find (std::begin (productions), std::end (productions), p);
	if (it != std::end (productions)) { return true; }
	return false;
}

std::vector<Production> Grammar::ProductionsOfVariable (Variable var) const
{
	std::vector<Production> prods;
	for (auto &prod : productions)
	{
		if (prod.var == var) prods.push_back (prod);
	}
	return prods;
}

FirstsAndFollows::FirstsAndFollows (Grammar &grammar)
{
	FindFirsts ();
	FindFollows ();
}
/*
1. If X is terminal, then FIRST(X) is {X}.
2. If X -> e is a production, then add e to FIRST(X).
3. If X is nonterminal and X -> Y1 Y2 ... Yk is a production, then place a in
FIRST(X) if for some i, a is in FIRST(Yi), and e is in all of
FIRST(Y1), FIRST(Y2), ..., FIRST(Yi-1); that is, y1, ... yi =*> e. If e is in
FIRST(Yj) for all j = 1, 2, ..., k, then add e to FIRST(X). For
example, everything in FIRSTS(Y) is surely in FIRST(X). If Y1 does not
derive e, then we add nothing more to FIRST(X), but if Y1 =*> e, then we
add FIRST(Y2) and so on.
*/


void FindFirsts ()
{
	int epsilon = grammar.find_epsilon_index ();
	std::set<int> e_vars;

	for (auto &prod : grammar.productions)
	{
		if (grammar.isEProd (prod.rule))
		{
			firsts.at (prod.var).push_back (Token (false, epsilon));
			e_vars.insert (prod.var);
		}

		if (prod.rule.size () > 0 && prod.rule.at (0).isTerm)
		{ first.at (prod.var).push_back (prod.rule.at (0)); } }

	for (auto &prod : grammar.productions)
	{
		bool FailedEarly = false;
		for (auto &token : prod.rule)
		{
			if (e_vars.count (token.index) == 1)
			{
				first.at (prod.var).insert (std::end (first.at (prod.var)),
				std::begin (first.at (token.index)),
				std::end (first.at (token.index)));
			}
			else
			{
				FailedEarly = true;
				break; // stop looping
			}
		}
		if (!FailedEarly) { first.at (prod.var).push_back (Token (false, epsilon)); }
	}
}

/*
1. Place $ in FOLLOW(S), where S is the start symbol and $ is the input
right endmarker.
2. If there is a production A —> aBB', then everything in FIRST(B') except for
e is placed in FOLLOW(B).
3. If there is a production A —> aB, or a production A -> aBB' where
FIRST(B') contains e (i.e., B =*> e), then everything in FOLLOW(A)) is in FOLLOW(B).
*/

void FindFollows ()
{

	for (auto &prod : grammar.productions)
	{
		for (auto &[key, value] : firsts)
		{
			for (auto &tok : value)
			{
				if (value) }
		}
	}
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

/*
1. For each production A -> a of the grammar, do steps 2 and 3.
2. For each terminal a in FIRST(a), add A -> a to M[A, a].
3. If e is in FIRST(a), add A -> a to M[A,b] for each terminal b in
FOLLOW(A). If e is in FIRST(a) and $ is in FOLLOW(A), add A -> a to [A, $].
4. Make each undefined entry of M be error.
*/

ParseTable::ParseTable (Grammar &grammar) : grammar (grammar)
{
	table = std::vector<std::vector<std::set<int>>> (
	grammar.variables.size (), std::vector<std::set<int>> (grammar.terminals.size ()));
	// grammar.ReorderProductionsByVariable ();
	int e_index = grammar.find_epsilon_index ();

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
	// 	follows.at (grammar.start_symbol).push_back (-1); // terminal symbol is -1 possibly?

	// for (auto [key, value] : grammar.variables)
	// {
	// 	FindFollow (key, var_productions, e_prods, follows);
	// }

	index = 0;
	for (auto &prod : grammar.productions)
	{
		if (prod.rule.at (0).isTerm)
		{

			int first_token_index = prod.rule.at (0).index;
			int var_key = var_key_to_index.at (prod.var);

			table.at (var_key).at (first_token_index).insert (index);
		}
		else
		{

			auto vec_indices = grammar.find_firsts_of_production (prod);
			for (auto &ind : vec_indices)
			{
				int token_index = ind;
				int var_key = var_key_to_index.at (prod.var);
				table.at (var_key).at (token_index).insert (index);
			}
		}
		index++;
	}
}


void Grammar::PrintGrammar (std::string out_file_name)
{

	OutputFileHandle ofh (out_file_name);


	fmt::print (ofh.FP (), "TOKENS ");
	for (auto &[key, term] : terminals)
	{
		fmt::print (ofh.FP (), "{} ", term);
	}
	fmt::print (ofh.FP (), "\n");


	for (auto &prod : productions)
	{
		fmt::print (ofh.FP (), "{} ->\n\t", variables.at (prod.var));
		for (auto &token : prod.rule)
		{
			if (token.isTerm)
				fmt::print (ofh.FP (), "{} ", terminals.at (token.index));
			else
				fmt::print (ofh.FP (), "{} ", variables.at (token.index));
		}
		fmt::print (ofh.FP (), "\n\n");
	}
}

void ParseTable::PrintParseTable (std::string out_file_name)
{
	OutputFileHandle ofh (out_file_name);

	fmt::print (ofh.FP (), "TOKENS, ");
	for (auto [key, value] : grammar.terminals)
	{
		fmt::print (ofh.FP (), "\'{}\', ", value);
	}
	fmt::print (ofh.FP (), "\n");


	int i = 0;
	for (auto [key, value] : grammar.variables)
	{
		if (value != "e")
		{ // don't print a row for the epsilon symbol
			fmt::print (ofh.FP (), "{}, ", value);

			for (auto &terms : table.at (i))
			{
				for (auto &index : terms)
				{
					fmt::print (
					ofh.FP (), "{} -> ", grammar.variables.at (grammar.productions.at (index).var));
					for (auto &token : grammar.productions.at (index).rule)
					{
						if (token.isTerm)
						{
							if (grammar.terminals.at (token.index) == ",")
								fmt::print (ofh.FP (), "\'comma\' ");
							else
								fmt::print (ofh.FP (), "\'{}\' ", grammar.terminals[token.index]);
						}
						else
							fmt::print (ofh.FP (), "{} ", grammar.variables.at (token.index));
					}
					if (terms.size () > 1) fmt::print (ofh.FP (), "| ");
				}
				fmt::print (ofh.FP (), ", ");
			}
			fmt::print (ofh.FP (), "\n");
		}
		i++;
	}
}

void Grammar::RemoveDuplicateProductions ()
{
	// copy old ones, and reinsert them, while checking if they are unique.
	std::vector<Production> old_prods = productions;
	productions.clear ();
	for (auto &prod : old_prods)
	{
		AddProduction (prod);
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
	if (found_e_vars.at (iteration))
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
		if (!prod.rule.at (i).isTerm)
		{
			auto it = std::find (std::begin (e_vars), std::end (e_vars), prod.rule.at (i).index);
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
	Grammar res_grammar = grammar;

	std::unordered_set<Variable> e_vars;
	std::vector<Production> e_prods;

	// finds direct e prods
	for (auto &prod : res_grammar.productions)
	{
		if (res_grammar.isEProd (prod.rule))
		{
			e_vars.insert (prod.var);
			e_prods.push_back (prod);
		}
	}

	std::vector<Production> contains_eProds;
	for (auto &prod : res_grammar.productions)
	{
		bool contains_e = false;
		for (auto &tok : prod.rule)
		{
			if (e_vars.count (tok.index)) contains_e = true;
		}
		if (contains_e) { contains_eProds.push_back (prod); }
	}


	for (auto &prod : contains_eProds)
	{
		auto new_prods = PermuteProductionOnERemoval (prod, e_vars);
		if (new_prods.size () > 0)
			res_grammar.productions.insert (
			std::end (res_grammar.productions), std::begin (new_prods), std::end (new_prods));
	}
	for (auto &prod : e_prods)
	{
		res_grammar.EraseProduction (prod);
	}

	res_grammar.RemoveDuplicateProductions ();
	return res_grammar;
}

Grammar BetterRemoveEpsilonProductions (Grammar &in_grammar)
{
	auto res_grammar = in_grammar;

	for (auto [key, value] : res_grammar.variables)
	{
		auto var_prods = res_grammar.ProductionsOfVariable (key);

		// determine if A has an e production rule.
		bool isE = false;
		for (auto p : var_prods)
		{
			if (res_grammar.isEProd (p.rule)) isE = true;
		}

		if (isE) {}
	}


	return res_grammar;
}

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
		int inner_priority = 0;
		for (auto &prod : res_grammar.productions)
		{
			// std::find (std::begin (var_index_priority), std::end (var_index_priority), prod.var);
			auto it = var_index_priority.find (prod.var);
			if (it != std::end (var_index_priority)) {}
			else
			{
				var_index_priority[prod.var] = inner_priority++;
			}
		}

		int index = -1;
		for (auto [cur_key, cur_value] : var_index_priority)
		{
			if (priority == cur_value) index = cur_key;
		}
		if (index == -1)
		{
			fmt::print ("OH NO");
			fmt::print ("OH NO");
		}
		fmt::print ("On iteration {}\n", priority);

		bool keepSorting = true;
		while (keepSorting)
		{
			keepSorting = false;


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

							res_grammar.AddProduction (Production (prod.var, new_r));
							keepSorting = true;
						}
					}
				}
			}
		}
		fmt::print ("Removing Left Recusion of {}\n", priority);

		////////////////////// Remove Immediate left recursion //////////////////////////

		bool hasILR = false;
		for (auto &prod : res_grammar.productions)
		{
			if (prod.var == index && prod.rule.size () > 0 && !prod.rule[0].isTerm && prod.rule[0].index == index)
			{ hasILR = true; } }

		// only remove it if it has it
		if (!hasILR) { continue; }


		// find all productions with this variable as its left side
		std::vector<Production> prods;
		for (auto &prod : res_grammar.productions)
		{
			if (prod.var == index && prod.rule.size () > 0) //&& !(*it).rule[0].isTerm && (*it).rule[0].index == var)
			{ prods.push_back (prod); } }

		for (auto &prod : prods)
		{
			auto it =
			std::find (std::begin (res_grammar.productions), std::end (res_grammar.productions), prod);
			if (it != std::end (res_grammar.productions)) { res_grammar.productions.erase (it); }
		}

		fmt::print ("Immediately recursive {}\n", prods.size ());

		if (prods.size () > 0)
		{

			// create a new variable

			int new_index = res_grammar.DeriveNewVariable (index, "_prime");

			// find the 'e' variable and add a new production for var_prime with e as its rule
			int e_index = res_grammar.find_epsilon_index ();

			Rule r;
			r.push_back (Token (false, e_index));
			res_grammar.AddProduction (Production (new_index, r));

			std::vector<Production> new_prods;
			for (auto &prod : prods)
			{
				// For productions starting with var, make a new prod for var_prime
				if (prod.rule.size () > 0

				    && prod.rule[0].index == index)
				{
					Rule r = prod.rule;
					r.push_back (Token (false, new_index));
					r.erase (std::begin (r));
					res_grammar.AddProduction (Production (new_index, r));
				}
				else // otherwise just append var_prime onto the original prod's rule
				{
					Rule r = prod.rule;
					r.push_back (Token (false, new_index));
					// new_prods.push_back (Production (prod.var, r));
					res_grammar.AddProduction (Production (prod.var, r));
				}
			}

			res_grammar.productions.insert (
			std::end (res_grammar.productions), std::begin (new_prods), std::end (new_prods));
			//}
		}
	}
	return res_grammar;
}

Grammar RemoveXLeftFactoring (Grammar &in_grammar)
{
	Grammar res_grammar = in_grammar;

	bool has_changed = true;
	while (has_changed)
	{
		has_changed = false;
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

				has_changed = true;
				// make new variable
				int new_index = res_grammar.DeriveNewVariable (prods.at (0).var, "_factoring");


				// remove original production
				for (auto &prod : prods)
				{
					auto it = std::find (
					std::begin (res_grammar.productions), std::end (res_grammar.productions), prod);
					if (it != std::end (res_grammar.productions))
						res_grammar.productions.erase (it);
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
				res_grammar.AddProduction (Production (prods[0].var, rule));


				// Add new productions with
				for (auto &prod : prods)
				{
					Rule new_r;
					new_r.insert (
					std::end (new_r), std::begin (prod.rule) + num_repeated, std::end (prod.rule));
					if (new_r.size () == 0)
					{
						new_r.push_back (Token (false, res_grammar.find_epsilon_index ())); // insert epsilon
					}
					res_grammar.AddProduction (Production (new_index, new_r));
				}
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

	grammar.find_epsilon_index ();

	return grammar;
}

void MassageGrammar (std::string grammar_fileName, std::string out_name = std::string ("grammar_"))
{
	std::ifstream in (grammar_fileName, std::ios::in);
	if (!in.is_open ()) { fmt::print ("failed to open {}\n", grammar_fileName); }
	else
	{
		using namespace std::string_literals;

		auto grammar = ReadGrammar (in);
		grammar.PrintGrammar (out_name + "_original.txt"s);

		auto eLess_Grammar = RemoveEProds (grammar);
		eLess_Grammar.PrintGrammar (out_name + "_epsilon.txt"s);

		auto ll_less_Grammar = RemoveLeftRecursion (eLess_Grammar);
		ll_less_Grammar.PrintGrammar (out_name + "_left_recursion.txt"s);

		auto factored_Grammar = RemoveXLeftFactoring (ll_less_Grammar);
		factored_Grammar.PrintGrammar (out_name + "_factored.txt"s);

		auto parse_table = ParseTable (factored_Grammar);
		parse_table.PrintParseTable (out_name + "_parse_table.txt"s);
	}
}

int main ()
{
	MassageGrammar ("grammars/simple.txt", "simple");
	MassageGrammar ("grammars/grammar_modified.txt", "pascal");
	return 0;
}