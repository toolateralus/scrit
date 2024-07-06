#include "ast.hpp"
#include "ast_serializer.hpp"
#include "context.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "type.hpp"
#include "value.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

std::string logDir = "log";

void InsertCmdLineArgs(int argc, char **argv) {
  // create an 'args' array in language.
  Array args = Array_T::New();
  if (argc > 2) {
    for (size_t i = 2; i < (size_t)argc; ++i) {
      auto str = string(argv[i]);
      static const string breakpointKey = "br:";
      if (str.length() > breakpointKey.length() &&
          str.substr(0, breakpointKey.length()) == breakpointKey) {
        string num = "";
        for (size_t j = breakpointKey.length(); j < str.length(); ++j) {
          num += str[j];
        }
        int index = std::stoi(num);
        Debug::InsertBreakpoint(index, false);
      } else {
        args->Push(String_T::New(str));
      }
    }
  }
  ASTNode::context.CurrentScope()->Declare("args", args, Mutability::Const);
}

static std::vector<Token> &PreProcessUseStatements(std::vector<Token> &tokens) {
  size_t i = 0;
  while (i < tokens.size()) {
    const auto &tok = tokens[i];
    if (tok.type == TType::Import) {
      if (i + 1 < tokens.size()) {
        tokens.erase(tokens.begin() + i);       // remove 'use' token
        std::string filePath = tokens[i].value; // get file path token
        tokens.erase(tokens.begin() + i);       // remove file path token
        std::ifstream file(filePath);
        if (file.is_open()) {
          std::stringstream buffer;
          buffer << file.rdbuf();
          std::string code = buffer.str();
          file.close();
          Lexer lexer;
          auto includedTokens = lexer.Lex(code);
          includedTokens = PreProcessUseStatements(includedTokens);
          tokens.insert(tokens.begin() + i, includedTokens.begin(),
                        includedTokens.end());
        } else {
          std::cout << "Failed to open file: " << filePath << "\n";
        }
      }
    } else {
      i++;
    }
  }
  return tokens;
}

void serialize_ast(unique_ptr<Program> &ast) {
  auto serializer = ASTSerializer();
  ast->Accept(&serializer);

  // Check if the log directory exists, create it if it doesn't
  if (!std::filesystem::exists(logDir)) {
    std::filesystem::create_directory(logDir);
  }
  std::fstream out("log/ast.txt", std::ios::out);
  if (out.is_open()) {
    out << serializer.stream.str();
    out.close();
  } else {
    std::cout << "Failed to open file: ast.txt\n";
  }
}
int main(int argc, char **argv) {
  Lexer lexer;
  Parser parser;

  if (argc > 1) {
    std::string filename = argv[1];
    std::ifstream file(filename);
    if (file.is_open()) {
      std::stringstream buffer;
      buffer << file.rdbuf();
      std::string code = buffer.str();
      file.close();

      auto tokens = lexer.Lex(code);

      tokens = PreProcessUseStatements(tokens);

      auto ast = parser.Parse(std::move(tokens));

      InsertCmdLineArgs(argc, argv);

      if (ast) {
        
        //serialize_ast(ast);
        
        ast->Execute();
      } else {
        std::cout << "Parsing failed\n";
      }
    } else {
      std::cout << "Failed to open file: " << filename << "\n";
    }
  }
  TypeSystem::Current().global_types.clear();
}
