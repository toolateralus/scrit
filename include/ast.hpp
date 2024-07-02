#pragma once
#include "lexer.hpp"
#include "native.hpp"
#include "type.hpp"

#include <cassert>
#include <functional>
#include <memory>
#include <vector>

class ASTVisitor;

using std::string;
using std::vector;

enum class Mutability {
  Const = 0,
  Mut,
};

enum struct TType;
struct Context;

namespace Values {
struct Value_T;
struct Type_T;
enum class PrimitiveType;
} // namespace Values

using Type = std::shared_ptr<Values::Type_T>;

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
struct Lambda;
struct FunctionDecl;
struct Delete;

namespace Values {
struct Type_T;
};

using Type = std::shared_ptr<Values::Type_T>;

// typedefs
using DeletePtr = std::unique_ptr<Delete>;
using LambdaPtr = std::unique_ptr<Lambda>;
using UsingPtr = std::unique_ptr<Using>;
using StatementPtr = std::unique_ptr<Statement>;
using ExpressionPtr = std::unique_ptr<Expression>;
using BlockPtr = std::unique_ptr<Block>;
using ArgumentsPtr = std::unique_ptr<Arguments>;
using IfPtr = std::unique_ptr<If>;
using ElsePtr = std::unique_ptr<Else>;
using IdentifierPtr = std::unique_ptr<Identifier>;
using ParametersPtr = std::unique_ptr<Parameters>;
using OperandPtr = std::unique_ptr<Operand>;
using FunctionDeclPtr = std::unique_ptr<FunctionDecl>;

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
  virtual void Accept(ASTVisitor *visitor) = 0;
};

struct Executable : ASTNode {
  virtual ~Executable() {}
  virtual ExecutionResult Execute() = 0;
  void Accept(ASTVisitor *visitor) override;
  Executable(SourceInfo &info) : ASTNode(info) {}
};
struct Statement : Executable {
  Statement(SourceInfo &info) : Executable(info) {}
  void Accept(ASTVisitor *visitor) override;
};
struct Program : Executable {
  vector<StatementPtr> statements;
  Program(vector<StatementPtr> &&statements);
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Expression : ASTNode {
  Type type;
  Expression(SourceInfo &info, const Type &type) : ASTNode(info), type(type) {}
  virtual ~Expression() {}
  virtual Value Evaluate() = 0;
  void Accept(ASTVisitor *visitor) override;
};

struct DefaultValue : Expression {
  DefaultValue(SourceInfo &info, const Type &type) : Expression(info, type) {};
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};

struct Literal : Expression {
  Literal(SourceInfo &info, const Type &type, Value value)
      : Expression(info, type), expression(value) {}
  Value expression;
  void Accept(ASTVisitor *visitor) override;
  Value Evaluate() override;
};

struct Operand : Expression {
  Operand(SourceInfo &info, const Type &type, ExpressionPtr &&value);
  ExpressionPtr expression;
  void Accept(ASTVisitor *visitor) override;
  Value Evaluate() override;
};

struct Identifier : Expression {
  const string name;
  Identifier(SourceInfo &info, const Type &type, const string &name);
  Identifier(SourceInfo &info, const string &name);
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Arguments : Expression {
  Arguments(SourceInfo &info, vector<ExpressionPtr> &&args);
  vector<ExpressionPtr> values;
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};
struct TupleInitializer : Expression {
  vector<ExpressionPtr> values;
  vector<Type> types;
  TupleInitializer(SourceInfo &info, vector<ExpressionPtr> &&values);
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Property : Statement {
  const string name;
  ExpressionPtr lambda;
  const Mutability mutability;
  Property(SourceInfo &info, const string &name, ExpressionPtr &&lambda,
           const Mutability &mut);
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Parameters : Statement {
  struct Param {
    string name;
    Value default_value;
    Type type;
  };
  std::vector<Param> values;
  
  Parameters(SourceInfo &info);
  auto ParamTypes() -> vector<Type> {
    vector<Type> types;
    for (const auto &p : values) {
      types.push_back(p.type);
    }
    return types;
  }
  Parameters(SourceInfo &info, std::vector<Param> &&params);
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
  
  auto Clone() -> unique_ptr<Parameters>;
};
struct Continue : Statement {
  Continue(SourceInfo &info) : Statement(info) {}
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Break : Statement {
  Break(SourceInfo &info) : Statement(info) {}
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Return : Statement {
  Return(SourceInfo &info) : Statement(info) {}
  Return(SourceInfo &info, ExpressionPtr &&value);
  ExpressionPtr value;
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct DotExpr;
struct Delete : Statement {
  IdentifierPtr iden;
  ExpressionPtr dot;
  ~Delete();
  Delete(SourceInfo &info, ExpressionPtr &&dot);
  Delete(SourceInfo &info, IdentifierPtr &&iden);
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Block : Statement {
  Block(SourceInfo &info, vector<StatementPtr> &&statements, Scope scope);
  vector<StatementPtr> statements;
  Scope scope;
  ExecutionResult Execute() override;
  ExecutionResult Execute(Scope scope);
  void Accept(ASTVisitor *visitor) override;
};
struct ObjectInitializer : Expression {
  BlockPtr block;
  ObjectInitializer(SourceInfo &info, const Type &type, BlockPtr &&block);
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};

struct TypeAlias : Statement {
  const Type type;
  const string alias;
  TypeAlias(SourceInfo &info, const string &alias, const Type &type);
  ExecutionResult Execute() override { return ExecutionResult::None; }
};

struct ArrayInitializer : Expression {
  vector<ExpressionPtr> init;
  ArrayInitializer(SourceInfo &info, const Type &type,
                   vector<ExpressionPtr> &&init);
  Value Evaluate() override;
};
namespace Values {
struct Callable_T;
}
struct Call : virtual Expression, virtual Statement {
  ExpressionPtr operand;
  ArgumentsPtr args;
  Call(SourceInfo &info, ExpressionPtr &&operand, ArgumentsPtr &&args);
  static vector<Value> GetArgsValueList(ArgumentsPtr &args);
  void ValidateArgumentSize(std::shared_ptr<Values::Callable_T> &callable);
  Value Evaluate() override;
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct MethodCall : virtual Expression, virtual Statement {
  ExpressionPtr operand;
  ArgumentsPtr args;
  shared_ptr<Values::Callable_T> callable;
  shared_ptr<Values::Callable_T> FindCallable();
  MethodCall(SourceInfo &info, const Type &type, ExpressionPtr &&operand,
             ArgumentsPtr &&args);
  ExecutionResult Execute() override {
    Evaluate();
    return ExecutionResult::None;
  }
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};

struct If : Statement {
  If() = delete;
  static IfPtr NoElse(SourceInfo &info, ExpressionPtr &&condition,
                      BlockPtr &&block);
  static IfPtr WithElse(SourceInfo &info, ExpressionPtr &&condition,
                        BlockPtr &&block, ElsePtr &&elseStmnt);
  If(SourceInfo &info, ExpressionPtr &&condition, BlockPtr &&block);
  If(SourceInfo &info, ExpressionPtr &&condition, BlockPtr &&block,
     ElsePtr &&elseStmnt);
  ExpressionPtr condition;
  BlockPtr block;
  ElsePtr elseStmnt;
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
  ~If();
};

struct AnonymousFunction : Expression {
  Value callable;
  AnonymousFunction(SourceInfo &info, Type &type, Value callable);
  Value Evaluate() override;
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
  void Accept(ASTVisitor *visitor) override;
};
struct For : Statement {
  StatementPtr decl;
  ExpressionPtr condition;
  StatementPtr increment;
  BlockPtr block;
  Scope scope;
  For(SourceInfo &info, StatementPtr &&decl, ExpressionPtr &&condition,
      StatementPtr &&inc, BlockPtr &&block, Scope scope);
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct RangeBasedFor : Statement {
  RangeBasedFor(SourceInfo &info, vector<string> &names, ExpressionPtr &&rhs,
                BlockPtr &&block)
      : Statement(info), names(names), rhs(std::move(rhs)),
        block(std::move(block)) {}
  RangeBasedFor(SourceInfo &info, ExpressionPtr &&lhs, ExpressionPtr &&rhs,
                BlockPtr &&block);

  vector<string> names;
  ExpressionPtr lhs;
  ExpressionPtr rhs;
  BlockPtr block;
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Assignment : Statement {
  const IdentifierPtr iden;
  const ExpressionPtr expr;
  const Type type;
  Assignment(SourceInfo &info, const Type &type, IdentifierPtr &&iden,
             ExpressionPtr &&expr);
  void Accept(ASTVisitor *visitor) override;
  ExecutionResult Execute() override;
};
struct Declaration : Statement {
  const string name;
  const ExpressionPtr expr;
  const Mutability mut;
  const Type type;
  Declaration(SourceInfo &info, const string &name, ExpressionPtr &&expr,
              const Mutability &mut, const Type &type);
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct TupleDeconstruction : Statement {
  TupleDeconstruction(SourceInfo &info, vector<string> &&idens,
                      ExpressionPtr &&tuple)
      : Statement(info), idens(std::move(idens)), tuple(std::move(tuple)) {}
  vector<string> idens;
  ExpressionPtr tuple;
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct CompAssignExpr : Expression {
  ExpressionPtr left, right;
  TType op;
  CompAssignExpr(SourceInfo &info, ExpressionPtr &&left, ExpressionPtr &&right,
                 TType op);
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};
struct CompoundAssignment : Statement {
  ExpressionPtr expr;
  CompoundAssignment(SourceInfo &info, ExpressionPtr &&expr);
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct FunctionDecl : Statement {
  BlockPtr block;
  ParametersPtr parameters;
  const string name;
  const Type returnType;
  FunctionDecl(SourceInfo &info, string &name, BlockPtr &&block,
               ParametersPtr &&parameters, const Type &returnType)
      : Statement(info), block(std::move(block)),
        parameters(std::move(parameters)), name(name), returnType(returnType) {}
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Noop : Statement {
  Noop(SourceInfo &info) : Statement(info) {}
  ExecutionResult Execute() override { return ExecutionResult::None; }
  void Accept(ASTVisitor *visitor) override;
};
struct DotExpr : Expression {
  DotExpr(SourceInfo &info, const Type &type, ExpressionPtr &&left,
          ExpressionPtr &&right);
  ExpressionPtr left;
  ExpressionPtr right;
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
  void Assign(Value value);
};
struct DotAssignment : Statement {
  DotAssignment(SourceInfo &info, ExpressionPtr &&dot, ExpressionPtr &&value);
  ExpressionPtr dot;
  ExpressionPtr value;
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct DotCallStmnt : Statement {
  DotCallStmnt(SourceInfo &info, ExpressionPtr &&dot);
  ExpressionPtr dot;
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Subscript : Expression {
  Subscript(SourceInfo &info, const Type &type, ExpressionPtr &&left,
            ExpressionPtr &&idx);
  ExpressionPtr left;
  ExpressionPtr index;
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};
struct SubscriptAssignStmnt : Statement {
  SubscriptAssignStmnt(SourceInfo &info, ExpressionPtr &&subscript,
                       ExpressionPtr &&value);
  ExpressionPtr subscript;
  ExpressionPtr value;
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct UnaryExpr : Expression {
  UnaryExpr(SourceInfo &info, const Type &type, ExpressionPtr &&left, TType op);
  ExpressionPtr operand;
  TType op;
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};
// this is basically only for decrement / increment right now
struct UnaryStatement : Statement {
  ExpressionPtr expr;
  UnaryStatement(SourceInfo &info, ExpressionPtr &&expr);
  ExecutionResult Execute() override {
    expr->Evaluate();
    return ExecutionResult::None;
  }
  void Accept(ASTVisitor *visitor) override;
};
struct BinExpr : Expression {
  ExpressionPtr left;
  ExpressionPtr right;
  TType op;
  BinExpr(SourceInfo &info, ExpressionPtr &&left, ExpressionPtr &&right,
          TType op);
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Using : Statement {
  static vector<string> activeModules;
  void Load();
  Using(SourceInfo &info, const string &name, const bool isWildcard);
  Using(SourceInfo &info, const string &name, vector<string> &symbols);
  vector<string> symbols;
  string moduleName;
  bool isWildcard;
  // TODO: make this cross platform friendly. it's the only thing keeping us
  // linux-only, as well as the install script.
  const string moduleRoot = "/usr/local/scrit/modules/";
  ExecutionResult Execute() override;
  void Accept(ASTVisitor *visitor) override;
};
struct Lambda : Expression {
  BlockPtr block = nullptr;
  ExpressionPtr expr = nullptr;
  Lambda(SourceInfo &info, const Type &type, ExpressionPtr &&expr)
      : Expression(info, type), expr(std::move(expr)) {}
  Lambda(SourceInfo &info, const Type &type, BlockPtr &&block)
      : Expression(info, type), block(std::move(block)) {}
  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};
// TODO: make a match expression that calls into this and just returns the
// control flow change result.
struct Match : Expression {
  ExpressionPtr expr;
  std::vector<ExpressionPtr> patterns = {};
  std::vector<ExpressionPtr> expressions = {};
  ExpressionPtr branch_default;

  Match(SourceInfo &info, const Type &type, ExpressionPtr &&expr,
        std::vector<ExpressionPtr> &&branch_lhs,
        std::vector<ExpressionPtr> &&branch_rhs,
        ExpressionPtr &&branch_default = nullptr)
      : Expression(info, type), expr(std::move(expr)),
        patterns(std::move(branch_lhs)), expressions(std::move(branch_rhs)),
        branch_default(std::move(branch_default)) {
    assert(branch_lhs.size() == branch_rhs.size());
  }

  Value Evaluate() override;
  void Accept(ASTVisitor *visitor) override;
};

struct StructDeclaration : Statement {
  unique_ptr<ObjectInitializer> ctor_obj;
  string name;
  StructDeclaration(SourceInfo &info, const string &name,
                    unique_ptr<ObjectInitializer> &&ctor_obj);

  ExecutionResult Execute() override;
};

struct Constructor : Expression {
  ArgumentsPtr args;
  Constructor(SourceInfo &info, const Type &type, ArgumentsPtr &&args);
  Value Evaluate() override;
};

struct MatchStatement : Statement {
  ExpressionPtr match;
  MatchStatement(SourceInfo &info, ExpressionPtr &&match)
      : Statement(info), match(std::move(match)) {}
  ExecutionResult Execute() override {
    auto result = match->Evaluate();
    // we could just this value since its unreachable in this case.
    return ExecutionResult(ControlChange::None, result);
  }
  void Accept(ASTVisitor *visitor) override;
};
string CC_ToString(ControlChange controlChange);

Value EvaluateWithinObject(Scope &scope, Value object, ExpressionPtr &expr);
Value EvaluateWithinObject(Scope &scope, Value object,
                           std::function<Value()> lambda);
