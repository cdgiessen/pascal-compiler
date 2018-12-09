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


void ParserContext::LogErrorSem (RetType in, std::string msg)
{
	if (in != RT_err)
		logger.AddSemErrPrint (
		Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", "SEMERR: " + msg); });
}

void ParserContext::LogErrorUniqueProcedure (RetType in, TokenInfo t)
{
	LogErrorSem (in, "Procedure \"" + SymbolName (GetSymbol (t)) + "\" not unique in current scope");
}

void ParserContext::LogErrorIdentifierScope (RetType in, TokenInfo t)
{
	LogErrorSem (in, "Identifier \"" + SymbolName (GetSymbol (t)) + "\" not in current scope");
}

void ParserContext::LogErrorUniqueIdentifier (RetType in, TokenInfo t)
{
	LogErrorSem (in, "Identifier \"" + SymbolName (GetSymbol (t)) + "\" not unique in current scope");
}


std::string ParserContext::SymbolName (SymbolID loc)
{
	return context.symbolTable.SymbolView (loc);
}


void ParserContext::Print (OutputFileHandle &out)
{
	std::function<void(ProcedureID, int)> proc_print = [&, this](ProcedureID id, int width) {
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
	auto v = ProgramStatement (pc, RT_none);
	pc.Match (TT::END_FILE, v);
	if (v != RT_none) {}
	//}
	// catch (std::exception &err)
	//{
	//	fmt::print (std::string (err.what ()) + "\n");
	//	// found end file token, don't continue parsing
	//}
}

RetType prog_stmt_program (ParserContext &pc, RetType in)
{
	pc.Match (TT::PROG, in);

	ProcedureID cur = pc.tree.SetStartProcedure (GetSymbol (pc.Current ()));
	if (cur == -1) { pc.LogErrorUniqueProcedure (in, pc.Current ()); }
	pc.tree.Push (cur);
	pc.Match (TT::ID, in);

	pc.Match (TT::P_O, in);

	in = IdentifierList (pc, in);
	pc.Match (TT::P_C, in);
	pc.Match (TT::SEMIC, in);
	return ProgramStatementFactored (pc, in);
}

RetType ProgramStatement (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROG):
			return prog_stmt_program (pc, in);
		default:
			return DefaultErr (pc, { TT::PROG }, { TT::END_FILE });
	}
}
RetType ProgramStatementFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			in = Declarations (pc, in);
			in = ProgramStatementFactoredFactored (pc, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::PROC):
			in = SubprogramDeclarations (pc, in);
			in = CompoundStatement (pc, in);
			pc.Match (TT::DOT, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::BEGIN):
			in = CompoundStatement (pc, in);
			pc.Match (TT::DOT, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::VAR, TT::PROC, TT::BEGIN }, { TT::END_FILE });
	}
}
RetType ProgramStatementFactoredFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			in = SubprogramDeclarations (pc, in);
			in = CompoundStatement (pc, in);
			pc.Match (TT::DOT, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::BEGIN):
			in = CompoundStatement (pc, in);
			pc.Match (TT::DOT, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::PROC, TT::BEGIN }, { TT::END_FILE });
	}
}

RetType ident_list_id (ParserContext &pc, RetType in)
{
	bool exists = pc.tree.AddVariable (GetSymbol (pc.Current ()), RT_none, true);
	if (exists) { pc.LogErrorUniqueIdentifier (in, pc.Current ()); }
	pc.Match (TT::ID, in);

	return IdentifierListPrime (pc, in);
}

RetType IdentifierList (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			return ident_list_id (pc, in);
		default:
			return DefaultErr (pc, { TT::ID }, { TT::P_C });
	}
}

RetType ident_list_prime (ParserContext &pc, RetType in)
{
	pc.Match (TT::COMMA, in);
	bool exists = pc.tree.AddVariable (GetSymbol (pc.Current ()), RT_none, true);
	if (exists) { pc.LogErrorUniqueIdentifier (in, pc.Current ()); }
	pc.Match (TT::ID, in);
	return IdentifierListPrime (pc, in);
}
RetType IdentifierListPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			return ident_list_prime (pc, in);
		case (TT::P_C):
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::COMMA, TT::P_C }, { TT::P_C });
	}
	// e-prod
}

RetType decls (ParserContext &pc, RetType in)
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

RetType Declarations (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			return decls (pc, in);
		default:
			return DefaultErr (pc, { TT::VAR }, { TT::PROC, TT::BEGIN });
	}
}

RetType decls_prime (ParserContext &pc, RetType in)
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

RetType DeclarationsPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			return decls_prime (pc, in);
		case (TT::PROC):
		case (TT::BEGIN):
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::PROC, TT::BEGIN, TT::VAR }, { TT::PROC, TT::BEGIN });
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
		pc.LogErrorSem (in, "Array right bound not an int");
	}
	pc.Match (TT::NUM, in);
	pc.Match (TT::DOT_DOT, in);

	auto te = pc.Current ();
	if (std::get<NumType> (te.attrib).val.index () != 0)
	{
		ret = RT_err;
		pc.LogErrorSem (in, "Array left bound not an int");
	}
	pc.Match (TT::NUM, in);
	pc.Match (TT::B_C, in);
	pc.Match (TT::OF, in);
	auto t = StandardType (pc, in);
	if (ret != RT_err)
	{
		if (GetNumValInt (te) - GetNumValInt (ts) <= 0)
		{
			pc.LogErrorSem (in, "Array bounds must be positive");
			return RT_err;
		}
		else if (t == RT_int)
		{
			return RetType (RT_arr_int, GetNumValInt (te) - GetNumValInt (ts) + 1);
		}
		else if (t == RT_real)
		{
			return RetType (RT_arr_real, GetNumValInt (te) - GetNumValInt (ts) + 1);
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
			break;

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
			break;
		case (StandardTypeEnum::real):
			return RT_real;
			break;
		default:
			pc.LogErrorSem (in,
			"Identifier \"" + pc.SymbolName (GetSymbol (pc.Current ())) + "\" not a valid type (integer or real)");
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
RetType SubprogramDeclarations (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			in = SubprogramDeclaration (pc, in);
			pc.Match (TT::SEMIC, in);
			return SubprogramDeclarationsPrime (pc, in);
		default:
			return DefaultErr (pc, { TT::PROC }, { TT::BEGIN });
			return RT_err;
	}
}
RetType SubprogramDeclarationsPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			in = SubprogramDeclaration (pc, in);
			pc.Match (TT::SEMIC, in);
			return SubprogramDeclarationsPrime (pc, in);
		case (TT::BEGIN):
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::PROC, TT::BEGIN }, { TT::BEGIN });
	}
	// e-prod
}
RetType SubprogramDeclaration (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			in = SubprogramHead (pc, in);
			in = SubprogramDeclarationFactored (pc, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::PROC }, { TT::SEMIC });
	}
}
RetType SubprogramDeclarationFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::VAR):
			in = Declarations (pc, in);
			in = SubprogramDeclarationFactoredFactored (pc, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::PROC):
			in = SubprogramDeclarations (pc, in);
			in = CompoundStatement (pc, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::BEGIN):
			in = CompoundStatement (pc, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::VAR, TT::PROC, TT::BEGIN }, { TT::SEMIC });
	}
}
RetType SubprogramDeclarationFactoredFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			in = SubprogramDeclarations (pc, in);
			in = CompoundStatement (pc, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::BEGIN):
			in = CompoundStatement (pc, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::PROC, TT::BEGIN }, { TT::SEMIC });
	}
}

RetType sub_prog_head_procedure (ParserContext &pc, RetType in)
{
	pc.Match (TT::PROC, in);
	ProcedureID cur = pc.tree.AddSubProcedure (GetSymbol (pc.Current ()));
	if (cur == -1) { pc.LogErrorUniqueProcedure (in, pc.Current ()); }
	pc.Match (TT::ID, in);
	pc.tree.Push (cur);

	return SubprogramHeadFactored (pc, in);
}

RetType SubprogramHead (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::PROC):
			return sub_prog_head_procedure (pc, in);
		default:
			return DefaultErr (pc, { TT::PROC }, { TT::PROC, TT::VAR, TT::BEGIN });
	}
}
RetType SubprogramHeadFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::P_O):
			in = Arguments (pc, in);
			pc.Match (TT::SEMIC, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::SEMIC):
			pc.Match (TT::SEMIC, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::SEMIC, TT::P_O }, { TT::VAR, TT::BEGIN, TT::PROC });
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
	if (t == RT_err) { pc.LogErrorSem (in, "Parameter type cannot be an error"); }

	if (t == RT_none) { pc.LogErrorSem (in, "Parameter type cannot be none"); }

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
	if (t == RT_err) { pc.LogErrorSem (in, "Parameter type cannot be an error"); }

	if (t == RT_none) { pc.LogErrorSem (in, "Parameter type cannot be none"); }
	if (HasSymbol (tid))
	{
		bool exists = pc.tree.AddVariable (GetSymbol (tid), t, true);
		if (exists) { pc.LogErrorUniqueIdentifier (in, pc.Current ()); }
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
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::SEMIC, TT::P_C }, { TT::P_C });
	}
	// e-prod
}
RetType CompoundStatement (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::BEGIN):
			pc.Match (TT::BEGIN, in);
			return CompoundStatementFactored (pc, in);
		default:
			return DefaultErr (pc, { TT::BEGIN }, { TT::SEMIC, TT::DOT });
	}
}
RetType CompoundStatementFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::END):
			pc.Match (TT::END, in);
			pc.tree.Pop ();
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::BEGIN):
		case (TT::ID):
		case (TT::CALL):
		case (TT::IF):
		case (TT::WHILE):
			OptionalStatements (pc, in);
			pc.Match (TT::END, in);
			pc.tree.Pop ();
			if (in == RT_err) return RT_err;
			return RT_none;

		default:
			return DefaultErr (
			pc, { TT::END, TT::BEGIN, TT::ID, TT::CALL, TT::IF, TT::WHILE }, { TT::SEMIC, TT::DOT });
	}
}
RetType OptionalStatements (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::WHILE):
		case (TT::BEGIN):
		case (TT::IF):
		case (TT::CALL):
			return StatementList (pc, in);
		default:
			return DefaultErr (pc, { TT::ID, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL }, { TT::END });
	}
}
RetType StatementList (ParserContext &pc, RetType in)
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
			if (in == RT_err) return RT_err;
			return in;
			break;
		default:
			return DefaultErr (pc, { TT::ID, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL }, { TT::END });
	}
}
RetType StatementListPrime (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::SEMIC):
			pc.Match (TT::SEMIC, in);
			Statement (pc, in);
			StatementListPrime (pc, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::END):
			if (in == RT_err) return RT_err;
			return RT_none;

		default:
			return DefaultErr (pc, { TT::END, TT::SEMIC }, { TT::END });
	}

	// e -prod
}

RetType stmt_id (ParserContext &pc, RetType in)
{
	auto ret = Variable (pc, in);
	pc.Match (TT::A_OP, in);
	auto eret = Expression (pc, ret);
	if (ret != eret)
	{
		if (ret == RT_err)
			pc.LogErrorSem (in, "Cannot assign to an invalid array");
		else if (eret == RT_err)
			pc.LogErrorSem (in, "Cannot assign an invalid type to an array");
		else
			pc.LogErrorSem (in, "Cannot assign type " + eret.to_string () + " to type " + ret.to_string ());
		return RT_err;
	}
	return ret;
}
RetType stmt_call (ParserContext &pc, RetType in) { return ProcedureStatement (pc, in); }
RetType stmt_if (ParserContext &pc, RetType in)
{
	pc.Match (TT::IF, in);
	auto ret = Expression (pc, RT_none);
	if (ret != RT_bool)
	{
		if (ret == RT_err)
			pc.LogErrorSem (in, "Conditional is ill formed");
		else
			pc.LogErrorSem (in, "Conditional must use a boolean type, not " + ret.to_string ());
	}
	pc.Match (TT::THEN, in);
	in = Statement (pc, in);
	return StatementFactoredElse (pc, in);
} // namespace Parser
RetType stmt_while (ParserContext &pc, RetType in)
{
	pc.Match (TT::WHILE, in);
	auto ret = Expression (pc, RT_none);
	if (ret != RT_bool)
	{ pc.LogErrorSem (in, "While condition must use a boolean type, not " + ret.to_string ()); }
	pc.Match (TT::DO, in);
	return Statement (pc, in);
}
RetType stmt_begin (ParserContext &pc, RetType in)
{
	pc.Match (TT::BEGIN, in);
	return StatementFactoredBegin (pc, in);
}

RetType Statement (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ID):
			return stmt_id (pc, in);
		case (TT::WHILE):
			return stmt_while (pc, in);
		case (TT::BEGIN):
			return stmt_begin (pc, in);
		case (TT::IF):
			return stmt_if (pc, in);
		case (TT::CALL):
			return stmt_call (pc, in);
		default:
			return DefaultErr (
			pc, { TT::ID, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL }, { TT::SEMIC, TT::ELSE, TT::END });
	}
}
RetType StatementFactoredBegin (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::END):
			pc.Match (TT::END, in);
			pc.tree.Pop ();
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::BEGIN):
		case (TT::ID):
		case (TT::CALL):
		case (TT::IF):
		case (TT::WHILE):
			OptionalStatements (pc, in);
			pc.Match (TT::END, in);
			pc.tree.Pop ();
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc,
			{ TT::END, TT::BEGIN, TT::ID, TT::CALL, TT::IF, TT::WHILE },
			{ TT::SEMIC, TT::ELSE, TT::END });
	}
}
RetType StatementFactoredElse (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::ELSE):
			pc.Match (TT::ELSE, in);
			Statement (pc, in);
			if (in == RT_err) return RT_err;
			return RT_none;
		case (TT::SEMIC):
		case (TT::END):
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::END, TT::ELSE, TT::SEMIC }, { TT::SEMIC, TT::ELSE, TT::END });
	}

	// e-prod
}

RetType var_id (ParserContext &pc, RetType in)
{
	RetType ret = RT_none;
	auto exists = pc.tree.CheckVariable (GetSymbol (pc.Current ()));
	if (!exists.has_value ())
	{
		ret = RT_err;
		pc.LogErrorIdentifierScope (in, pc.Current ());

		pc.Match (TT::ID, in);
		return VariableFactored (pc, RT_err);
	}
	else
	{
		pc.Match (TT::ID, in);
		auto vr = VariableFactored (pc, exists.value ());
		if (vr == RT_none)
		{
			if (IsArrayType (exists.value ())) { return exists.value (); }
			if (exists.value () == RT_real || exists.value () == RT_int) { return exists.value (); }
			else
			{
				pc.LogErrorSem (in, "Variable can not be type" + exists.value ().to_string ());
				return RT_err;
			}
		}
		return vr;
	}
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
	if (in == RT_err) return RT_err;
	if (rt != RT_int)
	{
		if (rt == RT_err)
			pc.LogErrorSem (in, "Array index must be a valid expression");
		else
			pc.LogErrorSem (in, "Array index must be of type int, not " + rt.to_string ());

		return RT_err;
	}
	if (IsArrInt (in)) return RT_int;
	if (IsArrReal (in)) return RT_real;

	pc.LogErrorSem (in, "Array must be of type int or real, not " + in.to_string ());
	return RT_err;
}

RetType VariableFactored (ParserContext &pc, RetType in)
{
	switch (pc.Current ().type)
	{
		case (TT::B_O):
			return var_factored_bracket_open (pc, in);
		case (TT::A_OP):
			if (in == RT_err) return RT_err;
			return RT_none;
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
		pc.LogErrorSem (in, "Procedure \"" + pc.SymbolName (GetSymbol (pc.Current ())) + "\" not in current scope");
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
			return RT_err;
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
					"Procedures " + pc.SymbolName (id)
					+ "'s parameter's are ill-formed. Cannot invoke an invalid procedure.");

				else
					pc.LogErrorSem (in,
					"Argument " + std::to_string (i) + " is type " + expr_list.at (i).to_string ()
					+ " when it should be of type " + param_list.at (i).to_string ());
				isEqual = false;
				ret = RT_err;
			}
		}
		if (param_list.size () > expr_list.size ())
		{
			ret = RT_err;
			pc.LogErrorSem (in,
			"The call to procedure \"" + pc.SymbolName (id) + "\" has "
			+ std::to_string (param_list.size () - expr_list.size ()) + " to few arguments");
		}
		else if (param_list.size () < expr_list.size ())
		{
			ret = RT_err;
			pc.LogErrorSem (in,
			"The call to procedure \"" + pc.SymbolName (id) + "\" has "
			+ std::to_string (expr_list.size () - param_list.size ()) + " to many arguments");
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
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::P_C, TT::COMMA }, { TT::P_C });
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
		//	pc.LogErrorSem (in, "Can't compare types" + sr.to_string () + "and " + er.to_string
		//()); 	return RT_err; // relop in expr_factored
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
			return DefaultErr (pc,
			{ TT::ID, TT::P_O, TT::NUM, TT::NOT, TT::SIGN },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
}
RetType expr_factored_relop (ParserContext &pc, RetType in)
{
	pc.Match (TT::RELOP, in);
	auto ser = SimpleExpression (pc, in == RT_err ? RT_err : RT_bool);
	if (in == RT_err || ser == RT_err) return RT_err;
	if (in == ser) { return RT_bool; }
	else
	{
		pc.LogErrorSem (in, "Cannot compare types " + in.to_string () + " and " + ser.to_string ());
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
			if (in == RT_err) return RT_err;
			return RT_none; // means no relop
			break;
		default:
			return DefaultErr (pc,
			{ TT::RELOP, TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
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
		pc.LogErrorSem (in, "Types cannot mismatch: " + tr.to_string () + " and " + sr.to_string ());
		return RT_err;
	}
	return sr; // TODO
}
RetType simp_expr_sign (ParserContext &pc, RetType in)
{
	pc.Match (TT::SIGN, in);
	auto tr = Term (pc, in);
	auto sr = SimpleExpressionPrime (pc, tr);
	if (in == RT_err || tr == RT_err || sr == RT_err) { return RT_err; }
	if (sr == RT_none) { return tr; }
	if (tr != sr)
	{
		pc.LogErrorSem (in, "Types cannot mismatch:" + tr.to_string () + " and " + sr.to_string ());
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
			return DefaultErr (pc,
			{ TT::ID, TT::P_O, TT::NUM, TT::NOT, TT::SIGN },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
}
RetType simp_expr_prime_add (ParserContext &pc, RetType in)
{
	bool isAdd;
	switch (pc.Current ().type)
	{
		case (TT::ADDOP):
			pc.Match (TT::ADDOP, in);
			isAdd = true;
			break;
		case (TT::SIGN):
			Sign (pc, in);
			isAdd = false;
			break;
	}
	auto tr = Term (pc, in);
	auto sr = SimpleExpressionPrime (pc, tr);

	if (in == RT_err || tr == RT_err || sr == RT_err) { return RT_err; }

	if (sr == RT_none)
	{
		if (in != tr)
		{
			pc.LogErrorSem (in, "Cannot add values of type " + in.to_string () + " and " + tr.to_string ());
			return RT_err;
		}
		return tr;
	}
	if(isAdd){
		if ((tr == RT_int && in == RT_real) ||
			(tr == RT_real && in == RT_int)) {
			pc.LogErrorSem(in, "Cannot add values of type " + in.to_string() + " and " + tr.to_string());
			return RT_err;
		}
	}
	else if (!isAdd) {
		if (tr == RT_int && in == RT_int) return RT_int;
		if (tr == RT_real && in == RT_real) return RT_real;

	}
	if (in != tr)
	{
		pc.LogErrorSem (in, "Cannot add values of type " + in.to_string () + " and " + tr.to_string ());
		return RT_err;
	}
	if (tr != sr)
	{
		pc.LogErrorSem (in,
		"Types in an expression must be the same, not " + tr.to_string () + " and " + sr.to_string ());
		return RT_err;
	}
	return sr;
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
			if (in == RT_err) return RT_err;
			return RT_none; // no addop
		default:
			return DefaultErr (pc,
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::SIGN, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
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
		pc.LogErrorSem (
		in, "Terms must be be the same type, not " + fr.to_string () + " and " + tr.to_string ());
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
			return DefaultErr (pc,
			{ TT::ID, TT::P_O, TT::NUM, TT::NOT },
			{ TT::P_C, TT::SEMIC, TT::B_C, TT::COMMA, TT::RELOP, TT::ADDOP, TT::SIGN, TT::THEN, TT::ELSE, TT::DO, TT::END });
	}
}
RetType term_prime_mulop (ParserContext &pc, RetType in)
{
	auto tid = pc.Current ();
	pc.Match (TT::MULOP, in);
	auto fr = Factor (pc, in);
	auto tr = TermPrime (pc, fr);
	if (in == RT_err || fr == RT_err || tr == RT_err) return RT_err;
	if (tr == RT_none)
	{
		if (in != fr)
		{
			pc.LogErrorSem (in, "Cannot mulop between " + in.to_string () + " and " + fr.to_string ());
			return RT_err;
		}
		else
			return fr;
	}
	else if ((std::get<MulOpEnum> (tid.attrib) == MulOpEnum::mul || std::get<MulOpEnum> (tid.attrib) == MulOpEnum::div)
	         && ((fr == RT_real && tr == RT_real) || (fr == RT_int && tr == RT_int)))
	{
		return fr;
	}
	else if (std::get<MulOpEnum> (tid.attrib) == MulOpEnum::mod)
	{
		if (fr != RT_int || tr != RT_int)
		{
			pc.LogErrorSem (in, "Cannot perform mod on none int values");
			return RT_err;
		}
	}
	else if (std::get<MulOpEnum> (tid.attrib) == MulOpEnum::t_and)
	{
		if (fr != RT_bool || tr != RT_bool)
		{
			pc.LogErrorSem (in, "Cannot and non boolean expressions");
			return RT_err;
		}
	}
	else if (fr != tr)
	{

		pc.LogErrorSem (
		in, "Terms must be be the same type, not " + fr.to_string () + " and " + tr.to_string ());
		return RT_err;
	}
	return tr;
} // namespace Parser
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
			if (in == RT_err) return RT_err;
			return RT_none;
			break;
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
	pc.Match (TT::ID, in);
	if (!exists.has_value ())
	{
		if (in != RT_err) { pc.LogErrorIdentifierScope (in, tid); }


		FactorPrime (pc, RT_err);
		return RT_err;
	}
	else if (exists.value () == RT_none)
	{
		if (in != RT_err)
		{
			pc.LogErrorSem (in, std::string ("Variable ") + pc.SymbolName (GetSymbol (tid)) + " has invalid type");
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
	else if (ret == RT_err)
	{
		return RT_err;
	}
	else
	{
		if (in != RT_err)
			pc.LogErrorSem (in, "Can only negate booleans, not " + ret.to_string () + "s");
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
	if (in == RT_err || ret == RT_err) { return RT_err; }
	if (ret != RT_int)
	{
		pc.LogErrorSem (in, "Array index must be an int, not " + ret.to_string ());
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
			if (in == RT_err) return RT_err;
			return RT_none;
		default:
			return DefaultErr (pc, { TT::SIGN }, { TT::ID, TT::P_O, TT::NUM, TT::NOT });
	}
}

} // namespace Parser