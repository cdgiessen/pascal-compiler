

#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <variant>
#include <optional>
#include <any>
#include <memory>
#include <unordered_set>

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
    int val = 0;
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
    int line_location;
    int column_location;
};

struct ReservedWord {
    std::string word;
    TokenType type;
    TokenAttribute attrib;

    ReservedWord(std::string word, TokenType type, TokenAttribute attrib):
        word(word), type(type), attrib(attrib) {};

    bool operator==(const ReservedWord &other) const
    { return (word == other.word
              && type == other.type);
    }
};

namespace std {
  template <>
  struct hash<ReservedWord>
  {
    size_t operator()(const ReservedWord& w) const
    {
      return hash<std::string>()(w.word);
    }
  };

}

using ProgramLine = std::array<char, line_buffer_length>;

std::vector<TokenInfo> ParseLine(ProgramLine line ){
    int cur_column = 0;

    

    return {};
}

std::optional<TokenType> ReadTokenTypeFromString(std::string s){
    if (s == "program") return TokenType::program;
    if (s == "var") return TokenType::variable;
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
    if (s == "or") return TokenType::addop;
    if (s == "div") return TokenType::mulop;
    if (s == "mod") return TokenType::mulop;
    if (s == "and") return TokenType::mulop;
    return {};
}

std::optional<TokenAttribute> ReadTokenAttributeFromString(std::string s){
    if (s == "0") return NoAttrib{0};
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

        }
    } 
    else {
        fmt::print("Reserved Word List not found!\n");
    }
    return res_words;
}

int main(int argc, char *argv[]){
    std::string inFileName = "test_pascal.txt";

    if(argc == 2){
        inFileName = std::string(argv[1]);
    }

    std::fstream inFile(inFileName);
    
    auto reserved_words = ReadReservedWordsFile();

    fmt::print("Res Word size = {}\n", reserved_words.size());
    fmt::print("Reserved Words:\n");
    for(auto& item : reserved_words){
        fmt::print("Word: {} \t\n", item.word);
    }

    return 0;
}