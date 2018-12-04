#include "parser.h"

uint32_t RT_err = 0;
uint32_t RT_none = 1;
uint32_t RT_bool = 2;
uint32_t RT_int = 3;
uint32_t RT_real = 4;
uint32_t RT_arr_int = 5;
uint32_t RT_arr_real = 6;


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
				// error
				return -1;
			}
		}
		curProc = procedures.at (curProc).parent;
	}

	procedures.emplace (std::make_pair (procIDCounter, Procedure (newProcName, eye)));

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
	do
	{
		for (auto [id, type] : procedures.at (curProc).sub_procs)
		{
			if (id == s) { return procedures.at (curProc).Signature (); }
		}
		curProc = procedures.at (curProc).parent;
	} while (procedures.at (curProc).parent != -1);
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

ParserContext::ParserContext (TokenStream &ts, Logger &logger) : ts (ts), logger (logger) {}

TokenInfo ParserContext::Current () const { return ts.Current (); }
TokenInfo ParserContext::Advance () { return ts.Advance (); }


void ParserContext::LogError (int line_loc, std::string str)
{
	logger.AddSynErrPrint (line_loc, [=](FILE *fp) { fmt::print (fp, "{}\n", str); });
}

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

void ParserContext::LogErrorProcInUse (TokenInfo ti)
{
	using namespace std::string_literals;

	std::string out = "SEMERR: "s;
	out += "Procedure Id not unique";

	logger.AddSemErrPrint (Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", out); });
}
void ParserContext::LogErrorProcNotFound (TokenInfo ti)
{
	using namespace std::string_literals;

	std::string out = "SEMERR: "s;
	out += "Procedure Id not found";

	logger.AddSemErrPrint (Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", out); });
}

void ParserContext::LogErrorIdInUse (TokenInfo ti)
{
	using namespace std::string_literals;

	std::string out = "SEMERR: "s;
	out += "Id not unique";

	logger.AddSemErrPrint (Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", out); });
}
void ParserContext::LogErrorIdNotFound (TokenInfo ti)
{
	using namespace std::string_literals;

	std::string out = "SEMERR: "s;
	out += "Id not found";

	logger.AddSemErrPrint (Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", out); });
}

void ParserContext::LogErrorType (TokenInfo ti, RetType expected)
{
	using namespace std::string_literals;

	std::string out = "SEMERR: "s;
	out += "Expected " + expected.to_string ();

	logger.AddSemErrPrint (Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", out); });
}
void ParserContext::LogErrorType (TokenInfo ti, std::vector<RetType> expecteds)
{
	using namespace std::string_literals;

	std::string out = "SEMERR: "s;
	out += "Expected ";
	for (auto &e : expecteds)
	{
		out += e.to_string () + ", ";
	}

	logger.AddSemErrPrint (Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", out); });
}


void ParserContext::LogErrorSem (std::string msg)
{
	using namespace std::string_literals;

	std::string out = "SEMERR: "s;
	out += msg;

	logger.AddSemErrPrint (Current ().line_location, [=](FILE *fp) { fmt::print (fp, "{}\n", out); });
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
bool HasSymbol (TokenInfo t) { return std::holds_alternative<SymbolType> (t.attrib); }
int GetSymbol (TokenInfo t) { return std::get<SymbolType> (t.attrib).loc; }
int GetNumValInt (TokenInfo t) { return std::get<int> (std::get<NumType> (t.attrib).val); }
float GetNumValReal (TokenInfo t) { return std::get<float> (std::get<NumType> (t.attrib).val); }


PascalParser::PascalParser (Logger &logger) : logger (logger) {}

void PascalParser::Parse (ParserContext &pc)
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

RetType PascalParser::ProgramStatement (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PROGRAM):
			[&] {
				pc.Match (TT::PROGRAM);

				ProcedureID cur = pc.tree.SetStartProcedure (GetSymbol (pc.Current ()));
				if (cur == -1) { pc.LogErrorProcInUse (pc.Current ()); }
				pc.tree.Push (cur);
				pc.Match (TT::ID);

				pc.Match (TT::PAREN_OPEN);

				IdentifierList (pc);
				pc.Match (TT::PAREN_CLOSE);
				pc.Match (TT::SEMICOLON);
				ProgramStatementFactored (pc);
			}();
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::PROGRAM });
			pc.Synch ({ TT::END_FILE });
	}
	return ret;
}
RetType PascalParser::ProgramStatementFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
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
			pc.LogErrorExpectedGot ({ TT::VARIABLE, TT::PROCEDURE, TT::BEGIN });
			pc.Synch ({ TT::END_FILE });
	}
	return ret;
}
RetType PascalParser::ProgramStatementFactoredFactored (ParserContext &pc)
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
RetType PascalParser::IdentifierList (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			[&] {
				bool exists = pc.tree.AddVariable (GetSymbol (pc.Current ()), RT_none, true);
				if (exists) { pc.LogErrorIdInUse (pc.Current ()); }
				pc.Match (TT::ID);

				IdentifierListPrime (pc);
			}();
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID });

			pc.Synch ({ TT::PAREN_CLOSE });
	}
	return ret;
}
RetType PascalParser::IdentifierListPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			[&] {
				pc.Match (TT::COMMA);
				bool exists = pc.tree.AddVariable (GetSymbol (pc.Current ()), RT_none, true);
				if (exists) { pc.LogErrorIdInUse (pc.Current ()); }
				pc.Match (TT::ID);
				IdentifierListPrime (pc);
			}();
			break;
		case (TT::PAREN_CLOSE):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::COMMA, TT::PAREN_CLOSE });
			pc.Synch ({ TT::PAREN_CLOSE });
	}
	return ret;
	// e-prod
}
RetType PascalParser::Declarations (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
			[&] {
				pc.Match (TT::VARIABLE);
				auto exists = pc.tree.CheckVariable (GetSymbol (pc.Current ()));
				if (exists.has_value ()) { pc.LogErrorIdNotFound (pc.Current ()); }
				pc.Match (TT::ID);

				pc.Match (TT::COLON);
				Type (pc);
				pc.Match (TT::SEMICOLON);
				DeclarationsPrime (pc);
			}();
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::VARIABLE });

			pc.Synch ({ TT::PROCEDURE, TT::BEGIN });
	}
	return ret;
}
RetType PascalParser::DeclarationsPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
			[&] {
				pc.Match (TT::VARIABLE);
				auto tid = pc.Current ();
				pc.Match (TT::ID);
				pc.Match (TT::COLON);
				auto tt = Type (pc);
				auto exists = pc.tree.AddVariable (GetSymbol (tid), tt, false);
				if (exists) { pc.LogErrorIdInUse (tid); }
				pc.Match (TT::SEMICOLON);
				DeclarationsPrime (pc);
			}();
			break;
		case (TT::PROCEDURE):
		case (TT::BEGIN):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::PROCEDURE, TT::BEGIN, TT::VARIABLE });
			pc.Synch ({ TT::PROCEDURE, TT::BEGIN });
	}
	return ret;
	// e-prod
}

RetType PascalParser::Type (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ARRAY):
			[&]() -> RetType {
				pc.Match (TT::ARRAY);
				pc.Match (TT::BRACKET_OPEN);
				auto ts = pc.Current ();
				if (std::get<NumType> (ts.attrib).val.index () != 0)
				{ pc.LogErrorType (ts, RT_int); } pc.Match (TT::NUM);
				pc.Match (TT::DOT_DOT);

				auto te = pc.Current ();
				if (std::get<NumType> (te.attrib).val.index () != 0)
				{ pc.LogErrorType (te, RT_int); } pc.Match (TT::NUM);
				pc.Match (TT::BRACKET_CLOSE);
				pc.Match (TT::OF);
				auto t = StandardType (pc);
				if (t == RT_int)
				{ return RetType (RT_arr_int, GetNumValInt (te) - GetNumValInt (ts)); }
				else if (t == RT_real)
				{
					return RetType (RT_arr_real, GetNumValInt (te) - GetNumValInt (ts));
				}
				else
					return RT_err;
			}();
			break;
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
RetType PascalParser::StandardType (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::STANDARD_TYPE):
			[&] {
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
						pc.LogErrorType (t, std::vector<RetType> ({ RT_int, RT_real }));
						return RT_err;
				}
			}();
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::STANDARD_TYPE });
			pc.Synch ({ TT::PAREN_CLOSE, TT::SEMICOLON });
			return RT_err;
	}
	return ret;
}
RetType PascalParser::SubprogramDeclarations (ParserContext &pc)
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
RetType PascalParser::SubprogramDeclarationsPrime (ParserContext &pc)
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
RetType PascalParser::SubprogramDeclaration (ParserContext &pc)
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
RetType PascalParser::SubprogramDeclarationFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
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
			pc.LogErrorExpectedGot ({ TT::VARIABLE, TT::PROCEDURE, TT::BEGIN });
			pc.Synch ({ TT::SEMICOLON });
	}
	return ret;
}
RetType PascalParser::SubprogramDeclarationFactoredFactored (ParserContext &pc)
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
RetType PascalParser::SubprogramHead (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::PROCEDURE):
			[&] {
				pc.Match (TT::PROCEDURE);
				ProcedureID cur = pc.tree.AddSubProcedure (GetSymbol (pc.Current ()));
				if (cur == -1) { pc.LogErrorProcInUse (pc.Current ()); }
				pc.Match (TT::ID);
				pc.tree.Push (cur);

				SubprogramHeadFactored (pc);
			}();
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::PROCEDURE });

			pc.Synch ({ TT::PROCEDURE, TT::VARIABLE, TT::BEGIN });
	}
	return ret;
}
RetType PascalParser::SubprogramHeadFactored (ParserContext &pc)
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
			pc.Synch ({ TT::VARIABLE, TT::BEGIN, TT::PROCEDURE });
	}
	return ret;
}
RetType PascalParser::Arguments (ParserContext &pc)
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
RetType PascalParser::ParameterList (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			[&] {
				auto tid = pc.Current ();
				pc.Match (TT::ID);

				pc.Match (TT::COLON);
				auto t = Type (pc);
				if (HasSymbol (tid))
				{
					bool exists = pc.tree.AddVariable (GetSymbol (tid), t, true);
					if (exists) { pc.LogErrorIdInUse (tid); }
				}
				ParameterListPrime (pc);
			}();
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID });
			pc.Synch ({ TT::PAREN_CLOSE });
	}
	return ret;
}
RetType PascalParser::ParameterListPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::SEMICOLON):
			[&] {
				pc.Match (TT::SEMICOLON);
				auto tid = pc.Current ();
				pc.Match (TT::ID);
				pc.Match (TT::COLON);
				auto t = Type (pc);
				if (HasSymbol (tid))
				{
					bool exists = pc.tree.AddVariable (GetSymbol (tid), t, true);
					if (exists) { pc.LogErrorIdInUse (tid); }
				}
				ParameterListPrime (pc);
			}();
			break;
		case (TT::PAREN_CLOSE):
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::SEMICOLON, TT::PAREN_CLOSE });
			pc.Synch ({ TT::PAREN_CLOSE });
	}
	return ret;
	// e-prod
}
RetType PascalParser::CompoundStatement (ParserContext &pc)
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
RetType PascalParser::CompoundStatementFactored (ParserContext &pc)
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
RetType PascalParser::OptionalStatements (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
		case (TT::WHILE):
		case (TT::BEGIN):
		case (TT::IF):
		case (TT::CALL):
			StatementList (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::VARIABLE, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL });
			pc.Synch ({ TT::END });
	}
	return ret;
}
RetType PascalParser::StatementList (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::VARIABLE):
		case (TT::WHILE):
		case (TT::BEGIN):
		case (TT::IF):
		case (TT::CALL):
			Statement (pc);
			StatementListPrime (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::VARIABLE, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL });
			pc.Synch ({ TT::END });
			return RT_err;
	}
	return ret;
}
RetType PascalParser::StatementListPrime (ParserContext &pc)
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
RetType PascalParser::Statement (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			return [&]() -> RetType {
				auto ret = Variable (pc);
				pc.Match (TT::ASSIGNOP);
				auto eret = Expression (pc);

				if (ret != eret)
				{
					pc.LogErrorSem ("Var & expr have dif type yo!");
					return RT_err;
				}
				return RT_none;
			}();
			break;
		case (TT::WHILE):
			pc.Match (TT::WHILE);
			Expression (pc);
			pc.Match (TT::DO);
			Statement (pc);
			break;
		case (TT::BEGIN):
			pc.Match (TT::BEGIN);
			StatementFactoredBegin (pc);
			break;
		case (TT::IF):
			pc.Match (TT::IF);
			Expression (pc);
			pc.Match (TT::THEN);
			Statement (pc);
			StatementFactoredElse (pc);
			break;
		case (TT::CALL):
			ProcedureStatement (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::VARIABLE, TT::WHILE, TT::BEGIN, TT::IF, TT::CALL });
			pc.Synch ({ TT::SEMICOLON, TT::ELSE, TT::END });
			return RT_err;
	}
	return ret;
}
RetType PascalParser::StatementFactoredBegin (ParserContext &pc)
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
RetType PascalParser::StatementFactoredElse (ParserContext &pc)
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
RetType PascalParser::Variable (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			[&]() -> RetType {
				auto exists = pc.tree.CheckVariable (GetSymbol (pc.Current ()));
				if (!exists.has_value ()) { pc.LogErrorIdNotFound (pc.Current ()); }
				pc.Match (TT::ID);
				auto ret = VariableFactored (pc);
				if (ret == RT_int) {}
				return RT_none;
			}();
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID });
			pc.Synch ({ TT::ASSIGNOP });
			return RT_err;
	}
	return ret;
}
RetType PascalParser::VariableFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::BRACKET_OPEN):
			[&]() -> RetType {
				pc.Match (TT::BRACKET_OPEN);
				auto ret = Expression (pc);
				pc.Match (TT::BRACKET_CLOSE);
				if (ret != RT_int)
				{
					pc.LogErrorSem ("Array index not an int yo!");
					return RT_err;
				}
				return RT_int;
			}();
			break;
		case (TT::ASSIGNOP):
			return RT_none;
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::BRACKET_OPEN, TT::ASSIGNOP });
			pc.Synch ({ TT::ASSIGNOP });
			return RT_err;
	}
	return ret;
	// e-prod
}
RetType PascalParser::ProcedureStatement (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::CALL):
			[&] {
				pc.Match (TT::CALL);
				bool exists = pc.tree.CheckProcedure (GetSymbol (pc.Current ()));
				if (exists) { pc.LogErrorIdInUse (pc.Current ()); }
				pc.Match (TT::ID);
				ProcedureStatmentFactored (pc);
			}();
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::CALL });
			pc.Synch ({ TT::SEMICOLON, TT::ELSE, TT::END });
			return RT_err;
	}
	return ret;
}
RetType PascalParser::ProcedureStatmentFactored (ParserContext &pc)
{
	RetType ret = RT_none;
	switch (pc.Current ().type)
	{
		case (TT::PAREN_OPEN):
			[&]() -> RetType {
				pc.Match (TT::PAREN_OPEN);
				auto ret = ExpressionList (pc);
				pc.Match (TT::PAREN_CLOSE);
				return ret;
			}();
			break;
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
RetType PascalParser::ExpressionList (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::PAREN_OPEN):
		case (TT::NUM):
		case (TT::NOT):
		case (TT::SIGN):
			Expression (pc);
			ExpressionListPrime (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT, TT::SIGN });
			pc.Synch ({ TT::PAREN_CLOSE });
			return RT_err;
	}
	return ret;
}
RetType PascalParser::ExpressionListPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::COMMA):
			pc.Match (TT::COMMA);
			Expression (pc);
			ExpressionListPrime (pc);
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
RetType PascalParser::Expression (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::PAREN_OPEN):
		case (TT::NUM):
		case (TT::NOT):
		case (TT::SIGN):
			SimpleExpression (pc);
			ExpressionFactored (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT, TT::SIGN });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	return ret;
}
RetType PascalParser::ExpressionFactored (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::RELOP):
			pc.Match (TT::RELOP);
			SimpleExpression (pc);
			break;
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
			pc.LogErrorExpectedGot (
			{ TT::RELOP, TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	return ret;
	// e-prod
}
RetType PascalParser::SimpleExpression (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::PAREN_OPEN):
		case (TT::NUM):
		case (TT::NOT):
			Term (pc);
			SimpleExpressionPrime (pc);
			break;
		case (TT::SIGN):
			pc.Match (TT::SIGN);
			Term (pc);
			SimpleExpressionPrime (pc);

			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT, TT::SIGN });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	return ret;
}
RetType PascalParser::SimpleExpressionPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ADDOP):
			pc.Match (TT::ADDOP);
			Term (pc);
			SimpleExpressionPrime (pc);
			break;
		case (TT::PAREN_CLOSE):
		case (TT::SEMICOLON):
		case (TT::BRACKET_CLOSE):
		case (TT::COMMA):
		case (TT::RELOP):
		case (TT::THEN):
		case (TT::ELSE):
		case (TT::DO):
		case (TT::END):
			break;
		default:
			pc.LogErrorExpectedGot (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::RELOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	// e -prod
	return ret;
}
RetType PascalParser::Term (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
		case (TT::NUM):
		case (TT::PAREN_OPEN):
		case (TT::NOT):
			Factor (pc);
			TermPrime (pc);
			break;

		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::RELOP, TT::ADDOP, TT::THEN, TT::ELSE, TT::DO, TT::END });
			return RT_err;
	}
	return ret;
}
RetType PascalParser::TermPrime (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::MULOP):
			pc.Match (TT::MULOP);
			Factor (pc);
			TermPrime (pc);
			break;
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
			TT::ADDOP,
			TT::RELOP,
			TT::THEN,
			TT::ELSE,
			TT::DO,
			TT::END });
			pc.Synch (
			{ TT::PAREN_CLOSE, TT::SEMICOLON, TT::BRACKET_CLOSE, TT::COMMA, TT::ADDOP, TT::RELOP, TT::THEN, TT::ELSE, TT::END, TT::DO });
			return RT_err;
	}
	return ret;
	// e -prod
}
RetType PascalParser::Factor (ParserContext &pc)
{
	RetType ret = RT_none;

	switch (pc.Current ().type)
	{
		case (TT::ID):
			return [&]() -> RetType {
				auto exists = pc.tree.CheckVariable (GetSymbol (pc.Current ()));
				if (!exists.has_value ())
				{
					pc.LogErrorIdNotFound (pc.Current ());
					pc.Match (TT::ID);
					return RT_err;
				}
				pc.Match (TT::ID);
				return exists.value ();
			}();
			break;
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
			Expression (pc);
			pc.Match (TT::PAREN_CLOSE);
			break;
		case (TT::NOT):
			pc.Match (TT::NOT);
			Factor (pc);
			break;
		default:
			pc.LogErrorExpectedGot ({ TT::ID, TT::PAREN_OPEN, TT::NUM, TT::NOT });
			pc.Synch ({ TT::PAREN_CLOSE,
			TT::SEMICOLON,
			TT::BRACKET_CLOSE,
			TT::COMMA,
			TT::RELOP,
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
RetType PascalParser::Sign (ParserContext &pc)
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
