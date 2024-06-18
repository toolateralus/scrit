#pragma once
#include "lexer.hpp"
#include <gtest/gtest.h>
#include <memory>

using std::vector;
using std::string;


enum struct TType;
struct Context;

namespace Values {
  struct Value_T;
}
struct Scope_T;
typedef std::shared_ptr<Values::Value_T> Value;
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
struct Using;
// typedefs
typedef unique_ptr<Using> UsingPtr;
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
  SourceInfo srcInfo;
  ASTNode(SourceInfo &info) : srcInfo(info) {}

  static Context context;
  virtual ~ASTNode() {}
};


struct Executable : ASTNode {
  virtual ~Executable() {}
  virtual ExecutionResult Execute() = 0;
  Executable(SourceInfo &info) : ASTNode(info) {}
};
struct Statement : Executable {
  Statement(SourceInfo &info) : Executable(info) {}
};
struct Program : Executable {
  vector<StatementPtr> statements;
  Program(vector<StatementPtr> &&statements);
  ExecutionResult Execute() override;
};
struct Expression : ASTNode {
  Expression(SourceInfo &info) : ASTNode(info) {}
  virtual ~Expression() {}
  virtual Value Evaluate() = 0;
};
struct Operand : Expression {
  Operand(SourceInfo &info, Value value);
  Value value;
  Value Evaluate() override;
};
struct Identifier : Expression {
  Identifier(SourceInfo &info, string &name);
  string name;
  Value Evaluate() override;
};
struct Arguments : Expression {
  Arguments(SourceInfo &info, vector<ExpressionPtr> &&args);
  vector<ExpressionPtr> values;
  Value Evaluate() override;
};
struct Parameters : Statement {
  std::map<string, Value> map;
  Parameters(SourceInfo &info, std::map<string, Value> &&params);
  ExecutionResult Execute() override;
};
struct Continue : Statement {
  Continue(SourceInfo &info) : Statement(info) {}
  ExecutionResult Execute() override;
};
struct Break : Statement {
  Break(SourceInfo &info) : Statement(info) {}
  ExecutionResult Execute() override;
};
struct Return : Statement {

  Return(SourceInfo &info, ExpressionPtr &&value);
  ExpressionPtr value;
  ExecutionResult Execute() override;
};
struct Block : Statement {
  Block(SourceInfo &info, vector<StatementPtr> &&statements);
  vector<StatementPtr> statements;
  Scope scope;
  ExecutionResult Execute() override;
};
struct ObjectInitializer : Expression {
  BlockPtr block;
  Scope scope;
  ObjectInitializer(SourceInfo &info, BlockPtr &&block, Scope scope);
  Value Evaluate() override;
};
struct Call : Expression, Statement {
  ExpressionPtr operand;
  ArgumentsPtr args;
  Call(SourceInfo &info, ExpressionPtr &&operand,
       ArgumentsPtr &&args);
  static vector<Value> GetArgsValueList(ArgumentsPtr &args);
  Value Evaluate() override;
  ExecutionResult Execute() override;
};

struct If : Statement {
  If() = delete;
  static IfPtr NoElse(SourceInfo &info, ExpressionPtr &&condition,
                      BlockPtr &&block);
  static IfPtr WithElse(SourceInfo &info,
                        ExpressionPtr &&condition, BlockPtr &&block,
                        ElsePtr &&elseStmnt);
  If(SourceInfo &info, ExpressionPtr &&condition,
     BlockPtr &&block);
  If(SourceInfo &info, ExpressionPtr &&condition,
     BlockPtr &&block, ElsePtr &&elseStmnt);
  ExpressionPtr condition;
  BlockPtr block;
  ElsePtr elseStmnt;
  ExecutionResult Execute() override;
  ~If();
};
struct Else : Statement {
  ~Else();
  IfPtr ifStmnt;
  BlockPtr block;
  Else(SourceInfo &info) : Statement(info) {}
  Else(SourceInfo &info, IfPtr &&ifPtr, BlockPtr &&block);
  static ElsePtr NoIf(SourceInfo &info, BlockPtr &&block);
  static ElsePtr New(SourceInfo &info, IfPtr &&ifStmnt);
  ExecutionResult Execute() override;
};
struct For : Statement {
  StatementPtr decl;
  ExpressionPtr condition;
  StatementPtr increment;
  BlockPtr block;
  Scope scope;
  For(SourceInfo &info, StatementPtr &&decl,
      ExpressionPtr &&condition, StatementPtr &&inc, BlockPtr &&block,
      Scope scope);
  ExecutionResult Execute() override;
};

struct RangeBasedFor : Statement {
  RangeBasedFor(SourceInfo &info, IdentifierPtr &&lhs,
                ExpressionPtr &&rhs, BlockPtr &&block);
  IdentifierPtr valueName;
  ExpressionPtr rhs;
  BlockPtr block;
  ExecutionResult Execute() override;
};

struct Assignment : Statement {
  IdentifierPtr iden;
  ExpressionPtr expr;
  Assignment(SourceInfo &info, IdentifierPtr &&iden,
             ExpressionPtr &&expr);
  ExecutionResult Execute() override;
};

struct CompAssignExpr : Expression {
  ExpressionPtr left, right;
  TType op;
  CompAssignExpr(SourceInfo &info, ExpressionPtr &&left,
                 ExpressionPtr &&right, TType op);
  Value Evaluate() override;
};

struct CompoundAssignment : Statement {
  ExpressionPtr expr;
  CompoundAssignment(SourceInfo &info, ExpressionPtr &&expr);
  ExecutionResult Execute() override;
};
struct Noop : Statement {
  Noop(SourceInfo &info) : Statement(info) {}
  ExecutionResult Execute() override { return ExecutionResult::None; }
};
struct DotExpr : Expression {
  DotExpr(SourceInfo &info, ExpressionPtr &&left,
          ExpressionPtr &&right);
  ExpressionPtr left;
  ExpressionPtr right;
  Value Evaluate() override;
  void Assign(Value value);
};
struct DotAssignment : Statement {
  DotAssignment(SourceInfo &info, ExpressionPtr &&dot,
                ExpressionPtr &&value);
  ExpressionPtr dot;
  ExpressionPtr value;
  ExecutionResult Execute() override;
};
struct DotCallStmnt : Statement {
  DotCallStmnt(SourceInfo &info, ExpressionPtr &&dot);
  ExpressionPtr dot;
  ExecutionResult Execute() override;
};
struct Subscript : Expression {
  Subscript(SourceInfo &info, ExpressionPtr &&left,
            ExpressionPtr &&idx);
  ExpressionPtr left;
  ExpressionPtr index;
  Value Evaluate();
};
struct SubscriptAssignStmnt : Statement {
  SubscriptAssignStmnt(SourceInfo &info,
                       ExpressionPtr &&subscript, ExpressionPtr &&value);
  ExpressionPtr subscript;
  ExpressionPtr value;
  ExecutionResult Execute() override;
};
struct UnaryExpr : Expression {
  UnaryExpr(SourceInfo &info, ExpressionPtr &&left, TType op);
  ExpressionPtr left;
  TType op;
  Value Evaluate() override;
};

// this is basically only for decrement / increment right now
struct UnaryStatement : Statement {
  ExpressionPtr expr;
  UnaryStatement(SourceInfo &info, ExpressionPtr &&expr);
  ExecutionResult Execute() override {
    expr->Evaluate();
    return ExecutionResult::None;
  }
};

struct BinExpr : Expression {
  ExpressionPtr left;
  ExpressionPtr right;
  TType op;
  BinExpr(SourceInfo &info, ExpressionPtr &&left,
          ExpressionPtr &&right, TType op);
  Value Evaluate() override;
};

struct Using : Statement {
  static vector<string> activeModules;
  Using(SourceInfo &info, const string &name,
         const bool isWildcard);
  Using(SourceInfo &info, const string &name,
         vector<string> &symbols);
  vector<string> symbols;
  string moduleName;
  bool isWildcard;
  const string moduleRoot = "/usr/local/scrit/modules/";
  ExecutionResult Execute() override;
};

string CC_ToString(ControlChange controlChange);
Value EvaluateWithinObject(Scope scope, Value object, ExpressionPtr &expr);
Value TryCallMethods(unique_ptr<Expression> &right, Value lvalue);
Value EvaluateWithinObject(Scope scope, Value object, std::function<Value()> lambda);