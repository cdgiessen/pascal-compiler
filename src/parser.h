#pragma once

#include <cstdint>
#include <functional>
#include <variant>
#include <vector>

#include "common.h"
#include "lexer.h"

struct RetType
{
	RetType (const uint32_t cat, uint32_t size) { data = (size << 8) | static_cast<uint8_t> (cat); }
	RetType (const uint32_t t) { data = t; }

	uint32_t data;
	operator uint32_t () { return data; }
	std::string to_string ()
	{
		if (data == 0) return "error";
		if (data == 1) return "none";
		if (data == 2) return "bool";
		if (data == 3) return "int";
		if (data == 4) return "real";
		if (static_cast<uint8_t> (data) == 5)
		{
			int size = data >> 8;
			return "arr_int;size=" + std::to_string (size);
		}
		if (static_cast<uint8_t> (data) == 6)
		{
			int size = data >> 8;
			return "arr_real;size=" + std::to_string (size);
		}
		return "";
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

	private:
	ProcedureID eye = -1;
	ProcedureID procIDCounter = 0;

	std::unordered_map<ProcedureID, Procedure> procedures;
};


class ParserContext
{
	public:
	ParserContext (TokenStream &ts, Logger &logger);

	TokenInfo Current () const;

	TokenInfo Match (TT tt);
	void Synch (std::vector<TT> set);

	void LogError (int line_loc, std::string str);
	void LogErrorExpectedGot (std::vector<TT> types);

	void LogErrorProcInUse (TokenInfo ti);
	void LogErrorProcNotFound (TokenInfo ti);

	void LogErrorIdInUse (TokenInfo ti);
	void LogErrorIdNotFound (TokenInfo ti);

	void LogErrorType (TokenInfo ti, RetType expected);
	void LogErrorType (TokenInfo ti, std::vector<RetType> expecteds);


	ParseTree tree;

	private:
	TokenInfo Advance ();
	Logger &logger;
	TokenStream &ts;
};

class PascalParser
{
	public:
	PascalParser (Logger &logger);

	void Parse (ParserContext &pc);

	private:
	Logger &logger;

	RetType ProgramStatement (ParserContext &pc);
	RetType ProgramStatementFactored (ParserContext &pc);
	RetType ProgramStatementFactoredFactored (ParserContext &pc);
	RetType IdentifierList (ParserContext &pc);
	RetType IdentifierListPrime (ParserContext &pc);
	RetType Declarations (ParserContext &pc);
	RetType DeclarationsPrime (ParserContext &pc);
	RetType Type (ParserContext &pc);
	RetType StandardType (ParserContext &pc);
	RetType SubprogramDeclarations (ParserContext &pc);
	RetType SubprogramDeclarationsPrime (ParserContext &pc);
	RetType SubprogramDeclaration (ParserContext &pc);
	RetType SubprogramDeclarationFactored (ParserContext &pc);
	RetType SubprogramDeclarationFactoredFactored (ParserContext &pc);
	RetType SubprogramHead (ParserContext &pc);
	RetType SubprogramHeadFactored (ParserContext &pc);
	RetType Arguments (ParserContext &pc);
	RetType ParameterList (ParserContext &pc);
	RetType ParameterListPrime (ParserContext &pc);
	RetType CompoundStatement (ParserContext &pc);
	RetType CompoundStatementFactored (ParserContext &pc);
	RetType OptionalStatements (ParserContext &pc);
	RetType StatementList (ParserContext &pc);
	RetType StatementListPrime (ParserContext &pc);
	RetType Statement (ParserContext &pc);
	RetType StatementFactoredBegin (ParserContext &pc);
	RetType StatementFactoredElse (ParserContext &pc);
	RetType Variable (ParserContext &pc);
	RetType VariableFactored (ParserContext &pc);
	RetType ProcedureStatement (ParserContext &pc);
	RetType ProcedureStatmentFactored (ParserContext &pc);
	RetType ExpressionList (ParserContext &pc);
	RetType ExpressionListPrime (ParserContext &pc);
	RetType Expression (ParserContext &pc);
	RetType ExpressionFactored (ParserContext &pc);
	RetType SimpleExpression (ParserContext &pc);
	RetType SimpleExpressionPrime (ParserContext &pc);
	RetType Term (ParserContext &pc);
	RetType TermPrime (ParserContext &pc);
	RetType Factor (ParserContext &pc);
	RetType Sign (ParserContext &pc);
};
