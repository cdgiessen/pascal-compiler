
#include "lexer.h"

#include "parser.h"

class Compiler
{
	public:
	Compiler () : logger (), lexer (logger), parser (logger) {}

	void Compile (CodeSource &source)
	{
		CompilationContext context;

		TokenStream ts (lexer, context, source);

		ParserContext ct (ts, logger);
		parser.Parse (ct);
	}

	Logger logger;
	Lexer lexer;
	PascalParser parser;
};


int main (int argc, char *argv[])
{
	std::vector<std::string> file_list;
	file_list.push_back ("test_input/test_some_errors.txt");
	file_list.push_back ("test_input/test_error.txt");

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
