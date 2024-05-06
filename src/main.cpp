#include "lexer.hpp"
#include <iostream>
#include <ostream>
int main () {
  Lexer lexer = {};
  
  string code = "i = 0 + 2 - 3 && 4 / v {} ()";
  
  auto tokens = lexer.Lex(code);
  
  auto str = TokensToString(tokens);
  
  std::cout << "lexed " << tokens.size() << " tokens" << "\n";
  std::cout << str << "\n";
}