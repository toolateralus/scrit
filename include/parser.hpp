#pragma once

#include <memory>
#include <stdexcept>
#include <vector>
#include "ast.hpp"

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
    vector<unique_ptr<Statement>> statements;
    while (this->tokens.size() > 0) {
      auto statement = ParseStatement();
      statements.push_back(std::move(statement));
    }
    auto program = make_unique<Program>(std::move(statements));
    return program;
  }
  unique_ptr<Statement> ParseLValuePostFix(unique_ptr<Expression> &&expr);
  unique_ptr<Statement> ParseFor();
  unique_ptr<Statement> ParseFuncDecl();
  unique_ptr<If> ParseIf();
  unique_ptr<Else> ParseElse();
  unique_ptr<Statement> ParseContinue();
  unique_ptr<Statement> ParseReturn();
  unique_ptr<Statement> ParseBreak();
  unique_ptr<Statement> ParseIdentifierStatement(unique_ptr<Identifier> identifier);
  unique_ptr<Statement> ParseAssignment(unique_ptr<Identifier> identifier);
  unique_ptr<Statement> ParseCall(unique_ptr<Identifier> identifier);
  unique_ptr<Statement> ParseStatement();
  unique_ptr<Statement> ParseKeyword(Token keyword);
  
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