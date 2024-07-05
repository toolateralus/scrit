#pragma once

#include "ast.hpp"
#include "lexer.hpp"
#include <memory>
#include <vector>

struct Token;

using std::make_unique;
using std::unique_ptr;
using std::vector;

struct Parser final {
  Parser(const Parser &) = default;
  Parser(Parser &&) = default;
  Parser &operator=(const Parser &) = default;
  Parser &operator=(Parser &&) = default;
  Parser() {}
  ~Parser() {}
  explicit Parser(vector<Token> tokens) : tokens(tokens) {}
  vector<Token> tokens;
  SourceInfo info = SourceInfo(0, 0, "");
  
  Token Peek(size_t lookahead = 0);
  Token Expect(const TType ttype);
  
  Token Eat() noexcept;
  unique_ptr<Program> Parse(vector<Token> &&tokens);
  BlockPtr ParseBlock();
  BlockPtr ParseBlock(ParametersPtr &params, TypeParamsPtr &&type_params = nullptr);
  unique_ptr<StructDeclaration> ParseStructDeclaration();
  StatementPtr ParseAnonFuncInlineCall();
  StatementPtr ParseStatement();
  StatementPtr ParseKeyword(Token keyword);
  Type ParseFunctionType();
  Type ParseTupleType();
  Type ParseType();
  Type ParseTemplateType(const Type &base_type);
  TypeArgsPtr ParseTypeArgs();

  StatementPtr ParseTupleDeconstruction(IdentifierPtr &&iden);

  unique_ptr<ScopeResolution> ParseScopeResolution();

  StatementPtr ParseDeclaration();
  StatementPtr ParseDeclaration(SourceInfo &info, const string &iden,
                                const Mutability &mut);

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
  TypeParamsPtr ParseTypeParameters();
  ArgumentsPtr ParseArguments();

  DeletePtr ParseDelete();
  
// This function checks if the next token is part of an expression to decide
// whether to parse a return expression or not. A better implementation is needed.
static bool IsLiteralOrExpression(const Token &next) {
  const auto literal = next.family == TFamily::Literal;
  const auto literal_kw = next.family == TFamily::Keyword && (next.type == TType::False || next.type == TType::True || next.type == TType::Null);
  const auto iden = next.family == TFamily::Identifier;
  const auto valid_op = next.type == TType::LCurly || next.type == TType::LBrace;
  return literal || literal_kw || iden || valid_op;
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
  unique_ptr<FunctionDecl> ParseFunctionDeclaration();

  ExpressionPtr ParseAnonFunc();
  unique_ptr<ObjectInitializer> ParseObjectInitializer();
  OperandPtr ParseArrayInitializer();
};