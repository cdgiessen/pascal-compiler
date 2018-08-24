

#include <array>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include <fstream>
#include <variant>
#include <optional>
#include <any>
#include <memory>
#include <unordered_set>

#include <cstdio>

#include <fmt/format.h>
#include <fmt/ostream.h>


std::FILE* listing_file_fp;
std::FILE* token_file_fp;


constexpr int line_buffer_length = 72;
constexpr int identifier_length = 10;
constexpr int integer_digit_length = 10;

// xx.yyEzz
constexpr int real_base_length = 5;
constexpr int real_decimal_length = 5;
constexpr int real_exponent_length = 2;

class AST_root {

};

enum class TokenType {
    program,
    identifier_list,
    id,
    declarations,
    type,
    standard_type,
    integer,
    real,
    subprogram_declarations,
    subprogram_declaration,
    subprogram_head,
    function,
    procedure,
    arguments,
    parameter_list,
    compound_statement,
    optional_statements,
    statement_list,
    statement,
    assignop,
    variable,
    array,
    procedure_statement,
    expression_list,
    expression,
    relop,
    simple_expression,
    addop,
    term,
    mulop,
    factor,
    sign,
    begin,
    end,
    t_not,
    of,
    t_if,
    t_then,
    t_else,
    t_while,
    t_do
};

struct NoAttrib {
};

enum class AddOpType {
    plus, //+
    minus, //-
    t_or //or
};

enum class MulOpType {
    mul, //*
    div, // div or /
    mod, // mod
    t_and //and
};

enum class SignOpType {
    plus, //+
    minus //-
};

enum class RelOpType {
    equal, //=
    not_equal, //<>
    less_than, //<
    less_than_or_equal, //<=
    greater_than, //>
    greater_than_or_equal //>=
};

using TokenAttribute = std::variant<NoAttrib,
    AddOpType,MulOpType,SignOpType,RelOpType
    >;



struct TokenInfo {
    TokenType type;
    TokenAttribute attrib;
    int line_location = -1;
    int column_location = -1;

    TokenInfo(TokenType type, TokenAttribute attrib, int line, int column): 
        type(type), attrib(attrib), line_location(line), column_location(column) {}

    friend std::ostream &operator<<(std::ostream &os, const TokenInfo &t) {
        return os << static_cast<int>(t.type) << ' ' //<< static_cast<int>(t.attrib) 
            << ' ' << t.line_location << ' ' << t.column_location;
    }
};

struct ReservedWord {
    std::string word;
    TokenType type;
    TokenAttribute attrib;

    ReservedWord(std::string word, TokenType type, TokenAttribute attrib):
        word(word), type(type), attrib(attrib) {};

    bool operator==(const ReservedWord &other) const { 
        return (word == other.word && type == other.type);
    }
};

namespace std {
  template <>
  struct hash<ReservedWord> {
    size_t operator()(const ReservedWord& w) const
    {
      return hash<std::string>()(w.word);
    }
  };
}

std::optional<TokenType> ReadTokenTypeFromString(std::string s){
    if (s == "program") return TokenType::program;
    if (s == "variable") return TokenType::variable;
    if (s == "array") return TokenType::array;
    if (s == "of") return TokenType::of;
    if (s == "integer") return TokenType::integer;
    if (s == "real") return TokenType::real;
    if (s == "function") return TokenType::function;
    if (s == "procedure") return TokenType::procedure;
    if (s == "begin") return TokenType::begin;
    if (s == "end") return TokenType::end;
    if (s == "t_if") return TokenType::t_if;
    if (s == "t_then") return TokenType::t_then;
    if (s == "t_else") return TokenType::t_else;
    if (s == "t_while") return TokenType::t_while;
    if (s == "t_do") return TokenType::t_do;
    if (s == "not") return TokenType::t_not;
    if (s == "addop") return TokenType::addop;
    if (s == "mulop") return TokenType::mulop;
    return {};
}

std::optional<TokenAttribute> ReadTokenAttributeFromString(std::string s){
    if (s == "0") return NoAttrib{};
    if (s == "div") return MulOpType::div;
    if (s == "mod") return MulOpType::mod;
    if (s == "and") return MulOpType::t_and;
    if (s == "or") return AddOpType::t_or;
    return {};
}

std::unordered_set<ReservedWord> ReadReservedWordsFile(){
    std::unordered_set<ReservedWord> res_words;
   
    std::ifstream reserved_word_file(std::string("reserved_words.txt"));
    
    if(reserved_word_file){

        while(reserved_word_file.good()){
            std::string word, type_s, attrib_s; 
            reserved_word_file >> word >> type_s >> attrib_s;
            auto type = ReadTokenTypeFromString(type_s);
            auto attrib = ReadTokenAttributeFromString(attrib_s);
            if(type.has_value() && attrib.has_value())
                res_words.emplace(word, type.value(), attrib.value());
            else 
                fmt::print("Reserved word not found! {}\n", word);

        }

        fmt::print("Res Word size = {}\n", res_words.size());
        fmt::print("Reserved Words:\n");
        for(auto& item : res_words){
            fmt::print("Word: {} \t {}\n", item.word, static_cast<int>(item.type));
    }

    } 
    else {
        fmt::print("Reserved Word List not found!\n");
    }
    return res_words;
}

enum class LexerErrorType {
    Id,
    Int,
    SReal,
    LReal,
};

enum class LexerErrorSubType {
    TooLong,
    ZeroLength,
    LeadingZero,
    TrailingZero,
};

struct LexerError {
    LexerErrorType type;
    LexerErrorSubType subType;
    int line_location = -1;
    int column_location = -1;

    LexerError(LexerErrorType type, LexerErrorSubType subType, int line, int column): 
        type(type), subType(subType), line_location(line), column_location(column) {}

    friend std::ostream &operator<<(std::ostream &os, const LexerError &t) {
        return os << static_cast<int>(t.type) << ' ' << static_cast<int>(t.subType) 
            << ' ' << t.line_location << ' ' << t.column_location;
    }
};

struct LexerMachineReturn {
    int chars_to_eat = 0;
    std::variant<std::monostate, TokenInfo, LexerError> content;

    LexerMachineReturn(int chars_to_eat, TokenInfo token ): 
        chars_to_eat(chars_to_eat), content(token){} 
    LexerMachineReturn(int chars_to_eat, LexerError error ):
        chars_to_eat(chars_to_eat), content(error){}
};


using ProgramLine = std::array<char, line_buffer_length>;

using LexerMachineFuncSig = std::function<std::optional<LexerMachineReturn>(ProgramLine& line, int index)>; 

struct LexerMachine {
    std::string name;
    int precedence = 10;
    LexerMachineFuncSig machine;
    
    LexerMachine(std::string name, int precedence, LexerMachineFuncSig machine):
        name(name), precedence(precedence), machine(machine) {};
};

class Lexer {
public:
    void AddMachine(LexerMachine&& machine);

    std::vector<TokenInfo> GetTokens(ProgramLine line);
private:
    std::vector<LexerMachine> machines;
};


void Lexer::AddMachine(LexerMachine&& machine){
    machines.push_back(std::move(machine));
    std::sort(std::begin(machines), std::end(machines), 
        [](LexerMachine a, LexerMachine b){ return a.precedence > b.precedence;});
}



std::vector<TokenInfo> Lexer::GetTokens(ProgramLine line)
{
    std::vector<TokenInfo> tokens;
    
    int backward_index = 0;
    // int forward_index = 0;
    
    while(backward_index < line_buffer_length){
        auto iter = std::begin(machines);
        std::optional<LexerMachineReturn> machine_ret;
        while(!machine_ret.has_value() && iter != std::end(machines)){
            machine_ret = iter->machine(line, backward_index);
            iter++;
        }
        if(machine_ret.has_value() && iter != std::end(machines)){
            backward_index += machine_ret->chars_to_eat;
            //if(machine_ret->content.index() == 0)
            //{
                if(machine_ret->content.index() == 1){
                    tokens.push_back(std::get<1>(machine_ret->content));
                    fmt::print(token_file_fp, "{}\n", std::get<1>(machine_ret->content));
                }
                else if (machine_ret->content.index() == 2){
                    fmt::print(listing_file_fp, "{}\n", std::get<2>(machine_ret->content));
                }
            //fmt::print(token_file_fp, "iterateion\n");
            //}
        }
    }
    return tokens;
}

class OutputFileHandle {
public:
    OutputFileHandle(std::string file_name) {
        fp = std::fopen(file_name.c_str(), "w");
        if(!fp) {
            std::perror("File opening failed");
        }
    }

    ~OutputFileHandle(){
        std::fclose(fp);
    }

    FILE* FP() const {return fp;};
private:
    FILE* fp = nullptr;
};

void CreateMachines(Lexer& lexer){
    lexer.AddMachine({"Whitespace", 100,
        [](ProgramLine& line, int index)->std::optional<LexerMachineReturn>
        {
            return LexerMachineReturn(1, TokenInfo(TokenType::term, NoAttrib{}, -1, index));
        }}
    );
    lexer.AddMachine({"IdRes", 70,
        [](ProgramLine& line, int index)->std::optional<LexerMachineReturn>{
            //stuff
        }}
    );
    lexer.AddMachine({"Catch-all", 80,
        [](ProgramLine& line, int index)->std::optional<LexerMachineReturn>{
            //stuff
        }}
    );
    lexer.AddMachine({"Real", 50,
        [](ProgramLine& line, int index)->std::optional<LexerMachineReturn>{
            //stuff
        }}
    );
    lexer.AddMachine({"Integer", 50,
        [](ProgramLine& line, int index)->std::optional<LexerMachineReturn>{
            //stuff
        }}
    );
    lexer.AddMachine({"Relop", 50,
        [](ProgramLine& line, int index)->std::optional<LexerMachineReturn>{
            //stuff
        }}
    );

}

int main(int argc, char *argv[]){
    auto reserved_words = ReadReservedWordsFile();

    std::string inFileName = "test_pascal.txt";
    
    if(argc == 2){
        inFileName = std::string(argv[1]);
    }

    OutputFileHandle listing_file("listing_file.txt");
    listing_file_fp = listing_file.FP();
    OutputFileHandle token_file("token_file.txt");
    token_file_fp = token_file.FP();


    Lexer lexer;
    CreateMachines(lexer);


    fmt::print("Prining input file\n");
    std::fstream inFile(inFileName);
    if(inFile){
        while(inFile.good()){
            std::string line;
            std::getline(inFile, line);
            fmt::print("Code: {}\n", line);

            ProgramLine l;
            for(int i = 0; i < line.length(); i++){
                l[i] = line[i];
            }

            lexer.GetTokens(l);
        
        }
    } else {
        fmt::print("File not read, was there an error?");
    }


    return 0;
}