#pragma once

#include <memory>
#include <stdexcept>
#include <vector>
#include "ast.hpp"
#include "lexer.hpp"
#include "value.hpp"

using std::unique_ptr;
using std::make_unique;
using std::vector;

struct Parser {
  vector<Token> tokens;
  
  Token Peek() {
    return tokens.back();
  }
  Token Eat() {
    auto tkn = tokens.back();
    tokens.pop_back();
    return tkn;
  }
  Token Expect(const TType ttype) {
    if (tokens.back().type != ttype) {
      throw std::runtime_error("Expected " + TTypeToString(ttype) + " got " + TTypeToString(tokens.back().type));
    }
    auto tkn =tokens.back();
    tokens.pop_back();
    return tkn;
  }
  
  unique_ptr<Program> Parse(vector<Token> &&tokens) {
    this->tokens = std::move(tokens);
    vector<unique_ptr<Statement>> statements;
    while (tokens.size() > 0) {
      auto statement = ParseStatement();
      statements.push_back(std::move(statement));
    }
    auto program = make_unique<Program>(std::move(statements));
    return program;
  }
  unique_ptr<Statement> ParseDeclarationOrAssignment();
  unique_ptr<Statement> ParseStatement();
  unique_ptr<Statement> ParseKeyword();
  unique_ptr<Statement> ParseIdentifierStatement();
  
  unique_ptr<Expression> ParseExpression();
  unique_ptr<Expression> ParseLogicalOr();
  unique_ptr<Expression> ParseLogicalAnd();
  unique_ptr<Expression> ParseEquality();
  unique_ptr<Expression> ParseComparison();
  unique_ptr<Expression> ParseTerm();
  unique_ptr<Expression> ParseFactor();
  unique_ptr<Expression> ParsePostfix();
  unique_ptr<Block> ParseBlock();
  unique_ptr<Parameters> ParseParameters();
  unique_ptr<Arguments> ParseArguments();
  unique_ptr<Expression> ParseUnary();
  unique_ptr<Expression> ParseOperand();
  unique_ptr<Operand> ParseArrayInitializer();
};