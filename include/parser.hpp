#pragma once

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>
#include "ast.hpp"
#include <string>

struct Token;

using std::unique_ptr;
using std::make_unique;
using std::vector;

struct Parser {
  vector<Token> tokens;
  inline Token Peek() {
    return tokens.back();
  }
  inline Token Eat() {
    auto tkn = tokens.back();
    tokens.pop_back();
    return tkn;
  }
  inline Token Expect(const TType ttype) {
    if (tokens.back().type != ttype) {
      throw std::runtime_error("Expected " + TTypeToString(ttype) + " got " + TTypeToString(tokens.back().type));
    }
    auto tkn =tokens.back();
    tokens.pop_back();
    return tkn;
  }
  
  unique_ptr<Program> Parse(vector<Token> &&tokens) {
    std::reverse(tokens.begin(), tokens.end());
    this->tokens = std::move(tokens);
    vector<StatementPtr> statements;
    while (this->tokens.size() > 0) {
      auto statement = ParseStatement();
      statements.push_back(std::move(statement));
    }
    auto program = make_unique<Program>(std::move(statements));
    return program;
  }

  StatementPtr ParseImport();

  StatementPtr ParseLValuePostFix(ExpressionPtr &&expr);
  StatementPtr ParseFor();
  StatementPtr ParseFuncDecl();
  IfPtr ParseIf();
  ElsePtr ParseElse();
  StatementPtr ParseContinue();
  StatementPtr ParseReturn();
  StatementPtr ParseBreak();
  StatementPtr ParseIdentifierStatement(IdentifierPtr identifier);
  StatementPtr ParseAssignment(IdentifierPtr identifier);
  StatementPtr ParseCall(IdentifierPtr identifier);
  StatementPtr ParseStatement();
  StatementPtr ParseKeyword(Token keyword);
  
  ExpressionPtr ParseExpression();
  ExpressionPtr ParseLogicalOr();
  ExpressionPtr ParseLogicalAnd();
  ExpressionPtr ParseEquality();
  ExpressionPtr ParseComparison();
  ExpressionPtr ParseTerm();
  ExpressionPtr ParseFactor();
  ExpressionPtr ParsePostfix();
  BlockPtr ParseBlock();
  ParametersPtr ParseParameters();
  ArgumentsPtr ParseArguments();
  ExpressionPtr ParseUnary();
  ExpressionPtr ParseOperand();
  OperandPtr ParseArrayInitializer();
};