
#include "context.hpp"
#include "lexer.hpp"
#include "value.hpp"
#include <memory>
#include <stdexcept>

using std::unique_ptr;
using std::make_unique;

struct ASTNode  {
  static Context context;
  virtual ~ASTNode();
  virtual Value Evaluate() =0;
};

struct Expression {
  virtual ~Expression();
  virtual Value Evaluate() = 0;
};

struct Statement : ASTNode{
  virtual ~Statement();
  virtual Value Evaluate() =0;
};

struct Program : ASTNode {
  vector<unique_ptr<Statement>> statements;
  Program(vector<unique_ptr<Statement>> &&statements) : statements(std::move(statements)) {}
  Value Evaluate() override {
    Value result;
    for (auto &statement : statements) {
      result = statement->Evaluate();
    }
    return result;
  }
};


struct Operand : Expression {
  Operand(Value &value): value(value){}
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
  Value Evaluate() override {
    //do nothing here.
    return Value_T::Null;
  }
};

struct Call : Expression, Statement {
  unique_ptr<Operand> operand;
  unique_ptr<Arguments> args;
  
  Value Evaluate() override {
    auto lvalue = operand->Evaluate();
    if (auto callable = dynamic_cast<Callable_T*>(lvalue.get())) {
      return callable->Call(std::move(args));
    }
    return Value_T::Undefined;
  }
};

struct BinExpr : Expression {
  unique_ptr<Expression> left;
  unique_ptr<Expression> right;
   TType op;
  BinExpr(unique_ptr<Expression> left, unique_ptr<Expression> right, TType op) :
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