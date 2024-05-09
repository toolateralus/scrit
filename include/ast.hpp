#pragma once
#include "lexer.hpp"
#include <gtest/gtest.h>
#include <memory>


struct Context;
struct Value_T;
struct Scope_T;
typedef std::shared_ptr<Value_T> Value;
typedef std::shared_ptr<Scope_T> Scope;

using std::make_unique;
using std::unique_ptr;

// forawrd declarations
struct Statement;
struct Expression;
struct Block;
struct Arguments;
struct If;
struct Else;
struct Identifier;
struct Parameters;
struct Operand;
struct Import;
// typedefs
typedef unique_ptr<Import> ImportPtr;
typedef unique_ptr<Statement> StatementPtr;
typedef unique_ptr<Expression> ExpressionPtr;
typedef unique_ptr<Block> BlockPtr;
typedef unique_ptr<Arguments> ArgumentsPtr;
typedef unique_ptr<If> IfPtr;
typedef unique_ptr<Else> ElsePtr;
typedef unique_ptr<Identifier> IdentifierPtr;
typedef unique_ptr<Parameters> ParametersPtr;
typedef unique_ptr<Operand> OperandPtr;

enum struct ControlChange {
  None,
  Return,
  Continue,
  Break,
  Goto,
  ContinueLabel,
  BreakLabel,
  Exception
};
struct ExecutionResult {
  ExecutionResult(ControlChange controlChange, Value value);
  static ExecutionResult None;
  static ExecutionResult Break;
  static ExecutionResult Continue;
  ControlChange controlChange;
  Value value;
};
struct ASTNode {
  static Context context;
  virtual ~ASTNode() {}
};
struct Executable : ASTNode {
  virtual ~Executable() {}
  virtual ExecutionResult Execute() = 0;
};
struct Statement : Executable {

};
struct Program : Executable {
  vector<StatementPtr> statements;
  Program(vector<StatementPtr> &&statements);
  ExecutionResult Execute() override;
};
struct Expression {
  virtual ~Expression() {}
  virtual Value Evaluate() = 0;
};
struct Operand : Expression {
  Operand(Value value);
  Value value;
  Value Evaluate() override;
};
struct Identifier : Expression {
  Identifier(string &name);
  string name;
  Value Evaluate() override;
};
struct Arguments : Expression {
  Arguments(vector<ExpressionPtr> &&args);
  vector<ExpressionPtr> values;
  Value Evaluate() override;
};
struct Parameters : Statement {
  vector<string> names;
  Parameters(vector<string> &&names);
  ExecutionResult Execute() override;
};
struct Continue : Statement {
  ExecutionResult Execute() override;
};
struct Break : Statement {
  ExecutionResult Execute() override;
};
struct Return : Statement {
  Return(ExpressionPtr &&value);
  ExpressionPtr value;
  ExecutionResult Execute() override;
};
struct Block : Statement {
  Block(vector<StatementPtr> &&statements);
  vector<StatementPtr> statements;
  Scope scope;
  ExecutionResult Execute() override;
};
struct ObjectInitializer : Expression {
  BlockPtr block;
  Scope scope;
  ObjectInitializer(BlockPtr block);
  Value Evaluate() override;
};
struct Call : Expression, Statement {
  ExpressionPtr operand;
  ArgumentsPtr args;
  Call(ExpressionPtr &&operand, ArgumentsPtr &&args);
  static vector<Value> GetArgsValueList(ArgumentsPtr &args);
  Value Evaluate() override;
  ExecutionResult Execute() override;
};
struct If : Statement {
  static IfPtr NoElse(ExpressionPtr &&condition, BlockPtr &&block);
  static IfPtr WithElse(ExpressionPtr &&condition, BlockPtr &&block, ElsePtr &&elseStmnt);
  If(ExpressionPtr &&condition, BlockPtr &&block);
  If(ExpressionPtr &&condition, BlockPtr &&block,  ElsePtr &&elseStmnt);
  ExpressionPtr condition;
  BlockPtr block;
  ElsePtr elseStmnt;
  ExecutionResult Execute() override;
};
struct Else : Statement {
  IfPtr ifStmnt;
  BlockPtr block;
  static ElsePtr NoIf(BlockPtr &&block);
  static ElsePtr New(IfPtr &&ifStmnt);
  ExecutionResult Execute() override;
};
struct For : Statement {
  StatementPtr decl;
  ExpressionPtr condition;
  StatementPtr increment;
  BlockPtr block;
  Scope scope;
  For(StatementPtr &&decl, ExpressionPtr &&condition, StatementPtr &&inc, BlockPtr &&block, Scope scope);
  ExecutionResult Execute() override;
};

struct RangeBasedFor : Statement {
  RangeBasedFor(IdentifierPtr &&lhs, ExpressionPtr &&rhs, BlockPtr &&block);
  IdentifierPtr valueName;
  ExpressionPtr rhs;
  BlockPtr block;
  ExecutionResult Execute() override;
};

struct Assignment : Statement {
  IdentifierPtr iden;
  ExpressionPtr expr;
  Assignment(IdentifierPtr &&iden, ExpressionPtr &&expr);
  ExecutionResult Execute() override;
};

struct CompAssignExpr : Expression {
  ExpressionPtr left, right;
  TType op;
  CompAssignExpr(ExpressionPtr &&left, ExpressionPtr &&right, TType op) : left(std::move(left)), right(std::move(right)), op(op) {}
  Value Evaluate() override;
};

struct CompoundAssignment: Statement {
  ExpressionPtr expr;
  CompoundAssignment(ExpressionPtr &&expr);
  ExecutionResult Execute() override;
};
struct Noop : Statement {
  ExecutionResult Execute() override {
    return ExecutionResult::None;
  }
};
struct DotExpr : Expression {
  DotExpr(ExpressionPtr &&left, ExpressionPtr &&right);
  ExpressionPtr left;
  ExpressionPtr right;
  Value Evaluate() override;
  void Assign(Value value);
};
struct DotAssignment : Statement {
  DotAssignment(ExpressionPtr &&dot, ExpressionPtr &&value);
  ExpressionPtr dot;
  ExpressionPtr value;
  ExecutionResult Execute() override;
};
struct DotCallStmnt : Statement {
  DotCallStmnt(ExpressionPtr &&dot);
  ExpressionPtr dot;
  ExecutionResult Execute() override;
};
struct Subscript : Expression {
  Subscript(ExpressionPtr &&left, ExpressionPtr &&idx);
  ExpressionPtr left;
  ExpressionPtr index;
  Value Evaluate();
};
struct SubscriptAssignStmnt : Statement {
  SubscriptAssignStmnt(ExpressionPtr &&subscript, ExpressionPtr &&value);
  ExpressionPtr subscript;
  ExpressionPtr value;
  ExecutionResult Execute() override;
};
struct UnaryExpr : Expression {
  UnaryExpr(ExpressionPtr &&left, TType op);
  ExpressionPtr left;
  TType op;
  Value Evaluate() override;
};

// this is basically only for decrement / increment right now
struct UnaryStatement : Statement {
  ExpressionPtr expr;
  UnaryStatement(ExpressionPtr &&expr);
  ExecutionResult Execute() override {
    expr->Evaluate();
    return ExecutionResult::None;
  }
};

struct BinExpr : Expression {
  ExpressionPtr left;
  ExpressionPtr right;
  TType op;
  BinExpr(ExpressionPtr &&left, ExpressionPtr &&right, TType op);
  Value Evaluate() override;
};


struct Import : Statement {
  Import(const string &name, const bool isWildcard);
  Import(const string &name, vector<string> &symbols);
  vector<string> symbols;
  string moduleName;
  bool isWildcard;
  const string moduleRoot = "/usr/local/scrit/modules/";
  ExecutionResult Execute() override;
};

string CC_ToString(ControlChange controlChange);