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
  int loc, col;
  ASTNode(const int &loc, const int &col) : loc(loc), col(col) {}

  static Context context;
  virtual ~ASTNode() {}
};


struct Executable : ASTNode {
  virtual ~Executable() {}
  virtual ExecutionResult Execute() = 0;
  Executable(const int &loc, const int &col) : ASTNode(loc, col) {}
};
struct Statement : Executable {
  Statement(const int &loc, const int &col) : Executable(loc, col) {}
};
struct Program : Executable {
  vector<StatementPtr> statements;
  Program(vector<StatementPtr> &&statements);
  ExecutionResult Execute() override;
};
struct Expression : ASTNode {
  Expression(const int &loc, const int &col) : ASTNode(loc, col) {}
  virtual ~Expression() {}
  virtual Value Evaluate() = 0;
};
struct Operand : Expression {
  Operand(const int &loc, const int &col, Value value);
  Value value;
  Value Evaluate() override;
};
struct Identifier : Expression {
  Identifier(const int &loc, const int &col, string &name);
  string name;
  Value Evaluate() override;
};
struct Arguments : Expression {
  Arguments(const int &loc, const int &col, vector<ExpressionPtr> &&args);
  vector<ExpressionPtr> values;
  Value Evaluate() override;
};
struct Parameters : Statement {
  vector<string> names;
  Parameters(const int &loc, const int &col, vector<string> &&names);
  ExecutionResult Execute() override;
};
struct Continue : Statement {
  Continue(const int &loc, const int &col) : Statement(loc, col) {}
  ExecutionResult Execute() override;
};
struct Break : Statement {
  Break(const int &loc, const int &col) : Statement(loc, col) {}
  ExecutionResult Execute() override;
};
struct Return : Statement {

  Return(const int &loc, const int &col, ExpressionPtr &&value);
  ExpressionPtr value;
  ExecutionResult Execute() override;
};
struct Block : Statement {
  Block(const int &loc, const int &col, vector<StatementPtr> &&statements);
  vector<StatementPtr> statements;
  Scope scope;
  ExecutionResult Execute() override;
};
struct ObjectInitializer : Expression {
  BlockPtr block;
  Scope scope;
  ObjectInitializer(const int &loc, const int &col, BlockPtr block);
  Value Evaluate() override;
};
struct Call : Expression, Statement {
  ExpressionPtr operand;
  ArgumentsPtr args;
  Call(const int &loc, const int &col, ExpressionPtr &&operand,
       ArgumentsPtr &&args);
  static vector<Value> GetArgsValueList(ArgumentsPtr &args);
  Value Evaluate() override;
  ExecutionResult Execute() override;
};
struct If : Statement {
  static IfPtr NoElse(const int &loc, const int &col, ExpressionPtr &&condition,
                      BlockPtr &&block);
  static IfPtr WithElse(const int &loc, const int &col,
                        ExpressionPtr &&condition, BlockPtr &&block,
                        ElsePtr &&elseStmnt);
  If(const int &loc, const int &col, ExpressionPtr &&condition,
     BlockPtr &&block);
  If(const int &loc, const int &col, ExpressionPtr &&condition,
     BlockPtr &&block, ElsePtr &&elseStmnt);
  ExpressionPtr condition;
  BlockPtr block;
  ElsePtr elseStmnt;
  ExecutionResult Execute() override;
};
struct Else : Statement {
  IfPtr ifStmnt;
  BlockPtr block;
  Else(const int &loc, const int &col) : Statement(loc, col) {}
  static ElsePtr NoIf(const int &loc, const int &col, BlockPtr &&block);
  static ElsePtr New(const int &loc, const int &col, IfPtr &&ifStmnt);
  ExecutionResult Execute() override;
};
struct For : Statement {
  StatementPtr decl;
  ExpressionPtr condition;
  StatementPtr increment;
  BlockPtr block;
  Scope scope;
  For(const int &loc, const int &col, StatementPtr &&decl,
      ExpressionPtr &&condition, StatementPtr &&inc, BlockPtr &&block,
      Scope scope);
  ExecutionResult Execute() override;
};

struct RangeBasedFor : Statement {
  RangeBasedFor(const int &loc, const int &col, IdentifierPtr &&lhs,
                ExpressionPtr &&rhs, BlockPtr &&block);
  IdentifierPtr valueName;
  ExpressionPtr rhs;
  BlockPtr block;
  ExecutionResult Execute() override;
};

struct Assignment : Statement {
  IdentifierPtr iden;
  ExpressionPtr expr;
  Assignment(const int &loc, const int &col, IdentifierPtr &&iden,
             ExpressionPtr &&expr);
  ExecutionResult Execute() override;
};

struct CompAssignExpr : Expression {
  ExpressionPtr left, right;
  TType op;
  CompAssignExpr(const int &loc, const int &col, ExpressionPtr &&left,
                 ExpressionPtr &&right, TType op);
  Value Evaluate() override;
};

struct CompoundAssignment : Statement {
  ExpressionPtr expr;
  CompoundAssignment(const int &loc, const int &col, ExpressionPtr &&expr);
  ExecutionResult Execute() override;
};
struct Noop : Statement {
  Noop(const int &loc, const int &col) : Statement(loc, col) {}
  ExecutionResult Execute() override { return ExecutionResult::None; }
};
struct DotExpr : Expression {
  DotExpr(const int &loc, const int &col, ExpressionPtr &&left,
          ExpressionPtr &&right);
  ExpressionPtr left;
  ExpressionPtr right;
  Value Evaluate() override;
  void Assign(Value value);
};
struct DotAssignment : Statement {
  DotAssignment(const int &loc, const int &col, ExpressionPtr &&dot,
                ExpressionPtr &&value);
  ExpressionPtr dot;
  ExpressionPtr value;
  ExecutionResult Execute() override;
};
struct DotCallStmnt : Statement {
  DotCallStmnt(const int &loc, const int &col, ExpressionPtr &&dot);
  ExpressionPtr dot;
  ExecutionResult Execute() override;
};
struct Subscript : Expression {
  Subscript(const int &loc, const int &col, ExpressionPtr &&left,
            ExpressionPtr &&idx);
  ExpressionPtr left;
  ExpressionPtr index;
  Value Evaluate();
};
struct SubscriptAssignStmnt : Statement {
  SubscriptAssignStmnt(const int &loc, const int &col,
                       ExpressionPtr &&subscript, ExpressionPtr &&value);
  ExpressionPtr subscript;
  ExpressionPtr value;
  ExecutionResult Execute() override;
};
struct UnaryExpr : Expression {
  UnaryExpr(const int &loc, const int &col, ExpressionPtr &&left, TType op);
  ExpressionPtr left;
  TType op;
  Value Evaluate() override;
};

// this is basically only for decrement / increment right now
struct UnaryStatement : Statement {
  ExpressionPtr expr;
  UnaryStatement(const int &loc, const int &col, ExpressionPtr &&expr);
  ExecutionResult Execute() override {
    expr->Evaluate();
    return ExecutionResult::None;
  }
};

struct BinExpr : Expression {
  ExpressionPtr left;
  ExpressionPtr right;
  TType op;
  BinExpr(const int &loc, const int &col, ExpressionPtr &&left,
          ExpressionPtr &&right, TType op);
  Value Evaluate() override;
};

struct Import : Statement {
  Import(const int &loc, const int &col, const string &name,
         const bool isWildcard);
  Import(const int &loc, const int &col, const string &name,
         vector<string> &symbols);
  vector<string> symbols;
  string moduleName;
  bool isWildcard;
  const string moduleRoot = "/usr/local/scrit/modules/";
  ExecutionResult Execute() override;
};

string CC_ToString(ControlChange controlChange);