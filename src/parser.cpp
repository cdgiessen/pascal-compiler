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

		if (newProcName == procedures.at (curProc).name) return -1;

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
	if (procedures.count (eye) == 1)
	{
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
	}
	else
	{
		int i = 0;
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
void ParseTree::Pop ()
{
	if (procedures.count (eye) == 1) eye = procedures.at (eye).parent;
}

ParserContext::ParserContext (CompilationContext &context, TokenStream &ts, Logger &logger)
: context (context), ts (ts), logger (logger)
{
}

TokenInfo ParserContext::Current () const { return ts.Current (); }
TokenInfo ParserContext::Advance () { return ts.Advance (); }

void ParserContext::LogErrorExpectedGot (std::vector<TT> const &types)
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


void ParserContext::LogErrorSem (RetType in, std::string msg, TokenInfo const &t)
{
	if (in != RT_err)
		logger.AddSemErrPrint (
		t.line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", "SEMERR: " + msg); });
}

void ParserContext::LogErrorUniqueProcedure (RetType in, TokenInfo const &t)
{
	LogErrorSem (in, "Procedure \"" + SymbolName (GetSymbol (t)) + "\" not unique in current scope", t);
}

void ParserContext::LogErrorIdentifierScope (RetType in, TokenInfo const &t)
{
	LogErrorSem (in, "Identifier \"" + SymbolName (GetSymbol (t)) + "\" not in current scope", t);
}

void ParserContext::LogErrorUniqueIdentifier (RetType in, TokenInfo const &t)
{
	LogErrorSem (in, "Identifier \"" + SymbolName (GetSymbol (t)) + "\" not unique in current scope", t);
}


std::string ParserContext::SymbolName (SymbolID loc)
{
	return context.symbolTable.SymbolView (loc);
}


void ParserContext::Print (OutputFileHandle &out)
{
	std::function<void(ProcedureID, int)> proc_print = [&, this](ProcedureID id, int width) {
		if (tree.procedures.count (id) == 1)
		{


			fmt::print (out.FP (),
			"{:=<{}}Procedure Id: {}; Name: {}\n",
			"",
			width,
			std::to_string (id),
			SymbolName (tree.procedures.at (id).name));

			uint32_t addr = 0;
			for (auto &[name, type] : tree.procedures.at (id).locals)
			{
				fmt::print (out.FP (),
				"{:-<{}}Local variable Name: {}, Type: {}, Address: {}\n",
				"",
				width,
				SymbolName (name),
				type.to_string (),
				std::to_string (addr));

				addr += type.size ();
			}
			for (auto &[name, sub_id] : tree.procedures.at (id).sub_procs)
			{
				proc_print (sub_id, width + 4);
			}
			fmt::print (out.FP (), "\n", " ", width);
		}
	};
	proc_print (0, 0);
}

void ParserContext::Match (TT tt, RetType &rt)
{
	using namespace std::string_literals;

	if (tt == Current ().type && tt != TT::END_FILE) { Advance (); }
	else if (tt == Current ().type && tt == TT::END_FILE)
	{
		rt = RT_none;
	}
	// throw "End of Parse";

	else //  (tt != Current ().type)
	{
		LogErrorExpectedGot ({ tt });
		Advance ();
		rt = RT_err;
	}
}

void ParserContext::Synch (std::vector<TT> const &set)
{
	TT tt = Current ().type;

	bool found = false;
	if (tt == TT::END_FILE) found = true;
	for (auto &s : set)
		if (s == tt) found = true;
	while (!found)
	{
		tt = Advance ().type;
		if (tt == TT::END_FILE) found = true;
		for (auto &s : set)
			if (s == tt) found = true;
	}
}

RetType DefaultErr (ParserContext &pc, std::vector<TT> got, std::vector<TT> synch)
{
	pc.LogErrorExpectedGot (got);
	pc.Synch (synch);
	return RT_err;
}

namespace Parser
{
void Parse (ParserContext &pc)
{
	// try
	RetType ret = RT_none;
	ProgramStatement (pc, ret);
	pc.Match (TT::END_FILE, ret);

	//}
	// catch (std::exception &err)
	//{
	//	fmt::print (std::string (err.what ()) + "\n");
	//	// found end file token, don't continue parsing
	//}
}

void prog_stmt_program (ParserContext &pc, RetType in)
{
	pc.Match (TT::PROG, in);

	ProcedureID cur = pc.tree.SetStartProcedure (GetSymbol (pc.Current ()));
	if (cur == -1) { pc.LogErrorUniqueProcedure (in, pc.Current ()); }
	pc.tree.Push (cur);
	pc.Match (TT::ID, in);

	pc.Match (TT::P_O, in);

	IdentifierList (pc, in);
	pc.Match (TT::P_C, in);
	pc.Match (TT::SEMIC, in);
	return ProgramStatementFactored (pc, in);
}

void ProgramStatement (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROG):
			prog_stmt_program (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::PROG }, { TT::END_FILE });
	}
}
void ProgramStatementFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			Declarations (pc, in);
			ProgramStatementFactoredFactored (pc, in);
			break;
		case (TT::PROC):
			SubprogramDeclarations (pc, in);
			CompoundStatement (pc, in);
			pc.Match (TT::DOT, in);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc, in);
			pc.Match (TT::DOT, in);
			break;
		default:
			DefaultErr (pc, { TT::VAR, TT::PROC, TT::BEGIN }, { TT::END_FILE });
	}
}
void ProgramStatementFactoredFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramDeclarations (pc, in);
			CompoundStatement (pc, in);
			pc.Match (TT::DOT, in);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc, in);
			pc.Match (TT::DOT, in);
			break;
		default:
			DefaultErr (pc, { TT::PROC, TT::BEGIN }, { TT::END_FILE });
	}
}

void ident_list_id (ParserContext &pc, RetType in)
{
	bool exists = pc.tree.AddVariable (GetSymbol (pc.Current ()), RT_none, true);
	if (exists) { pc.LogErrorUniqueIdentifier (in, pc.Current ()); }
	pc.Match (TT::ID, in);

	return IdentifierListPrime (pc, in);
}

void IdentifierList (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			ident_list_id (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::ID }, { TT::P_C });
	}
}

void ident_list_prime (ParserContext &pc, RetType in)
{
	pc.Match (TT::COMMA, in);
	bool exists = pc.tree.AddVariable (GetSymbol (pc.Current ()), RT_none, true);
	if (exists) { pc.LogErrorUniqueIdentifier (in, pc.Current ()); }
	pc.Match (TT::ID, in);
	return IdentifierListPrime (pc, in);
}
void IdentifierListPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			ident_list_prime (pc, in);
			break;
		case (TT::P_C):
			break;
		default:
			DefaultErr (pc, { TT::COMMA, TT::P_C }, { TT::P_C });
	}
	// e-prod
}

void decls (ParserContext &pc, RetType in)
{
	pc.Match (TT::VAR, in);
	auto tid = pc.Current ();
	pc.Match (TT::ID, in);

	pc.Match (TT::COLON, in);
	auto t = Type (pc, in);
	if (HasSymbol (tid))
	{
		auto exists = pc.tree.AddVariable (GetSymbol (tid), t, false);
		if (exists) { pc.LogErrorIdentifierScope (in, tid); }
	}

	pc.Match (TT::SEMIC, in);
	return DeclarationsPrime (pc, in);
}

void Declarations (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			decls (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::VAR }, { TT::PROC, TT::BEGIN });
	}
}

void decls_prime (ParserContext &pc, RetType in)
{
	pc.Match (TT::VAR, in);
	auto tid = pc.Current ();
	pc.Match (TT::ID, in);
	pc.Match (TT::COLON, in);
	auto tt = Type (pc, in);
	if (HasSymbol (tid))
	{
		auto exists = pc.tree.AddVariable (GetSymbol (tid), tt, false);
		if (exists) { pc.LogErrorUniqueIdentifier (in, tid); }
	}
	pc.Match (TT::SEMIC, in);
	return DeclarationsPrime (pc, in);
}

void DeclarationsPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			decls_prime (pc, in);
			break;
		case (TT::PROC):
		case (TT::BEGIN):
			break;
		default:
			DefaultErr (pc, { TT::PROC, TT::BEGIN, TT::VAR }, { TT::PROC, TT::BEGIN });
	}
	// e-prod
}

RetType type_array (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;
	pc.Match (TT::ARRAY, in);
	pc.Match (TT::B_O, in);
	auto ts = pc.Current ();
	if (std::get<NumType> (ts.attrib).val.index () != 0)
	{
		ret = RT_err;
		pc.LogErrorSem (in, "Array right bound not an int", ts);
	}
	pc.Match (TT::NUM, in);
	pc.Match (TT::DOT_DOT, in);

	auto te = pc.Current ();
	if (std::get<NumType> (te.attrib).val.index () != 0)
	{
		ret = RT_err;
		pc.LogErrorSem (in, "Array left bound not an int", te);
	}
	pc.Match (TT::NUM, in);
	pc.Match (TT::B_C, in);
	pc.Match (TT::OF, in);
	auto t = StandardType (pc, in);
	if (ret != RT_err)
	{
		int size = GetNumValInt (te) - GetNumValInt (ts) + 1;
		if (size <= 0)
		{
			pc.LogErrorSem (in, "Array bounds must be positive", te);
			return RT_err;
		}
		else if (t == RT_int)
		{
			return RetType (RT_arr_int, size);
		}
		else if (t == RT_real)
		{
			return RetType (RT_arr_real, size);
		}
	}
	return RT_err;
}
RetType Type (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ARRAY):
			return type_array (pc, in);
		case (TT::STD_T):
			return StandardType (pc, in);
		default:
			return DefaultErr (pc, { TT::ARRAY, TT::STD_T }, { TT::P_C, TT::SEMIC });
	}
}

RetType std_type (ParserContext &pc, RetType in)
{
	auto t = pc.Current ();
	pc.Match (TT::STD_T, in);
	switch (std::get<StandardTypeEnum> (t.attrib))
	{
		case (StandardTypeEnum::integer):
			return RT_int;
		case (StandardTypeEnum::real):
			return RT_real;
		default:
			pc.LogErrorSem (in,
			"Identifier \"" + pc.SymbolName (GetSymbol (pc.Current ())) + "\" not a valid type (integer or real)",
			t);
			return RT_err;
	}
}
RetType StandardType (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::STD_T):
			return std_type (pc, in);
		default:
			return DefaultErr (pc, { TT::STD_T }, { TT::P_C, TT::SEMIC });
	}
}
void SubprogramDeclarations (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramDeclaration (pc, in);
			pc.Match (TT::SEMIC, in);
			SubprogramDeclarationsPrime (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::PROC }, { TT::BEGIN });
	}
}
void SubprogramDeclarationsPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramDeclaration (pc, in);
			pc.Match (TT::SEMIC, in);
			SubprogramDeclarationsPrime (pc, in);
			break;
		case (TT::BEGIN):
			break;
		default:
			DefaultErr (pc, { TT::PROC, TT::BEGIN }, { TT::BEGIN });
	}
	// e-prod
}
void SubprogramDeclaration (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramHead (pc, in);
			SubprogramDeclarationFactored (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::PROC }, { TT::SEMIC });
	}
}
void SubprogramDeclarationFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			Declarations (pc, in);
			SubprogramDeclarationFactoredFactored (pc, in);
			break;
		case (TT::PROC):
			SubprogramDeclarations (pc, in);
			CompoundStatement (pc, in);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::VAR, TT::PROC, TT::BEGIN }, { TT::SEMIC });
	}
}
void SubprogramDeclarationFactoredFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			SubprogramDeclarations (pc, in);
			return CompoundStatement (pc, in);
			break;
		case (TT::BEGIN):
			CompoundStatement (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::PROC, TT::BEGIN }, { TT::SEMIC });
	}
}

void sub_prog_head_procedure (ParserContext &pc, RetType in)
{
	pc.Match (TT::PROC, in);
	ProcedureID cur = pc.tree.AddSubProcedure (GetSymbol (pc.Current ()));
	if (cur == -1) { pc.LogErrorUniqueProcedure (in, pc.Current ()); }
	pc.Match (TT::ID, in);
	if (cur != -1) pc.tree.Push (cur);

	return SubprogramHeadFactored (pc, in);
}

void SubprogramHead (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			sub_prog_head_procedure (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::PROC }, { TT::PROC, TT::VAR, TT::BEGIN });
	}
}
void SubprogramHeadFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::P_O):
			Arguments (pc, in);
			pc.Match (TT::SEMIC, in);
			break;
		case (TT::SEMIC):
			pc.Match (TT::SEMIC, in);
			break;
		default:
			DefaultErr (pc, { TT::SEMIC, TT::P_O }, { TT::VAR, TT::BEGIN, TT::PROC });
	}
}
RetType Arguments (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::P_O):
			pc.Match (TT::P_O, in);
			in = ParameterList (pc, in);
			pc.Match (TT::P_C, in);
			return in;
		default:
			return DefaultErr (pc, { TT::P_O }, { TT::SEMIC });
	}
}
RetType param_list_id (ParserContext &pc, RetType in)
{
	auto tid = pc.Current ();
	pc.Match (TT::ID, in);

	pc.Match (TT::COLON, in);
	auto t = Type (pc, in);
	// if (t == RT_err) { pc.LogErrorSem (in, "Parameter type cannot be an error"); }

	if (t == RT_none) { pc.LogErrorSem (in, "Parameter type cannot be none", pc.Current ()); }

	if (HasSymbol (tid))
	{
		bool exists = pc.tree.AddVariable (GetSymbol (tid), t, true);
		if (exists) { pc.LogErrorUniqueIdentifier (in, pc.Current ()); }
	}
	return ParameterListPrime (pc, in);
}

RetType ParameterList (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			return param_list_id (pc, in);
		default:
			return DefaultErr (pc, { TT::ID }, { TT::P_C });
	}
}
RetType param_list_prime_id (ParserContext &pc, RetType in)
{
	pc.Match (TT::SEMIC, in);
	auto tid = pc.Current ();
	pc.Match (TT::ID, in);
	pc.Match (TT::COLON, in);
	auto t = Type (pc, in);
	if (t == RT_err) { pc.LogErrorSem (in, "Parameter type cannot be an error", pc.Current ()); }

	if (t == RT_none) { pc.LogErrorSem (in, "Parameter type cannot be none", pc.Current ()); }
	if (HasSymbol (tid))
	{
		bool exists = pc.tree.AddVariable (GetSymbol (tid), t, true);
		if (exists) { pc.LogErrorUniqueIdentifier (in, tid); }
	}
	return ParameterListPrime (pc, in);
}
RetType ParameterListPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::SEMIC):
			return param_list_prime_id (pc, in);
		case (TT::P_C):
			return in;
		default:
			return DefaultErr (pc, { TT::SEMIC, TT::P_C }, { TT::P_C });
	}
	// e-prod
}
void CompoundStatement (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::BEGIN):
			pc.Match (TT::BEGIN, in);
			CompoundStatementFactored (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::BEGIN }, { TT::SEMIC, TT::DOT });
	}
}
void CompoundStatementFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::END):
			pc.Match (TT::END, in);
			pc.tree.Pop ();
			break;
		case (TT::BEGIN):
		case (TT::ID):
		case (TT::CALL):
		case (TT::IF):
		case (TT::WHILE):
			OptionalStatements (pc, in);
			pc.Match (TT::END, in);
			pc.tree.Pop ();
			break;

		default:
			DefaultErr (pc, { TT::END, TT::BEGIN, TT::ID, TT::CALL, TT::IF, TT::WHILE }, { TT::SEMIC, TT::DOT });
	}
}
void OptionalStatements (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::WHILE):
		case (TT::BEGIN):
		case (TT::IF):
		case (TT::CALL):
			StatementList (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::ID, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL }, { TT::END });
	}
}
void StatementList (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::WHILE):
		case (TT::BEGIN):
		case (TT::IF):
		case (TT::CALL):
			Statement (pc, in);
			StatementListPrime (pc, in);
			break;
		default:
			DefaultErr (pc, { TT::ID, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL }, { TT::END });
	}
}

void StatementListPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::SEMIC):
			pc.Match (TT::SEMIC, in);
			Statement (pc, RT_none);
			StatementListPrime (pc, RT_none);
		case (TT::END):
			break;

		default:
			DefaultErr (pc, { TT::END, TT::SEMIC }, { TT::END });
	}
	// e -prod
}

void stmt_id (ParserContext &pc, RetType in)
{
	auto ret = Variable (pc, in);
	pc.Match (TT::A_OP, in);
	auto ll = pc.Current ();
	auto eret = Expression (pc, ret);
	if (ret == RT_err || eret == RT_err) return;
	if (ret == RT_int && eret == RT_int || ret == RT_real && eret == RT_real) return;

	if (ret != eret)
		pc.LogErrorSem (in, "Cannot assign " + eret.to_string () + " to " + ret.to_string (), ll);
}

void stmt_if (ParserContext &pc, RetType in)
{
	pc.Match (TT::IF, in);
	auto eret = Expression (pc, RT_none);
	if (eret != RT_err && eret != RT_bool)
	{
		pc.LogErrorSem (in, "Conditional must use a boolean type, not " + eret.to_string (), pc.Current ());
	}
	pc.Match (TT::THEN, in);
	Statement (pc, RT_none);
	StatementFactoredElse (pc, RT_none);
}
void stmt_while (ParserContext &pc, RetType in)
{
	pc.Match (TT::WHILE, in);
	auto ret = Expression (pc, RT_none);
	if (ret != RT_bool)
	{
		pc.LogErrorSem (in, "While condition must use a boolean type, not " + ret.to_string (), pc.Current ());
	}
	pc.Match (TT::DO, in);
	Statement (pc, RT_none);
}

void Statement (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			stmt_id (pc, in);
			break;
		case (TT::WHILE):
			stmt_while (pc, in);
			break;
		case (TT::BEGIN):
			pc.Match (TT::BEGIN, in);
			// pc.tree.Push (-2);
			StatementFactoredBegin (pc, RT_none);
			break;
		case (TT::IF):
			stmt_if (pc, in);
			break;
		case (TT::CALL):
			ProcedureStatement (pc, in);
			break;
		default:
			DefaultErr (
			pc, { TT::ID, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL }, { TT::SEMIC, TT::ELSE, TT::END });
	}
}

void StatementFactoredBegin (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::END):
			pc.Match (TT::END, in);
			// pc.tree.Pop ();
			break;
		case (TT::BEGIN):
		case (TT::ID):
		case (TT::CALL):
		case (TT::IF):
		case (TT::WHILE):
			OptionalStatements (pc, in);
			pc.Match (TT::END, in);
			// pc.tree.Pop ();
			break;
		default:
			DefaultErr (pc,
			{ TT::END, TT::BEGIN, TT::ID, TT::CALL, TT::IF, TT::WHILE },
			{ TT::SEMIC, TT::ELSE, TT::END });
	}
}
void StatementFactoredElse (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ELSE):
			pc.Match (TT::ELSE, in);
			Statement (pc, in);
			break;
		case (TT::SEMIC):
		case (TT::END):
			break;
		default:
			DefaultErr (pc, { TT::END, TT::ELSE, TT::SEMIC }, { TT::SEMIC, TT::ELSE, TT::END });
	}
	// e-prod
}

RetType var_id (ParserContext &pc, RetType in)
{
	auto tid = pc.Current ();
	auto exists = pc.tree.CheckVariable (GetSymbol (tid));
	if (!exists.has_value ()) { pc.LogErrorIdentifierScope (in, tid); }

	pc.Match (TT::ID, in);
	RetType fp = RT_err;
	if (exists.has_value ()) fp = exists.value ();
	return VariableFactored (pc, fp);
}
RetType Variable (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			return var_id (pc, in);
		default:
			return DefaultErr (pc, { TT::ID }, { TT::A_OP });
	}
}
RetType var_factored_bracket_open (ParserContext &pc, RetType in)
{
	pc.Match (TT::B_O, in);
	auto rt = Expression (pc, RT_none);
	pc.Match (TT::B_C, in);
	if (in == RT_err || rt == RT_err) { return RT_err; }
	if (IsArrInt (in) && rt == RT_int) { return RT_int; }
	else if (IsArrReal (in) && rt == RT_int)
	{
		return RT_real;
	}
	else if (rt != RT_int)
	{
		pc.LogErrorSem (in, "Array index must resolve to an int, not " + rt.to_string (), pc.Current ());
	}
	else if (!IsArrInt (in) || !IsArrReal (in))
	{
		pc.LogErrorSem (in, "Cannot array index a non array type", pc.Current ());
	}
	return RT_err;
}

RetType VariableFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::B_O):
			return var_factored_bracket_open (pc, in);
		case (TT::A_OP):
			return in;
		default:
			return DefaultErr (pc, { TT::B_O, TT::A_OP }, { TT::A_OP });
	}
	// e-prod
}
RetType proc_stmt_call (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;
	pc.Match (TT::CALL, in);
	bool exists = pc.tree.CheckProcedure (GetSymbol (pc.Current ()));
	if (!exists)
	{
		ret = RT_err;
		pc.LogErrorSem (in,
		"Procedure \"" + pc.SymbolName (GetSymbol (pc.Current ())) + "\" not in current scope",
		pc.Current ());
	}
	auto tid = GetSymbol (pc.Current ());
	pc.Match (TT::ID, in);

	return ProcedureStatmentFactored (pc, tid, ret);
}
RetType ProcedureStatement (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::CALL):
			return proc_stmt_call (pc, in);

		default:
			return DefaultErr (pc, { TT::CALL }, { TT::SEMIC, TT::ELSE, TT::END });
	}
}
RetType check_param_list_with_expr_list (ParserContext &pc, SymbolID id, RetType in, std::vector<RetType> expr_list)
{
	auto param_list = pc.tree.SubProcedureType (id);
	RetType ret = RT_none;
	if (in != RT_err)
	{
		bool isEqual = true;
		for (int i = 0; i < expr_list.size (); i++)
		{
			if (param_list.size () <= i) { isEqual = false; }
			else if (param_list.at (i) != expr_list.at (i))
			{
				if (param_list.at (i) == RT_err)
					pc.LogErrorSem (in,
					"Procedures " + pc.SymbolName (id) + "'s parameter's are ill-formed. Cannot invoke an invalid procedure.",
					pc.Current ());

				else
					pc.LogErrorSem (in,
					"Argument " + std::to_string (i) + " is type " + expr_list.at (i).to_string ()
					+ " when it should be of type " + param_list.at (i).to_string (),
					pc.Current ());
				isEqual = false;
				ret = RT_err;
			}
		}
		if (param_list.size () > expr_list.size ())
		{
			ret = RT_err;
			pc.LogErrorSem (in,
			"The call to procedure \"" + pc.SymbolName (id) + "\" has "
			+ std::to_string (param_list.size () - expr_list.size ()) + " to few arguments",
			pc.Current ());
		}
		else if (param_list.size () < expr_list.size ())
		{
			ret = RT_err;
			pc.LogErrorSem (in,
			"The call to procedure \"" + pc.SymbolName (id) + "\" has "
			+ std::to_string (expr_list.size () - param_list.size ()) + " to many arguments",
			pc.Current ());
		}
		if (!isEqual) { ret = RT_err; }
	}
	return ret;
}

RetType proc_stmt_factored_paren_open (ParserContext &pc, SymbolID id, RetType in)
{
	pc.Match (TT::P_O, in);
	std::vector<RetType> expr_list;
	auto er = ExpressionList (pc, expr_list, in);
	auto cr = check_param_list_with_expr_list (pc, id, in, expr_list);
	pc.Match (TT::P_C, in);
	if (cr == RT_err || er == RT_err) return RT_err;
	return er;
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
			return check_param_list_with_expr_list (pc, id, in, {});
		default:
			return DefaultErr (pc, { TT::P_O, TT::SEMIC, TT::ELSE }, { TT::ELSE, TT::SEMIC, TT::END });
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
			return DefaultErr (pc, { TT::ID, TT::P_O, TT::NUM, TT::NOT, TT::SIGN }, { TT::P_C });
	}
}
RetType expr_list_prime_elem (ParserContext &pc, std::vector<RetType> &expr_list, RetType in)
{
	pc.Match (TT::COMMA, in);
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
			return in;
		default:
			return DefaultErr (pc, { TT::P_C, TT::COMMA }, { TT::P_C });
	}
	// e -prod
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
			in = SimpleExpression (pc, in);
			return ExpressionFactored (pc, in);
		default:
			return DefaultErr (pc,
			{ TT::ID, TT::P_O, TT::NUM, TT::NOT, TT::SIGN },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
}
RetType expr_factored_relop (ParserContext &pc, RetType in)
{
	pc.Match (TT::RELOP, in);
	auto ll = pc.Current ();
	auto ser = SimpleExpression (pc, in);
	if (in == RT_err || ser == RT_err) return RT_err;
	if ((in == RT_int && ser == RT_int) || (in == RT_real && ser == RT_real)) { return RT_bool; }
	else
	{
		pc.LogErrorSem (in, "Cannot compare types " + in.to_string () + " and " + ser.to_string (), ll);
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
			return in;
		default:
			return DefaultErr (pc,
			{ TT::RELOP, TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
	// e-prod
}

RetType SimpleExpression (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::P_O):
		case (TT::NUM):
		case (TT::NOT):
			in = Term (pc, in);
			return SimpleExpressionPrime (pc, in);
		case (TT::SIGN):
			pc.Match (TT::SIGN, in);
			in = Term (pc, in);
			if (in != RT_int && in != RT_real)
			{
				pc.LogErrorSem (in, "Cannot add a sign to a non int or real term", pc.Current ());
				in = RT_err;
			}
			return SimpleExpressionPrime (pc, in);
		default:
			return DefaultErr (pc,
			{ TT::ID, TT::P_O, TT::NUM, TT::NOT, TT::SIGN },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
}

RetType simp_expr_prime_add (ParserContext &pc, RetType in)
{
	bool isOr = pc.Current ().type == TT::ADDOP ? true : false;
	if (isOr) { pc.Match (TT::ADDOP, in); }
	else
	{
		Sign (pc, in); // match signop
	}

	auto tr = Term (pc, in);
	RetType sep = RT_none;

	if (!isOr)
	{ // + or -
		if (in == RT_int && tr == RT_int || in == RT_real && tr == RT_real) { sep = in; }
		else if (in == RT_err || tr == RT_err)
		{
			sep = RT_err;
		}
		else
		{
			pc.LogErrorSem (
			in, "Cannot add types " + in.to_string () + " and " + tr.to_string (), pc.Current ());
			sep = RT_err;
		}
	}
	else
	{ // or
		if (in == RT_bool && tr == RT_bool) { sep = in; }
		else if (in == RT_err || tr == RT_err)
		{
			sep = RT_err;
		}
		else
		{
			pc.LogErrorSem (in, "Cannot or types " + in.to_string () + " and " + tr.to_string (), pc.Current ());
			sep = RT_err;
		}
	}
	return SimpleExpressionPrime (pc, sep);
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
			return in;
		default:
			return DefaultErr (pc,
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::SIGN, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
	// e -prod
}
RetType Term (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::NUM):
		case (TT::P_O):
		case (TT::NOT):
			in = Factor (pc, in);
			return TermPrime (pc, in);
		default:
			return DefaultErr (pc,
			{ TT::ID, TT::P_O, TT::NUM, TT::NOT },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::ADDOP, TT::SIGN, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
}
RetType term_prime_mulop (ParserContext &pc, RetType in)
{
	auto mulOp = std::get<MulOpEnum> (pc.Current ().attrib);
	bool isMul = mulOp == MulOpEnum::mul || mulOp == MulOpEnum::div;
	bool isMod = mulOp == MulOpEnum::mod;
	bool isAnd = mulOp == MulOpEnum::t_and;
	pc.Match (TT::MULOP, in);
	auto fr = Factor (pc, in);
	RetType itp = RT_none;
	if (isMul)
	{
		if (in == RT_int && fr == RT_int || in == RT_real && fr == RT_real)
			itp = in;
		else if (in == RT_err || fr == RT_err)
		{
			itp = RT_err;
		}
		else
		{
			pc.LogErrorSem (
			in, "Cannot mulop between " + in.to_string () + " and " + fr.to_string (), pc.Current ());
			itp = RT_err;
		}
	}
	else if (isMod)
	{
		if (in == RT_int && fr == RT_int) { itp = RT_int; }
		else if (in == RT_err || fr == RT_err)
		{
			itp = RT_err;
		}
		else
		{
			pc.LogErrorSem (in, "Cannot perform mod on non int values", pc.Current ());
			itp = RT_err;
		}
	}
	else if (isAnd)
	{
		if (in == RT_bool && fr == RT_bool) { itp = RT_bool; }
		else if (in == RT_err || fr == RT_err)
		{
			itp = RT_err;
		}
		else
		{
			pc.LogErrorSem (in, "Cannot and non boolean expressions", pc.Current ());
			itp = RT_err;
		}
	}
	return TermPrime (pc, itp);
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
			return in;
		default:
			return DefaultErr (pc,
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::MULOP, TT::SIGN, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::SIGN, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::END, TT::DO });
	}
	// e -prod
}
RetType factor_id (ParserContext &pc, RetType in)
{
	auto tid = pc.Current ();
	auto exists = pc.tree.CheckVariable (GetSymbol (tid));
	if (!exists.has_value ()) { pc.LogErrorIdentifierScope (in, tid); }

	pc.Match (TT::ID, in);
	RetType fp = RT_err;
	if (exists.has_value ()) fp = exists.value ();
	return FactorPrime (pc, fp);
}
RetType factor_num (ParserContext &pc, RetType in)
{
	auto tid = pc.Current ();
	pc.Match (TT::NUM, in);
	if (std::get<NumType> (tid.attrib).val.index () == 0)
		return RT_int;
	else if (std::get<NumType> (tid.attrib).val.index () == 1)
		return RT_real;
	else
		return RT_err;
}
RetType factor_paren_open (ParserContext &pc, RetType in)
{
	pc.Match (TT::P_O, in);
	auto ret = Expression (pc, in);
	pc.Match (TT::P_C, in);
	return ret;
}
RetType factor_not (ParserContext &pc, RetType in)
{
	pc.Match (TT::NOT, in);
	auto ret = Factor (pc, in);
	if (ret == RT_bool) { return RT_bool; }
	else if (ret != RT_err)
	{
		pc.LogErrorSem (in, "Can only negate booleans, not " + ret.to_string () + "s", pc.Current ());
	}
	return RT_err;
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
			return DefaultErr (pc,
			{ TT::ID, TT::P_O, TT::NUM, TT::NOT },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::SIGN, TT::ADDOP, TT::MULOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
}

RetType factor_prime_braket_open (ParserContext &pc, RetType in)
{
	pc.Match (TT::B_O, in);
	auto ret = Expression (pc, in);
	pc.Match (TT::B_C, in);
	if (IsArrInt (in) && ret == RT_int) return RT_int;
	if (IsArrReal (in) && ret == RT_int) return RT_real;
	if (in == RT_err || ret == RT_err) return RT_err;

	if (ret != RT_int)
	{ pc.LogErrorSem (in, "Array index must be an int, not " + ret.to_string (), pc.Current ()); }
	else if (ret != RT_arr_int || ret != RT_arr_real)
	{
		pc.LogErrorSem (in, "Cannot array index a non array type", pc.Current ());
	}
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
		default:
			return DefaultErr (pc,
			{ TT::B_O, TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::SIGN, TT::ADDOP, TT::MULOP, TT::THEN, TT::ELSE, TT::DO, TT::END },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::SIGN, TT::ADDOP, TT::MULOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
}

RetType Sign (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::SIGN):
			pc.Match (TT::SIGN, in);
			return in;
		default:
			return DefaultErr (pc, { TT::SIGN }, { TT::ID, TT::P_O, TT::NUM, TT::NOT });
	}
}

} // namespace Parser