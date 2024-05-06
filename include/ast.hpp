
#include "context.hpp"
#include "lexer.hpp"
#include "value.hpp"
#include <memory>
#include <stdexcept>

using std::unique_ptr;
using std::make_unique;

struct ASTNode  {
  static Context context;
  virtual ~ASTNode() {
    
  }
  virtual Value Evaluate() =0;
};

struct Expression {
  virtual ~Expression() {
    
  }
  virtual Value Evaluate() = 0;
};

struct Statement : ASTNode{
  virtual ~Statement() {
    
  }
  virtual Value Evaluate() override {
    throw std::runtime_error("Cannot evaluate a statement to a value");
  }
  virtual unique_ptr<ASTNode> EvaluateStatement() = 0;
};

struct Program : ASTNode {
  vector<unique_ptr<Statement>> statements;
  Program(vector<unique_ptr<Statement>> &&statements) : statements(std::move(statements)) {}
  Value Evaluate() override {
    Value result;
    for (auto &statement : statements) {
      statement->EvaluateStatement();
    }
    return result;
  }
};


struct Operand : Expression {
  Operand(Value value): value(value){}
  Value value;
  Value Evaluate() override {
    return value;
  }
};


struct Identifier : Expression {
  Identifier(string &name): name(name){}
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
  Arguments(vector<unique_ptr<Expression>> &&args) : values(std::move(args)){}
  vector<unique_ptr<Expression>> values;
  Value Evaluate() override {
    //do nothing here.
    return Value_T::Null;
  }
};

struct Parameters : Statement {
  vector<string> names;
  Parameters(vector<string> &&names) : names(std::move(names)){} 
  unique_ptr<ASTNode> EvaluateStatement() override {
    return nullptr;
  }
};

struct Continue : Statement {
  unique_ptr<ASTNode> EvaluateStatement() override {
    return nullptr;
  }
};

struct Break : Statement {
  unique_ptr<ASTNode> EvaluateStatement() override {
    return nullptr;
  }
};

struct Return : Statement {
  Return(unique_ptr<Expression> &&value) : value(std::move(value)) {}
  unique_ptr<Expression> value;
  Value Evaluate() override {
    return value->Evaluate();
  }
  unique_ptr<ASTNode> EvaluateStatement() override {
    return nullptr;
  }
};

struct Block : Statement  {
  Block(vector<unique_ptr<Statement>> &&statements) : statements(std::move(statements)) {
    
  }
  vector<unique_ptr<Statement>> statements;
  unique_ptr<ASTNode> EvaluateStatement() override {
    ASTNode::context.PushScope();
    for (auto &statement : statements) {
      if (dynamic_cast<Return*>(statement.get()) || dynamic_cast<Break*>(statement.get()) || dynamic_cast<Continue*>(statement.get())) {
        auto value = statement->Evaluate();
        // todo: figure out how to return values. we might need to just have 
        // a field called returnValeu or something and get it where w ewant it
        return nullptr;
      }else {
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
  ObjectInitializer(unique_ptr<Block> block) : block(std::move(block)) {
    
  }
  Value Evaluate() override {
    scope = ASTNode::context.PushScope();
    auto value = block->Evaluate();
    ASTNode::context.PopScope();
    return value;
  }
};

struct Call : Expression, Statement {
  unique_ptr<Operand> operand;
  unique_ptr<Arguments> args;
  
  Call(unique_ptr<Operand> &&operand, unique_ptr<Arguments> &&args) : operand(std::move(operand)), args(std::move(args)) {
    
  }
  
  Value Evaluate() override {
    auto lvalue = operand->Evaluate();
    if (auto callable = dynamic_cast<Callable_T*>(lvalue.get())) {
      return callable->Call(std::move(args));
    }
    return Value_T::Undefined;
  }
  unique_ptr<ASTNode> EvaluateStatement() override {
    Evaluate();
    return nullptr;
  }
};

struct UnaryExpr : Expression {
  UnaryExpr(unique_ptr<Expression> &&left, TType op) :
  left(std::move(left)), op(op){
  }
  unique_ptr<Expression> left;
  TType op;
  Value Evaluate() override {
    auto lvalue = left->Evaluate();
    
    if (op == TType::Sub) {
      return lvalue->Negate();
    } else if (op == TType::Not) {
      return lvalue->Not();
    }
    
    
  }
};

struct BinExpr : Expression {
  unique_ptr<Expression> left;
  unique_ptr<Expression> right;
   TType op;
  BinExpr(unique_ptr<Expression> &&left, unique_ptr<Expression> &&right, TType op) :
  left(std::move(left)), right(std::move(right)), op(op){
    
  }
  
  Value Evaluate() override {
    auto left = this->left->Evaluate();
    auto right = this->left->Evaluate();
    
    switch(op) {
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
        throw std::runtime_error("invalid operator in binary expresison " + TTypeToString(op));
    }
  };
};