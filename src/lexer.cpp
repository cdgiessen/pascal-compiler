

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
    if (s == "if") return TokenType::t_if;
    if (s == "then") return TokenType::t_then;
    if (s == "else") return TokenType::t_else;
    if (s == "while") return TokenType::t_while;
    if (s == "do") return TokenType::t_do;
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
};

struct LexerMachineReturn {
    int chars_to_eat = 0;
    std::variant<TokenInfo, LexerError> content;
};


using ProgramLine = std::array<char, line_buffer_length>;

using LexerMachineFuncSig = std::function<std::optional<LexerMachineReturn>(ProgramLine& line, int index)>; 
struct LexerMachine {
    int precedence = 10;

    LexerMachineFuncSig machine;
    
    LexerMachine(int precedence, LexerMachineFuncSig machine):
        precedence(precedence), machine(machine) {};
};

class Lexer {
public:
    void AddMachine(LexerMachine machine);

    std::vector<TokenInfo> GetTokens(ProgramLine line);
private:
    std::vector<LexerMachine> machines;
};


void Lexer::AddMachine(LexerMachine machine){
    machines.push_back(machine);
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
        while(!machine.has_value() && iter != std::end(machines)){
            val = iter->machine(line, backward_index);
            iter++;
        }
        if(machine_ret.has_value() && iter != std::end(machines)){
            backward_index += machine_ret->chars_to_eat;
            if(machine_ret->content.has_value())
            {
                if(machine_ret->content.index() == 0){
                    tokens.push_back(std::get<0>(machine_ret->content));
                    fmt::print();
                }
                else if (machine_ret->content.index() == 1){
                    
                }
            }
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
            return EXIT_FAILURE;
        }
        return fp;
    }

    ~OutputFileHandle(std::FILE* fp){
        std::fclose(fp);
    }

    FILE* FP() {return fp;} const;
private:
    FILE* fp = nullptr;
}

int main(int argc, char *argv[]){
    auto reserved_words = ReadReservedWordsFile();

    std::string inFileName = "test_pascal.txt";
    
    if(argc == 2){
        inFileName = std::string(argv[1]);
    }

    std::fstream inFile(inFileName);
    if(inFile){
        while(inFile.good()){
            std::string line;
            inFile >> line;
            //needs whole line...
            fmt::print("{}\n", line);
        }
    } else {
        fmt::print("File not read, was there an error?");
    }

    OutputFileHandle listing_file("listing_file.txt");
    OutputFileHandle token_file("token_file.txt");

    return 0;
}