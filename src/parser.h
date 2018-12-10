#pragma once

#include <cstdint>
#include <functional>
#include <variant>
#include <vector>

#include "common.h"
#include "lexer.h"

struct RetType
{
	RetType (const uint32_t cat, uint32_t size) { data = (size << 8) | (cat); }
	RetType (const uint32_t t) { data = t; }

	uint32_t data;
	operator uint32_t () const { return data; }
	std::string to_string () const
	{
		if (data == 0) return "error";
		if (data == 1) return "none";
		if (data == 2) return "bool";
		if (data == 3) return "int";
		if (data == 4) return "real";
		if ((data & 255) == 5)
		{
			int size = data >> 8;
			return "array of ints with size of " + std::to_string (size);
		}
		if ((data & 255) == 6)
		{
			int size = data >> 8;
			return "array of reals with size of " + std::to_string (size);
		}
		return "NOT A TYPE!";
	}
	uint32_t size () const
	{
		if (data == 3) return 4;
		if (data == 4) return 8;
		if ((data & 255) == 5) { return 4 * (data >> 8); }
		if ((data & 255) == 6) { return 8 * (data >> 8); }
		return 0;
	}
};

// using RetType = std::variant<Type_bool, Type_int, Type_real, Type_arr_int, Type_arr_real, Type_none, Type_err>;

using ProcedureID = int;
using SymbolID = int;

struct Procedure
{
	SymbolID name; // symbol table id;
	std::vector<std::pair<SymbolID, RetType>> params;
	std::vector<std::pair<SymbolID, RetType>> locals;
	std::vector<std::pair<SymbolID, ProcedureID>> sub_procs;
	ProcedureID parent;

	Procedure (SymbolID name, ProcedureID parent = -1) : name (name), parent (parent) {}

	void AddParam (SymbolID name, RetType type)
	{
		params.push_back (std::pair<SymbolID, RetType> (name, type));
	}

	void AddLocal (SymbolID name, RetType type)
	{
		locals.push_back (std::pair<SymbolID, RetType> (name, type));
	}

	void AddSubProc (SymbolID name, ProcedureID id)
	{
		sub_procs.push_back (std::pair<SymbolID, ProcedureID> (name, id));
	}

	std::vector<RetType> Signature ()
	{
		std::vector<RetType> out;
		for (auto [id, type] : params)
		{
			out.push_back (type);
		}
		return out;
	}
};

class ParseTree
{
	public:
	ProcedureID SetStartProcedure (SymbolID s);
	ProcedureID AddSubProcedure (SymbolID s);
	std::vector<RetType> SubProcedureType (SymbolID s);
	bool CheckProcedure (SymbolID s);
	bool AddVariable (SymbolID s, RetType type, bool isParam);
	std::optional<RetType> CheckVariable (SymbolID s);

	void Push (ProcedureID id);
	void Pop ();

	std::unordered_map<ProcedureID, Procedure> procedures;

	private:
	ProcedureID eye = 0;
	ProcedureID procIDCounter = 0;
};


class ParserContext
{
	public:
	ParserContext (CompilationContext &context, TokenStream &ts, Logger &logger);

	TokenInfo Current () const;

	void Match (TT tt, RetType &rt);
	void Synch (std::vector<TT> const &set);

	void LogErrorExpectedGot (std::vector<TT> const &types);

	void LogErrorSem (RetType in, std::string msg);

	void LogErrorUniqueProcedure (RetType in, TokenInfo t);
	void LogErrorIdentifierScope (RetType in, TokenInfo t);
	void LogErrorUniqueIdentifier (RetType in, TokenInfo t);

	void Print (OutputFileHandle &out);

	std::string SymbolName (SymbolID);

	ParseTree tree;

	private:
	CompilationContext &context;
	TokenInfo Advance ();
	Logger &logger;
	TokenStream &ts;
};

namespace Parser
{
void Parse (ParserContext &pc);

void ProgramStatement (ParserContext &pc, RetType in);
void ProgramStatementFactored (ParserContext &pc, RetType in);
void ProgramStatementFactoredFactored (ParserContext &pc, RetType in);
void IdentifierList (ParserContext &pc, RetType in);
void IdentifierListPrime (ParserContext &pc, RetType in);
void Declarations (ParserContext &pc, RetType in);
void DeclarationsPrime (ParserContext &pc, RetType in);
RetType Type (ParserContext &pc, RetType in);
RetType StandardType (ParserContext &pc, RetType in);
void SubprogramDeclarations (ParserContext &pc, RetType in);
void SubprogramDeclarationsPrime (ParserContext &pc, RetType in);
void SubprogramDeclaration (ParserContext &pc, RetType in);
void SubprogramDeclarationFactored (ParserContext &pc, RetType in);
void SubprogramDeclarationFactoredFactored (ParserContext &pc, RetType in);
void SubprogramHead (ParserContext &pc, RetType in);
void SubprogramHeadFactored (ParserContext &pc, RetType in);
RetType Arguments (ParserContext &pc, RetType in);
RetType ParameterList (ParserContext &pc, RetType in);
RetType ParameterListPrime (ParserContext &pc, RetType in);
void CompoundStatement (ParserContext &pc, RetType in);
void CompoundStatementFactored (ParserContext &pc, RetType in);
void OptionalStatements (ParserContext &pc, RetType in);
void StatementList (ParserContext &pc, RetType in);
void StatementListPrime (ParserContext &pc, RetType in);
void Statement (ParserContext &pc, RetType in);
void StatementFactoredBegin (ParserContext &pc, RetType in);
void StatementFactoredElse (ParserContext &pc, RetType in);
RetType Variable (ParserContext &pc, RetType in);
RetType VariableFactored (ParserContext &pc, RetType in);
RetType ProcedureStatement (ParserContext &pc, RetType in);
RetType ProcedureStatmentFactored (ParserContext &pc, SymbolID id, RetType in);
RetType ExpressionList (ParserContext &pc, std::vector<RetType> &expr_list, RetType in);
RetType ExpressionListPrime (ParserContext &pc, std::vector<RetType> &expr_list, RetType in);
RetType Expression (ParserContext &pc, RetType in);
RetType ExpressionFactored (ParserContext &pc, RetType in);
RetType SimpleExpression (ParserContext &pc, RetType in);
RetType SimpleExpressionPrime (ParserContext &pc, RetType in);
RetType Term (ParserContext &pc, RetType in);
RetType TermPrime (ParserContext &pc, RetType in);
RetType Factor (ParserContext &pc, RetType in);
RetType FactorPrime (ParserContext &pc, RetType in);
RetType Sign (ParserContext &pc, RetType in);
}; // namespace Parser
