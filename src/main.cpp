#include "context.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

void InsertCmdLineArgs(int argc, char **argv) {
  // create an 'args' array in language.
  Array args = Array_T::New();
  if (argc > 2) {
    for (int i = 2; i < argc; ++i) {
      auto str = string(argv[i]);
      static const string breakpointKey = "breakpoint:";
      if (str.length() > breakpointKey.length() &&
          str.substr(0, breakpointKey.length()) == breakpointKey) {
        string num = "";
        for (int i = breakpointKey.length(); i < str.length(); ++i) {
          num += str[i];
        }
        int index = std::stoi(num);
        Debug::InsertBreakpoint(index, false);
      } else {
        args->Push(String_T::New(str));
      }
    }
  }
  ASTNode::context.Insert("args", args);
}

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

      InsertCmdLineArgs(argc, argv);

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
