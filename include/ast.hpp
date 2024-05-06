#pragma once
#include "context.hpp"
#include "lexer.hpp"
#include "native.hpp"
#include "value.hpp"
#include <memory>
#include <stdexcept>

using std::make_unique;
using std::unique_ptr;

struct ASTNode {
  static Context context;
  virtual ~ASTNode() {}
  virtual Value Evaluate() = 0;
};
struct Expression {
  virtual ~Expression() {}
  virtual Value Evaluate() = 0;
};
struct Statement : ASTNode {
  virtual ~Statement() {}
  virtual Value Evaluate() override {
    throw std::runtime_error("Cannot evaluate a statement to a value");
  }
  virtual unique_ptr<ASTNode> EvaluateStatement() = 0;
};
struct Program : ASTNode {
  vector<unique_ptr<Statement>> statements;
  Program(vector<unique_ptr<Statement>> &&statements)
      : statements(std::move(statements)) {}
  Value Evaluate() override {
    Value result;
    for (auto &statement : statements) {
      statement->EvaluateStatement();
    }
    return result;
  }
};
struct Operand : Expression {
  Operand(Value value) : value(value) {}
  Value value;
  Value Evaluate() override { return value; }
};
struct Identifier : Expression {
  Identifier(string &name) : name(name) {}
  string name;
  Value Evaluate() override {
    auto value = ASTNode::context.Find(name);
    if (value != nullptr) {
      return value;
    }
    return Value_T::Undefined;
  }
};
struct Arguments : Expression {
  Arguments(vector<unique_ptr<Expression>> &&args) : values(std::move(args)) {}
  vector<unique_ptr<Expression>> values;
  Value Evaluate() override {
    // do nothing here.
    return Value_T::Null;
  }
};
struct Parameters : Statement {
  vector<string> names;
  Parameters(vector<string> &&names) : names(std::move(names)) {}
  unique_ptr<ASTNode> EvaluateStatement() override { return nullptr; }
};
struct Continue : Statement {
  unique_ptr<ASTNode> EvaluateStatement() override { return nullptr; }
};
struct Break : Statement {
  unique_ptr<ASTNode> EvaluateStatement() override { return nullptr; }
};
struct Return : Statement {
  Return(unique_ptr<Expression> &&value) : value(std::move(value)) {}
  unique_ptr<Expression> value;
  Value Evaluate() override { return value->Evaluate(); }
  unique_ptr<ASTNode> EvaluateStatement() override { return nullptr; }
};
struct Block : Statement {
  Block(vector<unique_ptr<Statement>> &&statements)
      : statements(std::move(statements)) {}
  vector<unique_ptr<Statement>> statements;
  shared_ptr<Scope> scope;
  unique_ptr<ASTNode> EvaluateStatement() override {
    scope = ASTNode::context.PushScope();
    for (auto &statement : statements) {
      if (dynamic_cast<Return *>(statement.get()) ||
          dynamic_cast<Break *>(statement.get()) ||
          dynamic_cast<Continue *>(statement.get())) {
        auto value = statement->EvaluateStatement();
        ASTNode::context.PopScope();
        return value;
      } else {
        statement->EvaluateStatement();
      }
    }
    ASTNode::context.PopScope();
    return nullptr;
  }
};
struct ObjectInitializer : Expression {
  unique_ptr<Block> block;
  shared_ptr<Scope> scope;
  ObjectInitializer(unique_ptr<Block> block) : block(std::move(block)) {}
  Value Evaluate() override {
    scope = ASTNode::context.PushScope();
    auto value = block->EvaluateStatement();
    ASTNode::context.PopScope();
    return Value_T::Null;
  }
};

struct Call : Expression, Statement {
  unique_ptr<Expression> operand;
  unique_ptr<Arguments> args;
  
  Call(unique_ptr<Expression> &&operand, unique_ptr<Arguments> &&args)
      : operand(std::move(operand)), args(std::move(args)) {}
  
  vector<Value> GetArgsValueList(unique_ptr<Arguments> &args) {
    vector<Value> values ={};
    for (auto &expr : args->values) {
      values.push_back(expr->Evaluate());
    }
    return values;
  }
  
  Value Evaluate() override {
    auto lvalue = operand->Evaluate();
    if (auto callable = dynamic_cast<Callable_T *>(lvalue.get())) {
      return callable->Call(std::move(args));
    } else if (auto id = dynamic_cast<Identifier* >(operand.get())) {
      if (NativeFunctions::GetRegistry().count(id->name) > 0) {
        return NativeFunctions::GetRegistry()[id->name](GetArgsValueList(this->args));
      }
    }
    return Value_T::Undefined;
  }
  unique_ptr<ASTNode> EvaluateStatement() override {
    Evaluate();
    return nullptr;
  }
};
struct Else;
struct If : Statement {

  static unique_ptr<If> NoElse(unique_ptr<Expression> &&condition,
                               unique_ptr<Block> &&block);
  static unique_ptr<If> WithElse(unique_ptr<Expression> &&condition,
                                 unique_ptr<Block> &&block,
                                 unique_ptr<Else> &&elseStmnt);

  If(unique_ptr<Expression> &&condition, unique_ptr<Block> &&block);

  If(unique_ptr<Expression> &&condition, unique_ptr<Block> &&block,
     unique_ptr<Else> &&elseStmnt);

  unique_ptr<Expression> condition;
  unique_ptr<Block> block;
  unique_ptr<Else> elseStmnt;
  unique_ptr<ASTNode> EvaluateStatement() override;
};
struct Else : Statement {
  unique_ptr<If> ifStmnt;
  unique_ptr<Block> block;

  static unique_ptr<Else> NoIf(unique_ptr<Block> &&block);
  static unique_ptr<Else> New(unique_ptr<If> &&ifStmnt);

  unique_ptr<ASTNode> EvaluateStatement() override {
    if (ifStmnt != nullptr) {
      return ifStmnt->EvaluateStatement();
    } else {
      return block->EvaluateStatement();
    }
  }
};


struct For : Statement {
  unique_ptr<Statement> decl;
  unique_ptr<Expression> condition;
  unique_ptr<Statement> increment;
  unique_ptr<Block> block;
  shared_ptr<Scope> scope;

  For(unique_ptr<Statement> &&decl, unique_ptr<Expression> &&condition,
      unique_ptr<Statement> &&inc, unique_ptr<Block> &&block,
      shared_ptr<Scope> scope)
      : decl(std::move(decl)), condition(std::move(condition)),
        increment(std::move(inc)), block(std::move(block)), scope(scope) {}

  unique_ptr<ASTNode> EvaluateStatement() override {
    context.PushScope(scope);
    if (decl != nullptr)
      decl->EvaluateStatement();
    
    if (condition != nullptr) {
      while (true) {
        auto conditionResult = condition->Evaluate();
        auto b = dynamic_cast<Bool_T *>(conditionResult.get());

        if (b->Equals(Bool_T::False)) {
          context.PopScope();
          return nullptr;
        }

        auto blockResult = block->EvaluateStatement();
        increment->Evaluate();

        if (dynamic_cast<Return *>(blockResult.get()) ||
            dynamic_cast<Continue *>(blockResult.get()) ||
            dynamic_cast<Break *>(blockResult.get())) {
          context.PopScope();
          return blockResult;
        }
      }
    } else {
      while (true) {
        if (increment)
          increment->Evaluate();
        
        auto blockResult = block->EvaluateStatement();
        if (dynamic_cast<Return *>(blockResult.get()) ||
            dynamic_cast<Continue *>(blockResult.get()) ||
            dynamic_cast<Break *>(blockResult.get())) {
          context.PopScope();
          return blockResult;
        }
      }
    }
    context.PopScope();
    return nullptr;
  }
};
struct Assignment : Statement {
  unique_ptr<Identifier> iden;
  unique_ptr<Expression> expr;
  Assignment(unique_ptr<Identifier> &&iden, unique_ptr<Expression> &&expr)
      : iden(std::move(iden)), expr(std::move(expr)) {}
  unique_ptr<ASTNode> EvaluateStatement() override {
    context.Insert(iden->name, expr->Evaluate());
    return nullptr;
  }
};
struct FuncDecl : Statement {
  FuncDecl(unique_ptr<Identifier> &&name, unique_ptr<Block> &&body,
           unique_ptr<Parameters> &&parameters)
      : name(std::move(name)), body(std::move(body)),
        parameters(std::move(parameters)) {}
  unique_ptr<Identifier> name;
  unique_ptr<Block> body;
  unique_ptr<Parameters> parameters;

  unique_ptr<ASTNode> EvaluateStatement() override {
    auto callable =
        make_shared<Callable_T>(std::move(body), std::move(parameters));
    context.Insert(name->name, callable);
    return nullptr;
  }
};
struct DotExpr : Expression {
  DotExpr(unique_ptr<Expression> &&left, unique_ptr<Expression> &&right)
      : left(std::move(left)), right(std::move(right)) {}
  unique_ptr<Expression> left;
  unique_ptr<Expression> right;
  Value Evaluate() override {
    auto leftValue = left->Evaluate();
    auto obj = dynamic_cast<Object_T *>(leftValue.get());
    if (!obj) {
      throw std::runtime_error("invalid lhs on dot operation");
    }
    ASTNode::context.PushScope(obj->scope);
    auto result = right->Evaluate();
    ASTNode::context.PopScope();
    return result;
  }
  void Assign(Value value) {
    auto leftValue = left->Evaluate();
    auto obj = dynamic_cast<Object_T *>(leftValue.get());
    if (!obj) {
      throw std::runtime_error("invalid lhs on dot operation");
    }
    if (auto dotExpr = dynamic_cast<DotExpr *>(right.get())) {
      ASTNode::context.PushScope(obj->scope);
      dotExpr->Assign(value);
      ASTNode::context.PopScope();
    }
    if (auto identifier = dynamic_cast<Identifier *>(right.get())) {
      obj->SetMember(identifier->name, value);
    }
  }
};
struct DotAssignment : Statement {
  DotAssignment(unique_ptr<Expression> &&dot, unique_ptr<Expression> &&value)
      : dot(std::move(dot)), value(std::move(value)) {}
  unique_ptr<Expression> dot;
  unique_ptr<Expression> value;
  unique_ptr<ASTNode> EvaluateStatement() override {
    if (auto dot = dynamic_cast<DotExpr*>(this->dot.get()))
      dot->Assign(value->Evaluate());
    return nullptr;
  }
};
struct DotCallStmnt : Statement {
  DotCallStmnt(unique_ptr<Expression> &&dot) : dot(std::move(dot)) {}
  unique_ptr<Expression> dot;
  unique_ptr<ASTNode> EvaluateStatement() override {
    dot->Evaluate();
    return nullptr;
  }
};
struct Subscript : Expression {

  Subscript(unique_ptr<Expression> &&left, unique_ptr<Expression> &&idx)
      : left(std::move(left)), index(std::move(idx)) {}
  unique_ptr<Expression> left;
  unique_ptr<Expression> index;
  Value Evaluate() {

    auto lvalue = left->Evaluate();
    auto array = dynamic_cast<Array_T *>(lvalue.get());
    if (!array) {
      throw std::runtime_error("Cannot subscript a non array");
    }
    auto idx = index->Evaluate();
    auto number = std::dynamic_pointer_cast<Int_T>(idx);
    if (!number) {
      throw std::runtime_error("Subscript index must be a number.");
    }
    return array->At(number);
  }
};
struct SubscriptAssignStmnt : Statement {
  SubscriptAssignStmnt(unique_ptr<Expression> &&subscript,
                       unique_ptr<Expression> &&value)
      : subscript(std::move(subscript)), value(std::move(value)) {}
  unique_ptr<Expression> subscript;
  unique_ptr<Expression> value;
  unique_ptr<ASTNode> EvaluateStatement() override {
    auto subscript = dynamic_cast<Subscript*>(this->subscript.get());
    
    if (!subscript) {
      throw std::runtime_error("invalid subscript");
    }
    
    auto lvalue = subscript->left->Evaluate();
    auto idx = subscript->index->Evaluate();

    auto array = dynamic_cast<Array_T *>(lvalue.get());
    auto number = std::dynamic_pointer_cast<Int_T>(idx);

    if (!array || !number) {
      throw std::runtime_error(
          "cannot subscript a non array or with a non-integer value.");
    }

    array->Assign(number, value->Evaluate());
    return nullptr;
  }
};
struct UnaryExpr : Expression {
  UnaryExpr(unique_ptr<Expression> &&left, TType op)
      : left(std::move(left)), op(op) {}
  unique_ptr<Expression> left;
  TType op;
  Value Evaluate() override {
    auto lvalue = left->Evaluate();

    if (op == TType::Sub) {
      return lvalue->Negate();
    } else if (op == TType::Not) {
      return lvalue->Not();
    }
    return Value_T::Null;
  }
};
struct BinExpr : Expression {
  unique_ptr<Expression> left;
  unique_ptr<Expression> right;
  TType op;
  BinExpr(unique_ptr<Expression> &&left, unique_ptr<Expression> &&right,
          TType op)
      : left(std::move(left)), right(std::move(right)), op(op) {}

  Value Evaluate() override {
    auto left = this->left->Evaluate();
    auto right = this->left->Evaluate();

    switch (op) {
    case TType::Add:
      return left->Add(right);
    case TType::Sub:
      return left->Subtract(right);
    case TType::Mul:
      return left->Multiply(right);
    case TType::Div:
      return left->Divide(right);
    case TType::Or:
      return left->Or(right);
    case TType::And:
      return left->And(right);
    case TType::Greater:
      return left->Greater(right);
    case TType::Less:
      return left->Less(right);
    case TType::GreaterEQ:
      return left->GreaterEquals(right);
    case TType::LessEQ:
      return left->LessEquals(right);
    case TType::Equals:
      return make_shared<Bool_T>(left->Equals(right));
    case TType::Assign:
      left->Set(right);
      return left;
    default:
      throw std::runtime_error("invalid operator in binary expresison " +
                               TTypeToString(op));
    }
  };
};