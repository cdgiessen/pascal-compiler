#include <vector>
#include <fmt/format.h>


constexpr int line_buffer_length = 72;
constexpr int identifier_length = 10;
constexpr int integer_digit_length = 10;

// xx.yyEzz
constexpr int real_base_length = 5;
constexpr int real_decimal_length = 5;
constexpr int real_exponent_length = 2;

int main(){

    fmt::print("Hello, {}!", "world");
    return 0;
}