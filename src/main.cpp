#include "context.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"
#include <fstream>
#include <iostream>
#include <memory>

int main(int argc, char **argv) {
  Lexer lexer = {};
  Parser parser = {};

  if (argc > 1) {
    std::string filename = argv[1];
    std::ifstream file(filename);
    if (file.is_open()) {
      std::stringstream buffer;
      buffer << file.rdbuf();
      std::string code = buffer.str();
      file.close();

      auto tokens = lexer.Lex(code);
      auto ast = parser.Parse(std::move(tokens));

      // create an 'args' array in language.      
      if (argc > 2) {
        Array args = std::make_shared<Array_T>(vector<Value>());
        for (int i = 2; i < argc; ++i) {
          auto str = string(argv[i]);
          args->Push(make_shared<String_T>(str));
        }
        ASTNode::context.Insert("args", args);
      }

      if (ast) {
        ast->Execute();
      } else {
        std::cout << "Parsing failed\n";
      }
    } else {
      std::cout << "Failed to open file\n";
    }
  }
}
