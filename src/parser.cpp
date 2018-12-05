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
bool IsArrayType (RetType rt) { return IsArrInt (rt) || IsArrReal (rt); }

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
		if (s == procedures.at (curProc).name) { return procedures.at (curProc).Signature (); }
		for (auto [s_id, proc_id] : procedures.at (curProc).sub_procs)
		{
			if (s == s_id) { return procedures.at (proc_id).Signature (); }
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
	ProcedureID curProc = eye;
	while (curProc != -1)
	{
		for (auto [id, type] : procedures.at (curProc).params)
		{
			if (id == s) { return type; }
		}
		for (auto [id, type] : procedures.at (curProc).locals)
		{
			if (id == s) { return type; }
		}
		curProc = procedures.at (curProc).parent;
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
	pc.Match (TT::PROG);

	ProcedureID cur = pc.tree.SetStartProcedure (GetSymbol (pc.Current ()));
	if (cur == -1) { pc.LogErrorUniqueProcedure (pc.Current ()); }
	pc.tree.Push (cur);
	pc.Match (TT::ID);

	pc.Match (TT::P_O);

	IdentifierList (pc);
	pc.Match (TT::P_C);
	pc.Match (TT::SEMIC);
	return ProgramStatementFactored (pc);
}

RetType ProgramStatement (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::PROG):
			return prog_stmt_program (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::PROG });
			pc.Synch ({ TT::END_FILE });
			return RT_err;
	}
}
RetType ProgramStatementFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			Declarations (pc);
			ProgramStatementFactoredFactored (pc);
			break;
		case (TT::PROC):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			pc.Match (TT::DOT);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc);
			pc.Match (TT::DOT);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::VAR, TT::PROC, TT::BEGIN });
			pc.Synch ({ TT::END_FILE });
			return RT_err;
	}
}
RetType ProgramStatementFactoredFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			pc.Match (TT::DOT);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc);
			pc.Match (TT::DOT);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PROC, TT::BEGIN });
			pc.Synch ({ TT::END_FILE });
			return RT_err;
	}
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
	switch (pc.Current ().type)
	{
		case (TT::ID):
			return ident_list_id (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::ID });
			pc.Synch ({ TT::P_C });
			return RT_err;
	}
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


	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			return ident_list_prime (pc);
		case (TT::P_C):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::COMMA, TT::P_C });
			pc.Synch ({ TT::P_C });
	}

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

	pc.Match (TT::SEMIC);
	return DeclarationsPrime (pc);
}

RetType Declarations (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			return decls (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::VAR });
			pc.Synch ({ TT::PROC, TT::BEGIN });
			return RT_err;
	}
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
	pc.Match (TT::SEMIC);
	return DeclarationsPrime (pc);
}

RetType DeclarationsPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			return decls_prime (pc);
		case (TT::PROC):
		case (TT::BEGIN):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::PROC, TT::BEGIN, TT::VAR });
			pc.Synch ({ TT::PROC, TT::BEGIN });
			return RT_err;
	}
	// e-prod
}

RetType type_array (ParserContext &pc)
{
	RetType ret = RT_none;
	pc.Match (TT::ARRAY);
	pc.Match (TT::B_O);
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
	pc.Match (TT::B_C);
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
	switch (pc.Current ().type)
	{
		case (TT::ARRAY):
			return type_array (pc);
		case (TT::STD_T):
			return StandardType (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ARRAY, TT::STD_T });
			pc.Synch ({ TT::P_C, TT::SEMIC });
			return RT_err;
	}
}

RetType std_type (ParserContext &pc)
{
	auto t = pc.Match (TT::STD_T);
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
	switch (pc.Current ().type)
	{
		case (TT::STD_T):
			return std_type (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::STD_T });
			pc.Synch ({ TT::P_C, TT::SEMIC });
			return RT_err;
	}
}
RetType SubprogramDeclarations (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramDeclaration (pc);
			pc.Match (TT::SEMIC);
			SubprogramDeclarationsPrime (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PROC });
			pc.Synch ({ TT::BEGIN });
			return RT_err;
	}
}
RetType SubprogramDeclarationsPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramDeclaration (pc);
			pc.Match (TT::SEMIC);
			SubprogramDeclarationsPrime (pc);
			break;
		case (TT::BEGIN):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::PROC, TT::BEGIN });
			pc.Synch ({ TT::BEGIN });
			return RT_err;
	}
	// e-prod
}
RetType SubprogramDeclaration (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramHead (pc);
			SubprogramDeclarationFactored (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PROC });
			pc.Synch ({ TT::SEMIC });
			return RT_err;
	}
}
RetType SubprogramDeclarationFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			Declarations (pc);
			SubprogramDeclarationFactoredFactored (pc);
			break;
		case (TT::PROC):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::VAR, TT::PROC, TT::BEGIN });
			pc.Synch ({ TT::SEMIC });
			return RT_err;
	}
}
RetType SubprogramDeclarationFactoredFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramDeclarations (pc);
			CompoundStatement (pc);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PROC, TT::BEGIN });
			pc.Synch ({ TT::SEMIC });
			return RT_err;
	}
}

RetType sub_prog_head_procedure (ParserContext &pc)
{
	pc.Match (TT::PROC);
	ProcedureID cur = pc.tree.AddSubProcedure (GetSymbol (pc.Current ()));
	if (cur == -1) { pc.LogErrorUniqueProcedure (pc.Current ()); }
	pc.Match (TT::ID);
	pc.tree.Push (cur);

	return SubprogramHeadFactored (pc);
}

RetType SubprogramHead (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			return sub_prog_head_procedure (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::PROC });
			pc.Synch ({ TT::PROC, TT::VAR, TT::BEGIN });
			return RT_err;
	}
}
RetType SubprogramHeadFactored (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::P_O):
			Arguments (pc);
			pc.Match (TT::SEMIC);
			break;
		case (TT::SEMIC):
			pc.Match (TT::SEMIC);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::SEMIC, TT::P_O });
			pc.Synch ({ TT::VAR, TT::BEGIN, TT::PROC });
			return RT_err;
	}
}
RetType Arguments (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::P_O):
			pc.Match (TT::P_O);
			ParameterList (pc);
			pc.Match (TT::P_C);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::P_O });
			pc.Synch ({ TT::SEMIC });
			return RT_err;
	}
}
RetType param_list_id (ParserContext &pc)
{
	auto tid = pc.Current ();
	pc.Match (TT::ID);

	pc.Match (TT::COLON);
	auto t = Type (pc);
	if (t == RT_err) { pc.LogErrorSem ("Parameter type cannot be an error"); }

	if (t == RT_none) { pc.LogErrorSem ("Parameter type cannot be none"); }

	if (HasSymbol (tid))
	{
		bool exists = pc.tree.AddVariable (GetSymbol (tid), t, true);
		if (exists) { pc.LogErrorUniqueIdentifier (pc.Current ()); }
	}
	return ParameterListPrime (pc);
}

RetType ParameterList (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			return param_list_id (pc);

		default:
			pc.LogErrorExpectedGot ({ TT::ID });
			pc.Synch ({ TT::P_C });
			return RT_err;
	}
}
RetType param_list_prime_id (ParserContext &pc)
{
	pc.Match (TT::SEMIC);
	auto tid = pc.Current ();
	pc.Match (TT::ID);
	pc.Match (TT::COLON);
	auto t = Type (pc);
	if (t == RT_err) { pc.LogErrorSem ("Parameter type cannot be an error"); }

	if (t == RT_none) { pc.LogErrorSem ("Parameter type cannot be none"); }
	if (HasSymbol (tid))
	{
		bool exists = pc.tree.AddVariable (GetSymbol (tid), t, true);
		if (exists) { pc.LogErrorUniqueIdentifier (pc.Current ()); }
	}
	return ParameterListPrime (pc);
}
RetType ParameterListPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::SEMIC):
			return param_list_prime_id (pc);
		case (TT::P_C):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::SEMIC, TT::P_C });
			pc.Synch ({ TT::P_C });
			return RT_err;
	}
	// e-prod
}
RetType CompoundStatement (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::BEGIN):
			pc.Match (TT::BEGIN);
			CompoundStatementFactored (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::BEGIN });
			pc.Synch ({ TT::SEMIC, TT::DOT });
			return RT_err;
	}
}
RetType CompoundStatementFactored (ParserContext &pc)
{
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
			pc.Synch ({ TT::SEMIC, TT::DOT });
			return RT_err;
	}
}
RetType OptionalStatements (ParserContext &pc)
{
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
			return RT_err;
	}
}
RetType StatementList (ParserContext &pc)
{
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
}
RetType StatementListPrime (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::SEMIC):
			pc.Match (TT::SEMIC);
			Statement (pc);
			StatementListPrime (pc);
			break;
		case (TT::END):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::END, TT::SEMIC });
			pc.Synch ({ TT::END });
			return RT_err;
	}
	// e -prod
}

RetType stmt_id (ParserContext &pc)
{
	auto ret = Variable (pc);
	pc.Match (TT::A_OP);
	auto eret = Expression (pc, ret);
	if (ret == RT_err)
		if (ret != eret)
		{
			pc.LogErrorSem ("Cannot assign type " + eret.to_string () + " to type " + ret.to_string ());
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
	{ pc.LogErrorSem ("Conditional must use a boolean type, not " + ret.to_string ()); }
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
	return Statement (pc);
}
RetType stmt_begin (ParserContext &pc)
{
	pc.Match (TT::BEGIN);
	return StatementFactoredBegin (pc);
}

RetType Statement (ParserContext &pc)
{
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
			pc.Synch ({ TT::SEMIC, TT::ELSE, TT::END });
			return RT_err;
	}
}
RetType StatementFactoredBegin (ParserContext &pc)
{
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
			pc.Synch ({ TT::SEMIC, TT::ELSE, TT::END });
			return RT_err;
	}
}
RetType StatementFactoredElse (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::ELSE):
			pc.Match (TT::ELSE);
			Statement (pc);
			break;
		case (TT::SEMIC):
		case (TT::END):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::END, TT::ELSE, TT::SEMIC });
			pc.Synch ({ TT::SEMIC, TT::ELSE, TT::END });
			return RT_err;
	}
	// e-prod
}

RetType var_id (ParserContext &pc)
{
	RetType ret = RT_none;
	auto exists = pc.tree.CheckVariable (GetSymbol (pc.Current ()));
	if (!exists.has_value ())
	{
		ret = RT_err;
		pc.LogErrorIdentifierScope (pc.Current ());

		pc.Match (TT::ID);
		return VariableFactored (pc, RT_err);
	}
	else
	{
		pc.Match (TT::ID);
		auto vr = VariableFactored (pc, exists.value ());
		if (vr == RT_none)
		{
			if (IsArrayType (exists.value ())) { return exists.value (); }
			if (exists.value () == RT_real || exists.value () == RT_int) { return exists.value (); }
			else
			{
				pc.LogErrorSem ("Variable can not be type" + exists.value ().to_string ());
				return RT_err;
			}
		}
		return vr;
	}
}
RetType Variable (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			return var_id (pc);
		default:
			pc.LogErrorExpectedGot ({ TT::ID });
			pc.Synch ({ TT::A_OP });
			return RT_err;
	}
}
RetType var_factored_bracket_open (ParserContext &pc, RetType in)
{
	pc.Match (TT::B_O);
	auto rt = Expression (pc, RT_none);
	pc.Match (TT::B_C);
	if (in == RT_err) return RT_err;
	if (rt != RT_int)
	{
		pc.LogErrorSem ("Array index must be of type int, not " + rt.to_string ());
		return RT_err;
	}
	if (IsArrInt (in)) return RT_int;
	if (IsArrReal (in)) return RT_real;

	pc.LogErrorSem ("Array must be of type int or real, not " + in.to_string ());
	return RT_err;
}

RetType VariableFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::B_O):
			return var_factored_bracket_open (pc, in);
		case (TT::A_OP):
			return RT_none;
		default:
			pc.LogErrorExpectedGot ({ TT::B_O, TT::A_OP });
			pc.Synch ({ TT::A_OP });
			return RT_err;
	}
	// e-prod
}
RetType proc_stmt_call (ParserContext &pc)
{
	RetType ret = RT_none;
	pc.Match (TT::CALL);
	bool exists = pc.tree.CheckProcedure (GetSymbol (pc.Current ()));
	if (!exists)
	{
		ret = RT_err;
		pc.LogErrorSem ("Procedure \"" + pc.SymbolName (GetSymbol (pc.Current ())) + "\" not in current scope");
	}
	auto tid = GetSymbol (pc.Current ());
	pc.Match (TT::ID);

	return ProcedureStatmentFactored (pc, tid, ret);
}
RetType ProcedureStatement (ParserContext &pc)
{
	switch (pc.Current ().type)
	{
		case (TT::CALL):
			return proc_stmt_call (pc);

		default:
			pc.LogErrorExpectedGot ({ TT::CALL });
			pc.Synch ({ TT::SEMIC, TT::ELSE, TT::END });
			return RT_err;
	}
}
RetType proc_stmt_factored_paren_open (ParserContext &pc, SymbolID id, RetType in)
{
	pc.Match (TT::P_O);
	std::vector<RetType> expr_list;
	auto ret = ExpressionList (pc, expr_list, in);
	auto param_list = pc.tree.SubProcedureType (id);
	if (in != RT_err)
	{
		bool isEqual = true;
		for (int i = 0; i < expr_list.size (); i++)
		{
			if (param_list.size () <= i) { isEqual = false; }
			else if (param_list.at (i) != expr_list.at (i))
			{
				pc.LogErrorSem ("Argument " + std::to_string (i) + " is type " + expr_list.at (i).to_string ()
				                + " when it should be of type " + param_list.at (i).to_string ());
				isEqual = false;
				ret = RT_err;
			}
		}
		if (param_list.size () > expr_list.size ())
		{
			ret = RT_err;
			pc.LogErrorSem ("The call to procedure \"" + pc.SymbolName (id) + "\" has "
			                + std::to_string (param_list.size () - expr_list.size ()) + " to few arguments");
		}
		else if (param_list.size () < expr_list.size ())
		{
			ret = RT_err;
			pc.LogErrorSem ("The call to procedure \"" + pc.SymbolName (id) + "\" has "
			                + std::to_string (expr_list.size () - param_list.size ()) + " to many arguments");
		}
		if (!isEqual) { ret = RT_err; }
	}
	pc.Match (TT::P_C);
	return ret;
}

RetType ProcedureStatmentFactored (ParserContext &pc, SymbolID id, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::P_O):
			return proc_stmt_factored_paren_open (pc, id, in);
		case (TT::SEMIC):
		case (TT::ELSE):
		case (TT::END):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::P_O, TT::SEMIC, TT::ELSE });
			pc.Synch ({ TT::ELSE, TT::SEMIC, TT::END });
			return RT_err;
	}
	// e-prod
}

RetType expr_list_elem (ParserContext &pc, std::vector<RetType> &expr_list, RetType in)
{
	auto t = Expression (pc, in);
	expr_list.push_back (t);
	return ExpressionListPrime (pc, expr_list, in);
}
RetType ExpressionList (ParserContext &pc, std::vector<RetType> &expr_list, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::P_O):
		case (TT::NUM):
		case (TT::NOT):
		case (TT::SIGN):
			return expr_list_elem (pc, expr_list, in);

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::P_O, TT::NUM, TT::NOT, TT::SIGN });
			pc.Synch ({ TT::P_C });
			return RT_err;
	}
}
RetType expr_list_prime_elem (ParserContext &pc, std::vector<RetType> &expr_list, RetType in)
{
	pc.Match (TT::COMMA);
	auto t = Expression (pc, in);
	expr_list.push_back (t);
	return ExpressionListPrime (pc, expr_list, in);
}
RetType ExpressionListPrime (ParserContext &pc, std::vector<RetType> &expr_list, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			return expr_list_prime_elem (pc, expr_list, in);
		case (TT::P_C):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::P_C, TT::COMMA });
			pc.Synch ({ TT::P_C });
			return RT_err;
	}
	// e -prod
}
RetType expr (ParserContext &pc, RetType in)
{
	auto sr = SimpleExpression (pc, in);
	auto er = ExpressionFactored (pc, sr);
	if (in == RT_err || sr == RT_err || er == RT_err)
	{
		return RT_err; // propagate
	}
	else if (er == RT_none)
	{
		return sr; // no relation operator
	}
	else
	{
		return er;
		// if (sr != er)
		//{
		//	pc.LogErrorSem ("Can't compare types" + sr.to_string () + "and " + er.to_string ());
		//	return RT_err; // relop in expr_factored
		//}
		// else
		//{
		//	return RT_bool;
		//}
	}
}
RetType Expression (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::P_O):
		case (TT::NUM):
		case (TT::NOT):
		case (TT::SIGN):
			return expr (pc, in);
		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::P_O, TT::NUM, TT::NOT, TT::SIGN });
			pc.Synch ({ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
}
RetType expr_factored_relop (ParserContext &pc, RetType in)
{
	pc.Match (TT::RELOP);
	auto ser = SimpleExpression (pc, in == RT_err ? RT_err : RT_bool);
	if (in == RT_err || ser == RT_err) return RT_err;
	if (in == ser) { return RT_bool; }
	else
	{
		pc.LogErrorSem ("Cannot compare types " + in.to_string () + " and " + ser.to_string ());
		return RT_err;
	}
}

RetType ExpressionFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::RELOP):
			return expr_factored_relop (pc, in);
		case (TT::P_C):
		case (TT::SEMIC):
		case (TT::B_C):
		case (TT::COMMA):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
		case (TT::END):
			return RT_none; // means no relop
			break;
		default:
			pc.LogErrorExpectedGot (
			{ TT::RELOP, TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
			pc.Synch ({ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	// e-prod
}
RetType simp_expr (ParserContext &pc, RetType in)
{
	auto tr = Term (pc, in);
	auto sr = SimpleExpressionPrime (pc, tr);
	if (in == RT_err || tr == RT_err || sr == RT_err) { return RT_err; }
	if (sr == RT_none) { return tr; }
	if (tr != sr)
	{
		pc.LogErrorSem ("Types cannot mismatch: " + tr.to_string () + " and " + sr.to_string ());
		return RT_err;
	}
	return sr; // TODO
}
RetType simp_expr_sign (ParserContext &pc, RetType in)
{
	pc.Match (TT::SIGN);
	auto tr = Term (pc, in);
	auto sr = SimpleExpressionPrime (pc, tr);
	if (in == RT_err || tr == RT_err || sr == RT_err) { return RT_err; }
	if (sr == RT_none) { return tr; }
	if (tr != sr)
	{
		pc.LogErrorSem ("Types cannot mismatch:" + tr.to_string () + " and " + sr.to_string ());
		return RT_err;
	}
	return sr; // TODO
}
RetType SimpleExpression (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::P_O):
		case (TT::NUM):
		case (TT::NOT):
			return simp_expr (pc, in);
		case (TT::SIGN):
			return simp_expr_sign (pc, in);

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::P_O, TT::NUM, TT::NOT, TT::SIGN });
			pc.Synch ({ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
}
RetType simp_expr_prime_add (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ADDOP):
			pc.Match (TT::ADDOP);
			break;
		case (TT::SIGN):
			Sign (pc, in);
			break;
	}
	auto tr = Term (pc, in);
	auto sr = SimpleExpressionPrime (pc, tr);

	if (in == RT_err || tr == RT_err || sr == RT_err) { return RT_err; }

	if (sr == RT_none)
	{
		if (in != tr)
		{
			pc.LogErrorSem ("Cannot add values of type " + in.to_string () + " and " + tr.to_string ());
			return RT_err;
		}
		return tr;
	}
	else
	{
		if (in != tr)
		{
			pc.LogErrorSem ("Cannot add values of type " + in.to_string () + " and " + tr.to_string ());
			return RT_err;
		}
		if (tr != sr)
		{
			pc.LogErrorSem ("Types in an expression must be the same, not " + tr.to_string ()
			                + " and " + sr.to_string ());
			return RT_err;
		}
		return sr;
	}
}
RetType SimpleExpressionPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::SIGN):
		case (TT::ADDOP):
			return simp_expr_prime_add (pc, in);
		case (TT::P_C):
		case (TT::SEMIC):
		case (TT::B_C):
		case (TT::COMMA):
		case (TT::RELOP):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
		case (TT::END):
			return RT_none; // no addop
		default:
			pc.LogErrorExpectedGot (
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::SIGN, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			pc.Synch ({ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	// e -prod
}
RetType term (ParserContext &pc, RetType in)
{
	auto fr = Factor (pc, in);
	auto tr = TermPrime (pc, fr);
	if (in == RT_err || fr == RT_err || tr == RT_err) return RT_err;
	if (tr == RT_none) { return fr; }
	else if (fr != tr)
	{
		pc.LogErrorSem ("Terms must be be the same type, not " + fr.to_string () + " and " + tr.to_string ());
		return RT_err;
	}

	return tr;
}
RetType Term (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::NUM):
		case (TT::P_O):
		case (TT::NOT):
			return term (pc, in);

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::P_O, TT::NUM, TT::NOT });
			pc.Synch (
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::ADDOP, TT::SIGN, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
}
RetType term_prime_mulop (ParserContext &pc, RetType in)
{
	pc.Match (TT::MULOP);
	auto fr = Factor (pc, in);
	auto tr = TermPrime (pc, fr);
	if (in == RT_err || fr == RT_err || tr == RT_err) return RT_err;
	if (tr == RT_none)
	{
		if (in != fr)
		{
			pc.LogErrorSem ("Cannot mulop between " + in.to_string () + " and " + fr.to_string ());
			return RT_err;
		}
		else
			return fr;
	}
	else if (fr != tr)
	{
		pc.LogErrorSem ("Terms must be be the same type, not " + fr.to_string () + " and " + tr.to_string ());
		return RT_err;
	}
	return tr;
}
RetType TermPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::MULOP):
			return term_prime_mulop (pc, in);
		case (TT::SIGN):
		case (TT::ADDOP):
		case (TT::RELOP):
		case (TT::P_C):
		case (TT::SEMIC):
		case (TT::B_C):
		case (TT::COMMA):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
		case (TT::END):
			return RT_none;
			break;
		default:
			pc.LogErrorExpectedGot (
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::MULOP, TT::SIGN, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			pc.Synch (
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::SIGN, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::END, TT::DO });
			return RT_err;
	}
	// e -prod
}
RetType factor_id (ParserContext &pc, RetType in)
{
	auto tid = pc.Current ();
	auto exists = pc.tree.CheckVariable (GetSymbol (tid));
	pc.Match (TT::ID);
	if (!exists.has_value ())
	{
		if (in != RT_err) { pc.LogErrorIdentifierScope (tid); }


		FactorPrime (pc, RT_err);
		return RT_err;
	}
	else if (exists.value () == RT_none)
	{
		if (in != RT_err)
		{
			pc.LogErrorSem (std::string ("Variable ") + pc.SymbolName (GetSymbol (tid)) + " has invalid type");
		}

		FactorPrime (pc, RT_err);
		return RT_err;
	}
	else
	{
		auto fp = FactorPrime (pc, exists.value ());

		if (in == RT_err) return RT_err;
		return fp;
	}
}
RetType factor_num (ParserContext &pc, RetType in)
{
	auto tid = pc.Match (TT::NUM);
	if (std::get<NumType> (tid.attrib).val.index () == 0)
		return RT_int;
	else if (std::get<NumType> (tid.attrib).val.index () == 1)
		return RT_real;
	else
		return RT_err;
}
RetType factor_paren_open (ParserContext &pc, RetType in)
{
	pc.Match (TT::P_O);
	auto ret = Expression (pc, in);
	pc.Match (TT::P_C);
	return ret;
}
RetType factor_not (ParserContext &pc, RetType in)
{
	pc.Match (TT::NOT);
	auto ret = Factor (pc, in);
	if (ret == RT_bool) { return RT_bool; }
	else if (ret == RT_err)
	{
		return RT_err;
	}
	else
	{
		if (in != RT_err)
			pc.LogErrorSem ("Can only negate booleans, not " + ret.to_string () + "s");
		return RT_err;
	}
}
RetType Factor (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			return factor_id (pc, in);
		case (TT::NUM):
			return factor_num (pc, in);
		case (TT::P_O):
			return factor_paren_open (pc, in);
		case (TT::NOT):
			return factor_not (pc, in);
		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::P_O, TT::NUM, TT::NOT });
			pc.Synch (
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::SIGN, TT::ADDOP, TT::MULOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
}

RetType factor_prime_braket_open (ParserContext &pc, RetType in)
{
	pc.Match (TT::B_O);
	auto ret = Expression (pc, in);
	pc.Match (TT::B_C);
	if (in == RT_err || ret == RT_err) { return RT_err; }
	if (ret != RT_int)
	{
		pc.LogErrorSem ("Array index must be an int, not " + ret.to_string ());
		return RT_err;
	}
	if (IsArrInt (in)) return RT_int;
	if (IsArrReal (in)) return RT_real;
	return RT_err;
}
RetType FactorPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::B_O):
			return factor_prime_braket_open (pc, in);
		case (TT::P_C):
		case (TT::SEMIC):
		case (TT::B_C):
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
			pc.LogErrorExpectedGot (
			{ TT::B_O, TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::SIGN, TT::ADDOP, TT::MULOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			pc.Synch (
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::SIGN, TT::ADDOP, TT::MULOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
}

RetType Sign (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::SIGN):
			pc.Match (TT::SIGN);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::SIGN });
			pc.Synch ({ TT::ID, TT::P_O, TT::NUM, TT::NOT });
			return RT_err;
	}
}

} // namespace Parser