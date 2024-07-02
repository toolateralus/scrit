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
  Parser() {}
  explicit Parser(vector<Token> tokens) : tokens(tokens) {}
  vector<Token> tokens;
  SourceInfo info = {0, 0, ""};
  Token Peek(size_t lookahead = 0);
  Token Eat();
  Token Expect(const TType ttype);
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
  
  unique_ptr<Call> ParseCall(IdentifierPtr identifier);
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
  
    // This riduculous function is to check if the next token is a part of an
  // expression. this is how we judge whether to parse a return expression or
  // not. We should probably make a better way to do this, just don't know how.
  static bool IsLiteralOrExpression(const Token &next) {
      return (next.family == TFamily::Keyword && next.type != TType::Null &&
              next.type != TType::Undefined && next.type != TType::False &&
              next.type != TType::True && next.type != TType::Match) ||
              (next.family == TFamily::Operator && next.type != TType::LParen &&
              next.type != TType::LCurly &&
              (next.type != TType::Sub && next.type != TType::Not));
  }

  Type ParseReturnType();

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
  unique_ptr<ObjectInitializer> ParseObjectInitializer();
  OperandPtr ParseArrayInitializer();
};