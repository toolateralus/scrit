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
  Token Peek(size_t lookahead = 0);
  Token Eat();
  Token Expect(const TType ttype);
  SourceInfo info = {0, 0, ""};
  unique_ptr<Program> Parse(vector<Token> &&tokens);
  BlockPtr ParseBlock();
  StatementPtr ParseAnonFuncInlineCall();
  StatementPtr ParseStatement();
  StatementPtr ParseKeyword(Token keyword);
  Type ParseFunctionType(const Type &type);
  Type ParseTupleType();
  Type ParseType();
  Type ParseTemplateType(const Type &base_type);
  
  StatementPtr ParseTupleDeconstruction(IdentifierPtr &&iden);
  
  StatementPtr ParseDeclaration();
  StatementPtr ParseDeclaration(SourceInfo &info, const string &iden, const Mutability &mut);
  
  StatementPtr ParseCall(IdentifierPtr identifier);
  StatementPtr ParseAssignment(IdentifierPtr identifier);
  StatementPtr ParseLValuePostFix(ExpressionPtr &expr);
  StatementPtr ParseIdentifierStatement(IdentifierPtr identifier);
  ExpressionPtr ParseTuple(ExpressionPtr &&expr);
  ExpressionPtr ParseLambda();
  StatementPtr ParseUsing();
  StatementPtr ParseFor();
  IfPtr ParseIf();
  ElsePtr ParseElse();
  StatementPtr ParseContinue();
  StatementPtr ParseReturn();
  StatementPtr ParseBreak();
  ExpressionPtr ParseMatch();
  StatementPtr ParseMatchStatement();

  ParametersPtr ParseParameters();
  ArgumentsPtr ParseArguments();
  
  DeletePtr ParseDelete();

  ExpressionPtr ParseExpression();
  ExpressionPtr ParseCompoundAssignment();
  ExpressionPtr ParseLogicalOr();
  ExpressionPtr ParseLogicalAnd();
  ExpressionPtr ParseEquality();
  ExpressionPtr ParseComparison();
  ExpressionPtr ParseTerm();
  ExpressionPtr ParseFactor();
  ExpressionPtr ParsePostfix();
  ExpressionPtr ParseUnary();
  ExpressionPtr ParseOperand();
  
  unique_ptr<Noop>  ParseFunctionDeclaration();
  
  ExpressionPtr ParseAnonFunc();
  ExpressionPtr ParseObjectInitializer();
  OperandPtr ParseArrayInitializer();
};