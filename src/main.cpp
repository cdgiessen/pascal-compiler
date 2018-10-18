
#include "lexer.h"

#include "parser.h"

class Compiler
{
	public:
	Compiler () : logger (), lexer (logger), parser (logger) {}

	void Compile (CompilationContext &context)
	{
		TokenStream ts (lexer, context);
		parser.Parse (ParserContext (ts, logger));
	}

	Logger logger;
	Lexer lexer;
	PascalParser parser;
};


int main (int argc, char *argv[])
{
	std::vector<std::string> file_list;
	file_list.push_back ("test_input/test_passing.txt");
	// file_list.push_back("test_input/test_error.txt");

	// if (argc == 2) { inFileName = std::string (argv[1]); }

	Compiler compiler;

	FileReader fileReader (compiler.logger, file_list);

	CompilationContext context (fileReader);

	compiler.Compile(context);

	return 0;
}
