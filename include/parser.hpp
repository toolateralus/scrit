#pragma once

#include <memory>
#include <vector>
#include "ast.hpp"
#include "lexer.hpp"

struct Token;

using std::unique_ptr;
using std::make_unique;
using std::vector;

struct Parser {
  vector<Token> tokens;
  Token Peek();
  Token Eat();
  Token Expect(const TType ttype);
  SourceInfo info;
  unique_ptr<Program> Parse(vector<Token> &&tokens);
  StatementPtr ParseUsing();
  StatementPtr ParseLValuePostFix(ExpressionPtr &expr);
  StatementPtr ParseFor();
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
  ExpressionPtr ParseCompoundAssignment();
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