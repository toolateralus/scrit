#pragma once

#include <memory>
#include <vector>
#include "ast.hpp"

struct Token;

using std::unique_ptr;
using std::make_unique;
using std::vector;

struct Parser {
  vector<Token> tokens;
  Token Peek();
  Token Eat();
  Token Expect(const TType ttype);

  unique_ptr<Program> Parse(vector<Token> &&tokens);

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