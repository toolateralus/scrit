#pragma once
#include "lexer.hpp"
#include "value.hpp"
#include <memory>


struct Context;
struct Value_T;
struct Scope_T;
typedef std::shared_ptr<Value_T> Value;
typedef std::shared_ptr<Scope_T> Scope;

using std::make_unique;
using std::unique_ptr;

// forawrd declarations
struct ExecutionResult;
struct ASTNode;
struct Executable;
struct Statement;
struct Program;
struct Expression;
struct Operand;
struct Identifier;
struct Arguments;
struct Parameters;
struct Continue;
struct Break;
struct Return;
struct Block;
struct ObjectInitializer;
struct Call;
struct If;
struct Else;
struct For;
struct Assignment;
struct FuncDecl;
struct DotExpr;
struct DotAssignment;
struct DotCallStmnt;
struct Subscript;
struct SubscriptAssignStmnt;
struct UnaryExpr;
struct BinExpr;
// typedefs
typedef unique_ptr<Statement> Statement_up;
typedef unique_ptr<Expression> Expression_up;
typedef unique_ptr<Block> Block_up;
typedef unique_ptr<Arguments> Arguments_up;
typedef unique_ptr<If> If_up;
typedef unique_ptr<Else> Else_up;
typedef unique_ptr<Identifier> Identifier_up;
typedef unique_ptr<Parameters> Parameters_up;

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
  vector<Statement_up> statements;
  Program(vector<Statement_up> &&statements);
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
  Arguments(vector<Expression_up> &&args);
  vector<Expression_up> values;
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
  Return(Expression_up &&value);
  Expression_up value;
  ExecutionResult Execute() override;
};
struct Block : Statement {
  Block(vector<Statement_up> &&statements);
  vector<Statement_up> statements;
  Scope scope;
  ExecutionResult Execute() override;
};
struct ObjectInitializer : Expression {
  Block_up block;
  Scope scope;
  ObjectInitializer(Block_up block);
  Value Evaluate() override;
};
struct Call : Expression, Statement {
  Expression_up operand;
  Arguments_up args;
  Call(Expression_up &&operand, Arguments_up &&args);
  vector<Value> GetArgsValueList(Arguments_up &args);
  Value Evaluate() override;
  ExecutionResult Execute() override;
};
struct If : Statement {
  static If_up NoElse(Expression_up &&condition, Block_up &&block);
  static If_up WithElse(Expression_up &&condition, Block_up &&block, Else_up &&elseStmnt);
  If(Expression_up &&condition, Block_up &&block);
  If(Expression_up &&condition, Block_up &&block,  Else_up &&elseStmnt);
  Expression_up condition;
  Block_up block;
  Else_up elseStmnt;
  ExecutionResult Execute() override;
};
struct Else : Statement {
  If_up ifStmnt;
  Block_up block;
  static Else_up NoIf(Block_up &&block);
  static Else_up New(If_up &&ifStmnt);
  ExecutionResult Execute() override;
};
struct For : Statement {
  Statement_up decl;
  Expression_up condition;
  Statement_up increment;
  Block_up block;
  Scope scope;
  For(Statement_up &&decl, Expression_up &&condition, Statement_up &&inc, Block_up &&block, Scope scope);
  ExecutionResult Execute() override;
};
struct Assignment : Statement {
  Identifier_up iden;
  Expression_up expr;
  Assignment(Identifier_up &&iden, Expression_up &&expr);
  ExecutionResult Execute() override;
};
struct FuncDecl : Statement {
  FuncDecl(Identifier_up &&name, Block_up &&body, Parameters_up &&parameters);
  Identifier_up name;
  Block_up body;
  Parameters_up parameters;
  ExecutionResult Execute() override;
};
struct DotExpr : Expression {
  DotExpr(Expression_up &&left, Expression_up &&right);
  Expression_up left;
  Expression_up right;
  Value Evaluate() override;
  void Assign(Value value);
};
struct DotAssignment : Statement {
  DotAssignment(Expression_up &&dot, Expression_up &&value);
  Expression_up dot;
  Expression_up value;
  ExecutionResult Execute() override;
};
struct DotCallStmnt : Statement {
  DotCallStmnt(Expression_up &&dot);
  Expression_up dot;
  ExecutionResult Execute() override;
};
struct Subscript : Expression {
  Subscript(Expression_up &&left, Expression_up &&idx);
  Expression_up left;
  Expression_up index;
  Value Evaluate();
};
struct SubscriptAssignStmnt : Statement {
  SubscriptAssignStmnt(Expression_up &&subscript, Expression_up &&value);
  Expression_up subscript;
  Expression_up value;
  ExecutionResult Execute() override;
};
struct UnaryExpr : Expression {
  UnaryExpr(Expression_up &&left, TType op);
  Expression_up left;
  TType op;
  Value Evaluate() override;
};
struct BinExpr : Expression {
  Expression_up left;
  Expression_up right;
  TType op;
  BinExpr(Expression_up &&left, Expression_up &&right, TType op);
  Value Evaluate() override;
};

string CC_ToString(ControlChange controlChange);