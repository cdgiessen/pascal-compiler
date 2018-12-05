#include "parser.h"

uint32_t RT_err = 0;
uint32_t RT_none = 1;
uint32_t RT_bool = 2;
uint32_t RT_int = 3;
uint32_t RT_real = 4;
uint32_t RT_arr_int = 5;
uint32_t RT_arr_real = 6;

bool IsArrInt (RetType rt) { return (rt.data & 255) == RT_arr_int; }

bool IsArrReal (RetType rt) { return (rt.data & 255) == RT_arr_real; }

bool HasSymbol (TokenInfo t) { return std::holds_alternative<SymbolType> (t.attrib); }
int GetSymbol (TokenInfo t) { return std::get<SymbolType> (t.attrib).loc; }
int GetNumValInt (TokenInfo t) { return std::get<int> (std::get<NumType> (t.attrib).val); }
float GetNumValReal (TokenInfo t) { return std::get<float> (std::get<NumType> (t.attrib).val); }


ProcedureID ParseTree::SetStartProcedure (SymbolID name)
{
	procedures.emplace (0, name);
	return procIDCounter++;
}

ProcedureID ParseTree::AddSubProcedure (SymbolID newProcName)
{
	ProcedureID curProc = eye;
	while (curProc != -1)
	{
		for (auto [id, name] : procedures.at (curProc).sub_procs)
		{
			if (name == newProcName)
			{
				// error - not unique
				return -1;
			}
		}
		curProc = procedures.at (curProc).parent;
	}

	procedures.emplace (std::make_pair (procIDCounter, Procedure (newProcName, eye)));
	procedures.at (eye).AddSubProc (newProcName, procIDCounter);
	//	procedures[procIDCounter] = Procedure (newProcName, eye);
	return procIDCounter++;
}

bool ParseTree::CheckProcedure (SymbolID s)
{
	ProcedureID curProc = eye;
	while (curProc != -1)
	{
		for (auto [id, name] : procedures.at (curProc).sub_procs)
		{
			if (id == s) { return true; }
		}
		curProc = procedures.at (curProc).parent;
	}
	return false;
}

std::vector<RetType> ParseTree::SubProcedureType (SymbolID s)
{
	ProcedureID curProc = eye;
	while (curProc != -1)
	{
		for (auto [id, type] : procedures.at (curProc).sub_procs)
		{
			if (id == s) { return procedures.at (curProc).Signature (); }
		}
		curProc = procedures.at (curProc).parent;
	}
	return {};
}


bool ParseTree::AddVariable (SymbolID newName, RetType type, bool isParam)
{
	// TODO: check for procedures with same name

	for (auto [name, type] : procedures.at (eye).params)
	{
		if (newName == name)
		{ // error
			return true;
		}
	}
	for (auto [name, type] : procedures.at (eye).locals)
	{
		if (newName == name)
		{ // error
			return true;
		}
	}
	if (isParam)
	{ procedures.at (eye).params.push_back (std::pair<SymbolID, RetType> (newName, type)); } else
	{
		procedures.at (eye).locals.push_back (std::pair<SymbolID, RetType> (newName, type));
	}
	return false;
}

std::optional<RetType> ParseTree::CheckVariable (SymbolID s)
{
	for (auto [id, type] : procedures.at (eye).params)
	{
		if (id == s) { return type; }
	}
	for (auto [id, type] : procedures.at (eye).locals)
	{
		if (id == s) { return type; }
	}
	return {};
}


void ParseTree::Push (ProcedureID id) { eye = id; }
void ParseTree::Pop () { eye = procedures.at (eye).parent; }

ParserContext::ParserContext (CompilationContext &context, TokenStream &ts, Logger &logger)
: context (context), ts (ts), logger (logger)
{
}

TokenInfo ParserContext::Current () const { return ts.Current (); }
TokenInfo ParserContext::Advance () { return ts.Advance (); }

void ParserContext::LogErrorExpectedGot (std::vector<TT> types)
{
	using namespace std::string_literals;

	std::string out = "SYNERR: Expected "s;
	int count = 0;
	for (auto &t : types)
	{
		out = out + t;
		if (count++ != types.size () - 1) out += ", ";
	}
	out += "; Recieved "s + Current ().type;

	logger.AddSynErrPrint (Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", out); });
}


void ParserContext::LogErrorSem (std::string msg)
{
	logger.AddSemErrPrint (
	Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", "SEMERR: " + msg); });
}

void ParserContext::LogErrorProcedureScope (TokenInfo t)
{
	LogErrorSem ("Procedure \"" + SymbolName (GetSymbol (t)) + "\" not in current scope");
}
void ParserContext::LogErrorUniqueProcedure (TokenInfo t)
{
	LogErrorSem ("Procedure \"" + SymbolName (GetSymbol (t)) + "\" not unique in current scope");
}

void ParserContext::LogErrorIdentifierScope (TokenInfo t)
{
	LogErrorSem ("Identifier \"" + SymbolName (GetSymbol (t)) + "\" not in current scope");
}

void ParserContext::LogErrorUniqueIdentifier (TokenInfo t)
{
	LogErrorSem ("Identifier \"" + SymbolName (GetSymbol (t)) + "\" not unique in current scope");
}


std::string ParserContext::SymbolName (SymbolID loc)
{
	return context.symbolTable.SymbolView (loc);
}


TokenInfo ParserContext::Match (TT tt)
{
	using namespace std::string_literals;

	if (tt == Current ().type && tt != TT::END_FILE)

		return Advance ();

	else if (tt == Current ().type && tt == TT::END_FILE)
	{
		return Current ();
	}
	// throw "End of Parse";

	else //  (tt != Current ().type)
	{
		LogErrorExpectedGot ({ tt });
		return Advance ();
	}
}

void ParserContext::Synch (std::vector<TT> set)
{
	set.push_back (TT::END_FILE);
	TT tt = Current ().type;

	bool found = false;
	for (auto &s : set)
		if (s == tt) found = true;
	while (!found)
	{
		tt = Advance ().type;
		for (auto &s : set)
			if (s == tt) found = true;
	}
}
namespace Parser
{
void Parse (ParserContext &pc)
{
	// try
	//{
	auto v = ProgramStatement (pc);
	pc.Match (TT::END_FILE);
	//}
	// catch (std::exception &err)
	//{
	//	fmt::print (std::string (err.what ()) + "\n");
	//	// found end file token, don't continue parsing
	//}
}

RetType prog_stmt_program (ParserContext &pc)
{
	pc.Match (TT::PROGRAM);

	ProcedureID cur = pc.tree.SetStartProcedure (GetSymbol (pc.Current ()));
	if (cur == -1) { pc.LogErrorUniqueProcedure (pc.Current ()); }
	pc.tree.Push (cur);
	pc.Match (TT::ID);

	pc.Match (TT::PAREN_OPEN);

	IdentifierList (pc);
	pc.Match (TT::PAREN_CLOSE);
	pc.Match (TT::SEMICOLON);
	ProgramStatementFactored (pc);
}

RetType ProgramStatement (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PROGRAM):
			return prog_stmt_program (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::PROGRAM });
			pc.Synch ({ TT::END_FILE });
	}
	return ret;
}
RetType ProgramStatementFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VAR):
			Declarations (pc);
			ProgramStatementFactoredFactored (pc);
			break;
		case (TT::PROCEDURE):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			pc.Match (TT::DOT);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc);
			pc.Match (TT::DOT);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::VAR, TT::PROCEDURE, TT::BEGIN });
			pc.Synch ({ TT::END_FILE });
	}
	return ret;
}
RetType ProgramStatementFactoredFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			pc.Match (TT::DOT);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc);
			pc.Match (TT::DOT);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PROCEDURE, TT::BEGIN });
			pc.Synch ({ TT::END_FILE });
	}
	return ret;
}

RetType ident_list_id (ParserContext &pc)
{
	bool exists = pc.tree.AddVariable (GetSymbol (pc.Current ()), RT_none, true);
	if (exists) { pc.LogErrorUniqueIdentifier (pc.Current ()); }
	pc.Match (TT::ID);

	return IdentifierListPrime (pc);
}

RetType IdentifierList (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			return ident_list_id (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::ID });
			pc.Synch ({ TT::PAREN_CLOSE });
	}
	return ret;
}

RetType ident_list_prime (ParserContext &pc)
{
	pc.Match (TT::COMMA);
	bool exists = pc.tree.AddVariable (GetSymbol (pc.Current ()), RT_none, true);
	if (exists) { pc.LogErrorUniqueIdentifier (pc.Current ()); }
	pc.Match (TT::ID);
	return IdentifierListPrime (pc);
}
RetType IdentifierListPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			return ident_list_prime (pc);
		case (TT::PAREN_CLOSE):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::COMMA, TT::PAREN_CLOSE });
			pc.Synch ({ TT::PAREN_CLOSE });
	}
	return ret;
	// e-prod
}

RetType decls (ParserContext &pc)
{
	pc.Match (TT::VAR);
	auto tid = pc.Current ();
	pc.Match (TT::ID);

	pc.Match (TT::COLON);
	auto t = Type (pc);
	if (HasSymbol (tid))
	{
		auto exists = pc.tree.AddVariable (GetSymbol (tid), t, false);
		if (exists) { pc.LogErrorIdentifierScope (tid); }
	}

	pc.Match (TT::SEMICOLON);
	return DeclarationsPrime (pc);
}

RetType Declarations (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VAR):
			return decls (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::VAR });

			pc.Synch ({ TT::PROCEDURE, TT::BEGIN });
	}
	return ret;
}

RetType decls_prime (ParserContext &pc)
{
	pc.Match (TT::VAR);
	auto tid = pc.Current ();
	pc.Match (TT::ID);
	pc.Match (TT::COLON);
	auto tt = Type (pc);
	if (HasSymbol (tid))
	{
		auto exists = pc.tree.AddVariable (GetSymbol (tid), tt, false);
		if (exists) { pc.LogErrorUniqueIdentifier (tid); }
	}
	pc.Match (TT::SEMICOLON);
	DeclarationsPrime (pc);
}

RetType DeclarationsPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VAR):
			return decls_prime (pc);
		case (TT::PROCEDURE):
		case (TT::BEGIN):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::PROCEDURE, TT::BEGIN, TT::VAR });
			pc.Synch ({ TT::PROCEDURE, TT::BEGIN });
	}
	return ret;
	// e-prod
}

RetType type_array (ParserContext &pc)
{
	RetType ret = RT_none;
	pc.Match (TT::ARRAY);
	pc.Match (TT::BRACKET_OPEN);
	auto ts = pc.Current ();
	if (std::get<NumType> (ts.attrib).val.index () != 0)
	{
		ret = RT_err;
		pc.LogErrorSem ("Array right bound not an int");
	}
	pc.Match (TT::NUM);
	pc.Match (TT::DOT_DOT);

	auto te = pc.Current ();
	if (std::get<NumType> (te.attrib).val.index () != 0)
	{
		ret = RT_err;
		pc.LogErrorSem ("Array left bound not an int");
	}
	pc.Match (TT::NUM);
	pc.Match (TT::BRACKET_CLOSE);
	pc.Match (TT::OF);
	auto t = StandardType (pc);
	if (ret != RT_err)
	{
		if (GetNumValInt (te) - GetNumValInt (ts) <= 0)
		{
			pc.LogErrorSem ("Array bounds must be positive");
			return RT_err;
		}
		else if (t == RT_int)
		{
			return RetType (RT_arr_int, GetNumValInt (te) - GetNumValInt (ts));
		}
		else if (t == RT_real)
		{
			return RetType (RT_arr_real, GetNumValInt (te) - GetNumValInt (ts));
		}
	}
	return RT_err;
}
RetType Type (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ARRAY):
			return type_array (pc);
		case (TT::STANDARD_TYPE):
			return StandardType (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ARRAY, TT::STANDARD_TYPE });
			pc.Synch ({ TT::PAREN_CLOSE, TT::SEMICOLON });
			return RT_err;
	}
	return ret;
}

RetType std_type (ParserContext &pc)
{
	auto t = pc.Match (TT::STANDARD_TYPE);
	switch (std::get<StandardTypeEnum> (t.attrib))
	{
		case (StandardTypeEnum::integer):
			return RT_int;
			break;
		case (StandardTypeEnum::real):
			return RT_real;
			break;
		default:
			pc.LogErrorSem ("Identifier \"" + pc.SymbolName (GetSymbol (pc.Current ()))
			                + "\" not a valid type (integer or real)");
			return RT_err;
	}
}
RetType StandardType (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::STANDARD_TYPE):
			return std_type (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::STANDARD_TYPE });
			pc.Synch ({ TT::PAREN_CLOSE, TT::SEMICOLON });
			return RT_err;
	}
	return ret;
}
RetType SubprogramDeclarations (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramDeclaration (pc);
			pc.Match (TT::SEMICOLON);
			SubprogramDeclarationsPrime (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PROCEDURE });

			pc.Synch ({ TT::BEGIN });
	}
	return ret;
}
RetType SubprogramDeclarationsPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramDeclaration (pc);
			pc.Match (TT::SEMICOLON);
			SubprogramDeclarationsPrime (pc);
			break;
		case (TT::BEGIN):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::PROCEDURE, TT::BEGIN });
			pc.Synch ({ TT::BEGIN });
	}
	return ret;
	// e-prod
}
RetType SubprogramDeclaration (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramHead (pc);
			SubprogramDeclarationFactored (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PROCEDURE });

			pc.Synch ({ TT::SEMICOLON });
	}
	return ret;
}
RetType SubprogramDeclarationFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VAR):
			Declarations (pc);
			SubprogramDeclarationFactoredFactored (pc);
			break;
		case (TT::PROCEDURE):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::VAR, TT::PROCEDURE, TT::BEGIN });
			pc.Synch ({ TT::SEMICOLON });
	}
	return ret;
}
RetType SubprogramDeclarationFactoredFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PROCEDURE, TT::BEGIN });
			pc.Synch ({ TT::SEMICOLON });
	}
	return ret;
}

RetType sub_prog_head_procedure (ParserContext &pc)
{
	pc.Match (TT::PROCEDURE);
	ProcedureID cur = pc.tree.AddSubProcedure (GetSymbol (pc.Current ()));
	if (cur == -1) { pc.LogErrorUniqueProcedure (pc.Current ()); }
	pc.Match (TT::ID);
	pc.tree.Push (cur);

	return SubprogramHeadFactored (pc);
}

RetType SubprogramHead (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			return sub_prog_head_procedure (pc);

		default:
			pc.LogErrorExpectedGot ({ TT::PROCEDURE });
			pc.Synch ({ TT::PROCEDURE, TT::VAR, TT::BEGIN });
	}
	return ret;
}
RetType SubprogramHeadFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PAREN_OPEN):
			Arguments (pc);
			pc.Match (TT::SEMICOLON);
			break;
		case (TT::SEMICOLON):
			pc.Match (TT::SEMICOLON);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::SEMICOLON, TT::PAREN_OPEN });
			pc.Synch ({ TT::VAR, TT::BEGIN, TT::PROCEDURE });
	}
	return ret;
}
RetType Arguments (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PAREN_OPEN):
			pc.Match (TT::PAREN_OPEN);
			ParameterList (pc);
			pc.Match (TT::PAREN_CLOSE);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PAREN_OPEN });
			pc.Synch ({ TT::SEMICOLON });
	}
	return ret;
}
RetType param_list_id (ParserContext &pc)
{
	auto tid = pc.Current ();
	pc.Match (TT::ID);

	pc.Match (TT::COLON);
	auto t = Type (pc);
	if (HasSymbol (tid))
	{
		bool exists = pc.tree.AddVariable (GetSymbol (tid), t, true);
		if (exists) { pc.LogErrorUniqueIdentifier (pc.Current ()); }
	}
	return ParameterListPrime (pc);
}

RetType ParameterList (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			return param_list_id (pc);

		default:
			pc.LogErrorExpectedGot ({ TT::ID });
			pc.Synch ({ TT::PAREN_CLOSE });
	}
	return ret;
}
RetType param_list_prime_id (ParserContext &pc)
{
	pc.Match (TT::SEMICOLON);
	auto tid = pc.Current ();
	pc.Match (TT::ID);
	pc.Match (TT::COLON);
	auto t = Type (pc);
	if (HasSymbol (tid))
	{
		bool exists = pc.tree.AddVariable (GetSymbol (tid), t, true);
		if (exists) { pc.LogErrorUniqueIdentifier (pc.Current ()); }
	}
	return ParameterListPrime (pc);
}
RetType ParameterListPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::SEMICOLON):
			return param_list_prime_id (pc);
		case (TT::PAREN_CLOSE):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::SEMICOLON, TT::PAREN_CLOSE });
			pc.Synch ({ TT::PAREN_CLOSE });
	}
	return ret;
	// e-prod
}
RetType CompoundStatement (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::BEGIN):
			pc.Match (TT::BEGIN);
			CompoundStatementFactored (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::BEGIN });

			pc.Synch ({ TT::SEMICOLON, TT::DOT });
	}
	return ret;
}
RetType CompoundStatementFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::END):
			pc.Match (TT::END);
			pc.tree.Pop ();
			break;
		case (TT::BEGIN):
		case (TT::ID):
		case (TT::CALL):
		case (TT::IF):
		case (TT::WHILE):
			OptionalStatements (pc);
			pc.Match (TT::END);
			pc.tree.Pop ();
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::END, TT::BEGIN, TT::ID, TT::CALL, TT::IF, TT::WHILE });
			pc.Synch ({ TT::SEMICOLON, TT::DOT });
	}
	return ret;
}
RetType OptionalStatements (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::WHILE):
		case (TT::BEGIN):
		case (TT::IF):
		case (TT::CALL):
			StatementList (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL });
			pc.Synch ({ TT::END });
	}
	return ret;
}
RetType StatementList (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::WHILE):
		case (TT::BEGIN):
		case (TT::IF):
		case (TT::CALL):
			Statement (pc);
			StatementListPrime (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL });
			pc.Synch ({ TT::END });
			return RT_err;
	}
	return ret;
}
RetType StatementListPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::SEMICOLON):
			pc.Match (TT::SEMICOLON);
			Statement (pc);
			StatementListPrime (pc);
			break;
		case (TT::END):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::END, TT::SEMICOLON });
			pc.Synch ({ TT::END });
			return RT_err;
	}
	return ret;
	// e -prod
}

RetType stmt_id (ParserContext &pc)
{
	auto ret = Variable (pc);
	pc.Match (TT::ASSIGNOP);
	auto eret = Expression (pc, ret);

	if (ret != eret)
	{
		pc.LogErrorSem ("Var & expr have dif type yo!");
		return RT_err;
	}
	return RT_none;
}
RetType stmt_call (ParserContext &pc) { return ProcedureStatement (pc); }
RetType stmt_if (ParserContext &pc)
{
	pc.Match (TT::IF);
	auto ret = Expression (pc, RT_none);
	if (ret != RT_bool)
	{ pc.LogErrorSem ("Comparison must use a boolean type, not " + ret.to_string ()); }
	pc.Match (TT::THEN);
	Statement (pc);
	return StatementFactoredElse (pc);
}
RetType stmt_while (ParserContext &pc)
{
	pc.Match (TT::WHILE);
	auto ret = Expression (pc, RT_none);
	if (ret != RT_bool)
	{ pc.LogErrorSem ("While condition must use a boolean type, not " + ret.to_string ()); }
	pc.Match (TT::DO);
	Statement (pc);
}
RetType stmt_begin (ParserContext &pc)
{
	pc.Match (TT::BEGIN);
	return StatementFactoredBegin (pc);
}

RetType Statement (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			return stmt_id (pc);
		case (TT::WHILE):
			return stmt_while (pc);
		case (TT::BEGIN):
			return stmt_begin (pc);
		case (TT::IF):
			return stmt_if (pc);
		case (TT::CALL):
			return stmt_call (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL });
			pc.Synch ({ TT::SEMICOLON, TT::ELSE, TT::END });
			return RT_err;
	}
	return ret;
}
RetType StatementFactoredBegin (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::END):
			pc.Match (TT::END);
			pc.tree.Pop ();
			break;
		case (TT::BEGIN):
		case (TT::ID):
		case (TT::CALL):
		case (TT::IF):
		case (TT::WHILE):
			OptionalStatements (pc);
			pc.Match (TT::END);
			pc.tree.Pop ();
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::END, TT::BEGIN, TT::ID, TT::CALL, TT::IF, TT::WHILE });
			pc.Synch ({ TT::SEMICOLON, TT::ELSE, TT::END });
			return RT_err;
	}
	return ret;
}
RetType StatementFactoredElse (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ELSE):
			pc.Match (TT::ELSE);
			Statement (pc);
			break;
		case (TT::SEMICOLON):
		case (TT::END):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::END, TT::ELSE, TT::SEMICOLON });
			pc.Synch ({ TT::SEMICOLON, TT::ELSE, TT::END });
			return RT_err;
	}
	return ret;
	// e-prod
}

RetType var_id(ParserContext &pc)
{
	auto exists = pc.tree.CheckVariable(GetSymbol(pc.Current()));
	if (!exists.has_value()) { pc.LogErrorIdentifierScope(pc.Current()); }
	pc.Match(TT::ID);
	auto ret = VariableFactored(pc);
	if (ret == RT_int) {}
	return RT_none;

}
RetType Variable (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			return var_id(pc);

		default:
			pc.LogErrorExpectedGot ({ TT::ID });
			pc.Synch ({ TT::ASSIGNOP });
			return RT_err;
	}
	return ret;
}
RetType var_factored_bracket_open(ParserContext &pc)
{
	pc.Match(TT::BRACKET_OPEN);
	auto rt = Expression(pc, RT_none);
	pc.Match(TT::BRACKET_CLOSE);
	if (rt != RT_int)
	{
		pc.LogErrorSem("Array index must be of type int, not " + rt.to_string());
		return RT_err;
	}
	return RT_int;
}

RetType VariableFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::BRACKET_OPEN):
			return var_factored_bracket_open(pc);
		case (TT::ASSIGNOP):
			return RT_none;
		default:
			pc.LogErrorExpectedGot ({ TT::BRACKET_OPEN, TT::ASSIGNOP });
			pc.Synch ({ TT::ASSIGNOP });
			return RT_err;
	}
	return ret;
	// e-prod
}
RetType proc_stmt_call(ParserContext &pc)
{
	RetType ret = RT_none;
	pc.Match(TT::CALL);
	bool exists = pc.tree.CheckProcedure(GetSymbol(pc.Current()));
	if (!exists)
	{
		ret = RT_err;
		pc.LogErrorProcedureScope(pc.Current());
	}
	auto tid = GetSymbol(pc.Current());
	pc.Match(TT::ID);

	return ProcedureStatmentFactored(pc, tid, ret);
}
RetType ProcedureStatement (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::CALL):
			return proc_stmt_call(pc);

		default:
			pc.LogErrorExpectedGot ({ TT::CALL });
			pc.Synch ({ TT::SEMICOLON, TT::ELSE, TT::END });
			return RT_err;
	}
	return ret;
}
RetType proc_stmt_factored_paren_open(ParserContext &pc, SymbolID id, RetType in)
{
	pc.Match(TT::PAREN_OPEN);
	std::vector<RetType> expr_list;
	auto ret = ExpressionList(pc, expr_list, in);
	auto param_list = pc.tree.SubProcedureType(id);
	if (in != RT_err)
	{
		bool isEqual = true;
		for (int i = 0; i < expr_list.size(); i++)
		{
			if (param_list.size() <= i) { isEqual = false; }
			else if (param_list.at(i) != expr_list.at(i))
			{
				pc.LogErrorSem("Argument " + std::to_string(i) + " is type "
					+ expr_list.at(i).to_string() + " when it should be of type "
					+ param_list.at(i).to_string());
				isEqual = false;
			}
		}
		if (param_list.size() > expr_list.size())
		{
			pc.LogErrorSem("The call to procedure \"" + pc.SymbolName(id) + "\" has "
				+ std::to_string(param_list.size() - expr_list.size())
				+ " to few arguments");
		}
		else if (param_list.size() < expr_list.size())
		{
			pc.LogErrorSem("The call to procedure \"" + pc.SymbolName(id) + "\" has "
				+ std::to_string(expr_list.size() - param_list.size())
				+ " to many arguments");
		}
		if (!isEqual) { ret = RT_err; }
	}
	pc.Match(TT::PAREN_CLOSE);
	return ret;
}

RetType ProcedureStatmentFactored (ParserContext &pc, SymbolID id, RetType in)
{
	RetType ret = RT_none;
	switch (pc.Current ().type)
	{
		case (TT::PAREN_OPEN):
			return proc_stmt_factored_paren_open(pc, id, in);
		case (TT::SEMICOLON):
		case (TT::ELSE):
		case (TT::END):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::PAREN_OPEN, TT::SEMICOLON, TT::ELSE });
			pc.Synch ({ TT::ELSE, TT::SEMICOLON, TT::END });
			return RT_err;
	}
	// e-prod
	return ret;
}
RetType ExpressionList (ParserContext &pc, std::vector<RetType> &expr_list, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::PAREN_OPEN):
		case (TT::NUM):
		case (TT::NOT):
		case (TT::SIGN):
			ret = Expression (pc, in);
			expr_list.push_back (ret);
			ExpressionListPrime (pc, expr_list, in);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT, TT::SIGN });
			pc.Synch ({ TT::PAREN_CLOSE });
			return RT_err;
	}
	return ret;
}
RetType ExpressionListPrime (ParserContext &pc, std::vector<RetType> &expr_list, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			pc.Match (TT::COMMA);
			ret = Expression (pc, in);
			expr_list.push_back (ret);
			ExpressionListPrime (pc, expr_list, in);
			break;

		case (TT::PAREN_CLOSE):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::PAREN_CLOSE, TT::COMMA });
			pc.Synch ({ TT::PAREN_CLOSE });
			return RT_err;
	}
	return ret;
	// e -prod
}
RetType expr(ParserContext &pc, RetType in)
{
	auto sr = SimpleExpression(pc, in);
	auto er = ExpressionFactored(pc, sr);
	if (sr == RT_err || er == RT_err)
	{
		return RT_err; // propagate
	}
	else if (er == RT_none)
	{
		return sr; // no relation operator
	}
	else
	{
		if (sr != er)
		{
			pc.LogErrorSem("Can't compare types" + sr.to_string() + "and " + er.to_string());
			return RT_err; // relop in expr_factored
		}
		else
		{
			return RT_bool;
		}
	}
}
RetType Expression (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::PAREN_OPEN):
		case (TT::NUM):
		case (TT::NOT):
		case (TT::SIGN):
			return expr(pc, in);
		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT, TT::SIGN });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	return ret;
}
RetType expr_factored_relop(ParserContext &pc, RetType in) {
	pc.Match(TT::RELOP);
	auto ser = SimpleExpression(pc, in);
	return ser;
}

RetType ExpressionFactored (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::RELOP):
			return expr_factored_relop(pc,in);
		case (TT::PAREN_CLOSE):
		case (TT::SEMICOLON):
		case (TT::BRACKET_CLOSE):
		case (TT::COMMA):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
		case (TT::END):
			return RT_none; // means no relop
			break;
		default:
			pc.LogErrorExpectedGot (
			{ TT::RELOP, TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	return ret;
	// e-prod
}
RetType simp_expr(ParserContext &pc, RetType in)
{
	auto tr = Term(pc, in);
	auto sr = SimpleExpressionPrime(pc, tr);
	if (sr == RT_none) { return tr; }
	if (tr != sr) {}
	return RT_none; //TODO
}
RetType simp_expr_sign(ParserContext &pc, RetType in)
{
	pc.Match(TT::SIGN);
	auto tr = Term(pc, in);
	auto sr = SimpleExpressionPrime(pc, RT_none);
	if (sr == RT_none) { return tr; }

	return RT_none;//TODO
}
RetType SimpleExpression (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::PAREN_OPEN):
		case (TT::NUM):
		case (TT::NOT):
			return simp_expr(pc, in);
		case (TT::SIGN):
			return simp_expr_sign(pc, in);

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT, TT::SIGN });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	return ret;
}
RetType simp_expr_prime_add(ParserContext &pc, RetType in)
{
	RetType ret = RT_none;

	if (in == RT_err) { ret = RT_err; }
	switch (pc.Current().type)
	{
		case (TT::ADDOP):
			pc.Match(TT::ADDOP);
			break;
		case (TT::SIGN):
			pc.Match(TT::SIGN);
			break;
	}

	auto tr = Term(pc, in);
	if (tr == RT_int || tr == RT_real) {}
	auto sr = SimpleExpressionPrime(pc, in);
	if (sr == RT_none)
	{
		if (tr != sr)
		{
			pc.LogErrorSem(
				"Cannot add values of type " + sr.to_string() + " and " + tr.to_string());
			return RT_err;
		}
	}
	if (sr == RT_err) return sr;
	return ret;
}
RetType SimpleExpressionPrime (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::SIGN):
		case (TT::ADDOP):
			return simp_expr_prime_add(pc,in);
		case (TT::PAREN_CLOSE):
		case (TT::SEMICOLON):
		case (TT::BRACKET_CLOSE):
		case (TT::COMMA):
		case (TT::RELOP):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
		case (TT::END):
			return RT_none; // no addop
			break;
		default:
			pc.LogErrorExpectedGot (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::SIGN, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	// e -prod
	return ret;
}
RetType Term (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::NUM):
		case (TT::PAREN_OPEN):
		case (TT::NOT):
			ret = Factor (pc, in);
			ret = TermPrime (pc, ret);
			return ret;
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::RELOP, TT::ADDOP, TT::SIGN, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	return ret;
}
RetType TermPrime (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::MULOP):
			pc.Match (TT::MULOP);
			ret = Factor (pc, in);

			ret = TermPrime (pc, ret);
		
			break;
		case (TT::SIGN):
		case (TT::ADDOP):
		case (TT::RELOP):
		case (TT::PAREN_CLOSE):
		case (TT::SEMICOLON):
		case (TT::BRACKET_CLOSE):
		case (TT::COMMA):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
		case (TT::END):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::PAREN_CLOSE,
			TT::SEMICOLON,
			TT::BRACKET_CLOSE,
			TT::COMMA,
			TT::MULOP,
			TT::SIGN,
			TT::ADDOP,
			TT::RELOP,
			TT::THEN,
			TT::ELSE,
			TT::DO,
			TT::END });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::SIGN, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::END, TT::DO });
			return RT_err;
	}
	return ret;
	// e -prod
}
RetType factor_id(ParserContext &pc, RetType in)
{
	auto exists = pc.tree.CheckVariable(GetSymbol(pc.Current()));
	if (!exists.has_value())
	{
		pc.LogErrorIdentifierScope(pc.Current());

		pc.Match(TT::ID);
		return FactorPrime(pc, RT_err);
	}
	else if (exists.value() == RT_none)
	{
		pc.LogErrorSem(std::string("Variable ")
			+ pc.SymbolName(GetSymbol(pc.Current())) + " has invalid type");
		pc.Match(TT::ID);
		auto ft = FactorPrime(pc, exists.value());
		return RT_err;
	}
	else
	{
		pc.Match(TT::ID);
		auto ft = FactorPrime(pc, exists.value());
		if (ft == RT_none) { return ft; }
		else
			return RT_err;
	}
}
RetType Factor (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			return factor_id(pc,in);
		case (TT::NUM):
			return [&]() -> RetType {
				auto tid = pc.Match (TT::NUM);
				if (std::get<NumType> (tid.attrib).val.index () == 0)
					return RT_int;
				else if (std::get<NumType> (tid.attrib).val.index () == 1)
					return RT_real;
				else
					return RT_err;
			}();
			break;
		case (TT::PAREN_OPEN):
			pc.Match (TT::PAREN_OPEN);
			ret = Expression (pc, in);
			pc.Match (TT::PAREN_CLOSE);
			return ret;
			break;
		case (TT::NOT):
			pc.Match (TT::NOT);
			ret = Factor (pc, in);
			if (ret == RT_bool) { return RT_bool; }
			else if (ret == RT_err)
			{
				return RT_err;
			}
			else
			{
				pc.LogErrorSem ("Cannot negate a non-boolean type, not " + ret.to_string ());
				return RT_err;
			}
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT });
			pc.Synch ({ TT::PAREN_CLOSE,
			TT::SEMICOLON,
			TT::BRACKET_CLOSE,
			TT::COMMA,
			TT::RELOP,
			TT::SIGN,
			TT::ADDOP,
			TT::MULOP,
			TT::THEN,
			TT::ELSE,
			TT::DO,
			TT::END });
			return RT_err;
	}
	return ret;
}

RetType factor_prime_braket_open(ParserContext &pc, RetType in)
{
	pc.Match(TT::BRACKET_OPEN);
	auto ret = Expression(pc, in);
	if (ret == RT_err)
	{
		pc.Match(TT::BRACKET_CLOSE);
		return RT_err;
	}
	else if (ret == RT_int)
	{
		pc.Match(TT::BRACKET_CLOSE);
		if (IsArrInt(in) || IsArrReal(in)) return RT_none;
		return RT_err;
	}
	else
	{
		pc.LogErrorSem("Array index must be of type int, not " + ret.to_string());
		pc.Match(TT::BRACKET_CLOSE);
		return RT_err;
	}
}
RetType FactorPrime (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::BRACKET_OPEN):
			return factor_prime_braket_open(pc, in);
		case (TT::PAREN_CLOSE):
		case (TT::SEMICOLON):
		case (TT::BRACKET_CLOSE):
		case (TT::COMMA):
		case (TT::RELOP):
		case (TT::SIGN):
		case (TT::ADDOP):
		case (TT::MULOP):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
		case (TT::END):
			return in;
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::BRACKET_OPEN,
			TT::PAREN_CLOSE,
			TT::SEMICOLON,
			TT::BRACKET_CLOSE,
			TT::COMMA,
			TT::RELOP,
			TT::SIGN,
			TT::ADDOP,
			TT::MULOP,
			TT::THEN,
			TT::ELSE,
			TT::DO,
			TT::END });
			pc.Synch ({ TT::PAREN_CLOSE,
			TT::SEMICOLON,
			TT::BRACKET_CLOSE,
			TT::COMMA,
			TT::RELOP,
			TT::SIGN,
			TT::ADDOP,
			TT::MULOP,
			TT::THEN,
			TT::ELSE,
			TT::DO,
			TT::END });
			return RT_err;
	}
	return ret;
}

RetType Sign (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::SIGN):
			pc.Match (TT::SIGN);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::SIGN });
			pc.Synch ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT });
			return RT_err;
	}
	return ret;
}

} // namespace Parser