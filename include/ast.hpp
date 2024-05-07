#pragma once
#include "context.hpp"
#include "lexer.hpp"
#include "native.hpp"
#include "value.hpp"
#include <memory>
#include <stdexcept>

using std::make_unique;
using std::unique_ptr;

static Object MakeException(const string &msg, const string &type) {
  auto e = make_shared<Object_T>();
  e->scope = make_shared<Scope>();
  e->scope->variables["msg"] = make_shared<String_T>(msg);
  e->scope->variables["type"] = make_shared<String_T>(type);
  return e;
}

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

static string CC_ToString(ControlChange controlChange) {
  switch (controlChange) {
  case ControlChange::None:
    return "None";
  case ControlChange::Return:
    return "Return";
  case ControlChange::Continue:
    return "Continue";
  case ControlChange::Break:
    return "Break";
  case ControlChange::Goto:
    return "Goto";
  case ControlChange::ContinueLabel:
    return "ContinueLabel";
  case ControlChange::BreakLabel:
    return "BreakLabel";
  case ControlChange::Exception:
    return "Exception";
  }
}

struct ExecutionResult {
  ExecutionResult(ControlChange controlChange, Value value) : 
    controlChange(controlChange), value(value) {}
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
struct Expression {
  virtual ~Expression() {}
  virtual Value Evaluate() = 0;
};
struct Executable : ASTNode {
  virtual ~Executable() {}
  virtual ExecutionResult Execute() = 0;
};
struct Statement : Executable {};
struct Program : Executable {
  vector<unique_ptr<Statement>> statements;
  Program(vector<unique_ptr<Statement>> &&statements)
      : statements(std::move(statements)) {}
  ExecutionResult Execute() override {
    for (auto &statement : statements) {
      auto result = statement->Execute();
      switch (result.controlChange) {
      case ControlChange::Exception:
        throw std::runtime_error("Uncaught Exception: " + result.value->ToString());
      case ControlChange::Goto:
      case ControlChange::ContinueLabel:
      case ControlChange::BreakLabel:
        // TODO: Check for label Here
        throw std::runtime_error(CC_ToString(result.controlChange) + " not implemented");
      case ControlChange::None:
        continue;
      default:
        throw std::runtime_error("Uncaught " + CC_ToString(result.controlChange));
      }
    }
    return ExecutionResult::None;
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
  ExecutionResult Execute() override {
    return ExecutionResult::None;
  }
};
struct Continue : Statement {
  ExecutionResult Execute() override {
    return ExecutionResult::Continue;
  }
};
struct Break : Statement {
  ExecutionResult Execute() override {
    return ExecutionResult::Break;
  }
};
struct Return : Statement {
  Return(unique_ptr<Expression> &&value) : value(std::move(value)) {}
  unique_ptr<Expression> value;
  ExecutionResult Execute() override {
    return ExecutionResult(ControlChange::Return, value->Evaluate());
  }
};
struct Block : Statement {
  Block(vector<unique_ptr<Statement>> &&statements)
      : statements(std::move(statements)) {}
  vector<unique_ptr<Statement>> statements;
  shared_ptr<Scope> scope;
  ExecutionResult Execute() override {
    scope = ASTNode::context.PushScope();
    for (auto &statement : statements) {
      auto result = statement->Execute();
      switch (result.controlChange) {
      case ControlChange::Goto:
      case ControlChange::ContinueLabel:
      case ControlChange::BreakLabel:
        // TODO: Check for label Here
        throw std::runtime_error(CC_ToString(result.controlChange) + " not implemented");
      case ControlChange::Continue:
      case ControlChange::Break:
      case ControlChange::Return:
      case ControlChange::Exception:
        ASTNode::context.PopScope();
        return result;
      case ControlChange::None:
        continue;
      }
    }
    ASTNode::context.PopScope();
    return ExecutionResult::None;
  }
};
struct ObjectInitializer : Expression {
  unique_ptr<Block> block;
  shared_ptr<Scope> scope;
  ObjectInitializer(unique_ptr<Block> block) : block(std::move(block)) {}
  Value Evaluate() override {
    auto obj = make_shared<Object_T>();
    auto controlChange = block->Execute().controlChange;
    if (controlChange != ControlChange::None) {
      throw std::runtime_error(CC_ToString(controlChange) + " not allowed in object initialization.");
    }
    obj->scope = block->scope;
    return obj;
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
    if (lvalue->type == ValueType::Callable) {
      auto callable = static_cast<Callable_T*>(lvalue.get());
      return callable->Call(std::move(args));
    } else if (auto id = dynamic_cast<Identifier* >(operand.get())) {
      if (NativeFunctions::GetRegistry().count(id->name) > 0) {
        return NativeFunctions::GetRegistry()[id->name](GetArgsValueList(this->args));
      }
    }
    return Value_T::Undefined;
  }
  ExecutionResult Execute() override {
    Evaluate();
    return ExecutionResult::None;
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
  ExecutionResult Execute() override;
};
struct Else : Statement {
  unique_ptr<If> ifStmnt;
  unique_ptr<Block> block;

  static unique_ptr<Else> NoIf(unique_ptr<Block> &&block);
  static unique_ptr<Else> New(unique_ptr<If> &&ifStmnt);

  ExecutionResult Execute() override {
    if (ifStmnt != nullptr) {
      return ifStmnt->Execute();
    } else {
      return block->Execute();
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
  
  ExecutionResult Execute() override {
    context.PushScope(scope);
    if (decl != nullptr) {
      auto result = decl->Execute();
      switch (result.controlChange) {
      case ControlChange::None:
        break;
      case ControlChange::Continue:
      case ControlChange::Break:
      case ControlChange::Goto:
      case ControlChange::ContinueLabel:
      case ControlChange::BreakLabel:
        throw std::runtime_error(CC_ToString(result.controlChange) + " not allowed in for initialization.");
      case ControlChange::Return:
      case ControlChange::Exception:
        return result;
      }
    }
    
    if (condition != nullptr) {
      while (true) {
        auto conditionResult = condition->Evaluate();
        
        if (conditionResult->type != ValueType::Bool) {
          return ExecutionResult::None;
        }
        
        auto b = static_cast<Bool_T *>(conditionResult.get());

        if (b->Equals(Bool_T::False)) {
          context.PopScope();
          return ExecutionResult::None;
        }
        
        auto result = block->Execute();
        switch (result.controlChange) {
        case ControlChange::None:
        case ControlChange::Continue:
          break;
        case ControlChange::Return:
        case ControlChange::Exception:
          context.PopScope();
          return result;
        case ControlChange::Break:
          context.PopScope();
          return ExecutionResult::None;
        case ControlChange::Goto:
        case ControlChange::ContinueLabel:
        case ControlChange::BreakLabel:
          // TODO: Check for label Here
          throw std::runtime_error(CC_ToString(result.controlChange) + " not implemented");
        }
        result = increment->Execute();
        switch (result.controlChange) {
        case ControlChange::None:
          break;
        case ControlChange::Continue:
        case ControlChange::Return:
        case ControlChange::Break:
          throw std::runtime_error(CC_ToString(result.controlChange) + " not allowed in for initialization.");
        case ControlChange::Exception:
          context.PopScope();
          return result;
        case ControlChange::Goto:
        case ControlChange::ContinueLabel:
        case ControlChange::BreakLabel:
          // TODO: Check for label Here
          throw std::runtime_error(CC_ToString(result.controlChange) + " not implemented");
        }
      }
    } else {
      while (true) {
        if (increment) {
          auto result = increment->Execute();
          switch (result.controlChange) {
          case ControlChange::None:
            break;
          case ControlChange::Continue:
          case ControlChange::Return:
          case ControlChange::Break:
            throw std::runtime_error(CC_ToString(result.controlChange) + " not allowed in for initialization.");
          case ControlChange::Exception:
            context.PopScope();
            return result;
          case ControlChange::Goto:
          case ControlChange::ContinueLabel:
          case ControlChange::BreakLabel:
            // TODO: Check for label Here
            throw std::runtime_error(CC_ToString(result.controlChange) + " not implemented");
          }
        }
        auto result = block->Execute();
        switch (result.controlChange) {
        case ControlChange::None:
        case ControlChange::Continue:
          break;
        case ControlChange::Return:
        case ControlChange::Exception:
          context.PopScope();
          return result;
        case ControlChange::Break:
          context.PopScope();
          return ExecutionResult::None;
        case ControlChange::Goto:
        case ControlChange::ContinueLabel:
        case ControlChange::BreakLabel:
          // TODO: Check for label Here
          throw std::runtime_error(CC_ToString(result.controlChange) + " not implemented");
        }
      }
    }
    context.PopScope();
    return ExecutionResult::None;
  }
};
struct Assignment : Statement {
  unique_ptr<Identifier> iden;
  unique_ptr<Expression> expr;
  Assignment(unique_ptr<Identifier> &&iden, unique_ptr<Expression> &&expr)
      : iden(std::move(iden)), expr(std::move(expr)) {}
  ExecutionResult Execute() override {
    context.Insert(iden->name, expr->Evaluate());
    return ExecutionResult::None;
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

  ExecutionResult Execute() override {
    auto callable =
        make_shared<Callable_T>(std::move(body), std::move(parameters));
    context.Insert(name->name, callable);
    return ExecutionResult::None;
  }
};
struct DotExpr : Expression {
  DotExpr(unique_ptr<Expression> &&left, unique_ptr<Expression> &&right)
      : left(std::move(left)), right(std::move(right)) {}
  unique_ptr<Expression> left;
  unique_ptr<Expression> right;
  Value Evaluate() override {
    auto leftValue = left->Evaluate();
    
    if (leftValue->type != ValueType::Object) {
      throw std::runtime_error("invalid lhs on dot operation");
    }
    ASTNode::context.PushScope(static_cast<Object_T*>(leftValue.get())->scope);
    auto result = right->Evaluate();
    ASTNode::context.PopScope();
    return result;
  }
  void Assign(Value value) {
    auto lvalue = left->Evaluate();
    
    if (lvalue->type != ValueType::Object) {
      throw std::runtime_error("invalid lhs on dot operation");
    }
    
    auto obj = static_cast<Object_T *>(lvalue.get());
    
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
  ExecutionResult Execute() override {
    if (auto dot = dynamic_cast<DotExpr*>(this->dot.get()))
      dot->Assign(value->Evaluate());
    return ExecutionResult::None;
  }
};
struct DotCallStmnt : Statement {
  DotCallStmnt(unique_ptr<Expression> &&dot) : dot(std::move(dot)) {}
  unique_ptr<Expression> dot;
  ExecutionResult Execute() override {
    dot->Evaluate();
    return ExecutionResult::None;
  }
};
struct Subscript : Expression {

  Subscript(unique_ptr<Expression> &&left, unique_ptr<Expression> &&idx)
      : left(std::move(left)), index(std::move(idx)) {}
  unique_ptr<Expression> left;
  unique_ptr<Expression> index;
  Value Evaluate() {
    auto lvalue = left->Evaluate();
    if (lvalue->type != ValueType::Array) {
      throw std::runtime_error("Cannot subscript a non array");
    }
    auto array = static_cast<Array_T *>(lvalue.get());
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
  ExecutionResult Execute() override {
    auto subscript = dynamic_cast<Subscript*>(this->subscript.get());
    
    if (!subscript) {
      throw std::runtime_error("invalid subscript");
    }
    
    auto lvalue = subscript->left->Evaluate();
    auto idx = subscript->index->Evaluate();
    
    auto array = static_cast<Array_T *>(lvalue.get());
    auto number = std::dynamic_pointer_cast<Int_T>(idx);
    
    if (array->type != ValueType::Array || number->type != ValueType::Int) {
      throw std::runtime_error(
          "cannot subscript a non array or with a non-integer value.");
    }

    array->Assign(number, value->Evaluate());
    return ExecutionResult::None;
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
    auto right = this->right->Evaluate();
    
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