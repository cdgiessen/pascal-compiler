
#include "lexer.h"

#include "parser.h"

class Compiler
{
	public:
	Compiler () : logger (), lexer (logger) {}

	void Compile (CodeSource &source)
	{
		CompilationContext context;

		TokenStream ts (lexer, context, source);

		ParserContext ct (context, ts, logger);
		Parser::Parse (ct);

		OutputFileHandle sym ("symbol_file.txt");
		context.symbolTable.Print (sym);
	}

	Logger logger;
	Lexer lexer;
};


int main (int argc, char *argv[])
{
	std::vector<std::string> file_list;
	file_list.push_back("test_input/test_passing.txt");
	//file_list.push_back ("test_input/test_some_errors.txt");
	//file_list.push_back ("test_input/test_sem.txt");

	// if (argc == 2) { inFileName = std::string (argv[1]); }

	FileReader fileReader (file_list);

	auto sc = fileReader.Read ();
	while (sc.has_value ())
	{
		Compiler compiler;

		compiler.Compile (sc.value ());
		compiler.logger.LogErrors ();
		sc = fileReader.Read ();
	}

	return 0;
}
