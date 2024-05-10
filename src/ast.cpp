#include "ast.hpp"
#include "native.hpp"
#include "value.hpp"
#include <stdexcept>
#include <string>
#include "debug.hpp"
#include "context.hpp"

Context ASTNode::context = {};
auto ExecutionResult::None =
    ExecutionResult(ControlChange::None, Value_T::Undefined);
auto ExecutionResult::Break =
    ExecutionResult(ControlChange::Break, Value_T::Undefined);
auto ExecutionResult::Continue =
    ExecutionResult(ControlChange::Continue, Value_T::Undefined);

string CC_ToString(ControlChange controlChange) {
  switch (controlChange) {
  case ControlChange::None:
    return "None";
  case ControlChange::Return:
    return "Return";
  case ControlChange::Continue:
    return "Continue";
  case ControlChange::Break:
    return "Break";
  }
}
ExecutionResult::ExecutionResult(ControlChange controlChange, Value value) {
  this->controlChange = controlChange;
  this->value = value;
}
If::If(const int &loc, const int &col, ExpressionPtr &&condition, BlockPtr &&block, ElsePtr &&elseStmnt): Statement(loc,col) {
  this->condition = std::move(condition);
  this->block = std::move(block);
  this->elseStmnt = std::move(elseStmnt);
}
If::If(const int &loc, const int &col, ExpressionPtr &&condition, BlockPtr &&block) : Statement(loc,col) {
  this->condition = std::move(condition);
  this->block = std::move(block);
}
Arguments::Arguments(const int &loc, const int &col, vector<ExpressionPtr> &&args) : Expression(loc,col) {
  this->values = std::move(args);
}
Parameters::Parameters(const int &loc, const int &col, vector<string> &&names) : Statement(loc,col) {
  this->names = std::move(names);
}
Identifier::Identifier(const int &loc, const int &col, string &name) : Expression(loc, col) {
  this->name = name;
}
Operand::Operand(const int &loc, const int &col, Value value) : Expression(loc, col) {
  this->value = value;
}
Program::Program(vector<StatementPtr> &&statements) : Executable(0,0) {
  this->statements = std::move(statements);
}
Return::Return(const int &loc, const int &col, ExpressionPtr &&value) : Statement(loc,col) {
  this->value = std::move(value);
}
Block::Block(const int &loc, const int &col, vector<StatementPtr> &&statements) : Statement(loc,col) {
  this->statements = std::move(statements);
}
ObjectInitializer::ObjectInitializer(const int &loc, const int &col, BlockPtr block) : Expression(loc, col) {
  this->block = std::move(block);
}
Call::Call(const int &loc, const int &col, ExpressionPtr &&operand, ArgumentsPtr &&args) : Expression(loc,col), Statement(loc,col) {
  this->operand = std::move(operand);
  this->args = std::move(args);
}
For::For(const int &loc, const int &col, StatementPtr &&decl, ExpressionPtr &&condition, StatementPtr &&inc, BlockPtr &&block, Scope scope) : Statement(loc,col) {
  this->decl = std::move(decl);
  this->condition = std::move(condition);
  this->increment = std::move(inc);
  this->block = std::move(block);
  this->scope = scope;
}
Assignment::Assignment(const int &loc, const int &col, IdentifierPtr &&iden, ExpressionPtr &&expr) : Statement(loc,col) {
  this->iden = std::move(iden);
  this->expr = std::move(expr);
}

DotExpr::DotExpr(const int &loc, const int &col, ExpressionPtr &&left, ExpressionPtr &&right) : Expression(loc,col) {
  this->left = std::move(left);
  this->right = std::move(right);
}
DotAssignment::DotAssignment(const int &loc, const int &col, ExpressionPtr &&dot, ExpressionPtr &&value): Statement(loc,col)  {
  this->dot = std::move(dot);
  this->value = std::move(value);
}
DotCallStmnt::DotCallStmnt(const int &loc, const int &col, ExpressionPtr &&dot) : Statement(loc,col) {
  this->dot = std::move(dot);
}
Subscript::Subscript(const int &loc, const int &col, ExpressionPtr &&left, ExpressionPtr &&idx) : Expression(loc,col)  {
  this->left = std::move(left);
  this->index = std::move(idx);
}
SubscriptAssignStmnt::SubscriptAssignStmnt(const int &loc, const int &col, ExpressionPtr &&subscript, ExpressionPtr &&value) : Statement(loc,col)  {
  this->subscript = std::move(subscript);
  this->value = std::move(value);
}
UnaryExpr::UnaryExpr(const int &loc, const int &col, ExpressionPtr &&left, TType op) : Expression(loc,col)  {
  this->left = std::move(left);
  this->op = op;
}
BinExpr::BinExpr(const int &loc, const int &col, ExpressionPtr &&left, ExpressionPtr &&right, TType op) : Expression(loc,col)  {
  this->left = std::move(left);
  this->right = std::move(right);
  this->op = op;
}
ExecutionResult If::Execute() {
  auto condResult = condition->Evaluate();
  if (condResult->Equals(Value_T::True)) {
    auto result = block->Execute();
    switch (result.controlChange) {
    case ControlChange::None:
      break;
    case ControlChange::Return:
    case ControlChange::Continue:
    case ControlChange::Break:
      return result;
    }
  }
  else if (elseStmnt) {
    auto result = elseStmnt->Execute();
    switch (result.controlChange) {
    case ControlChange::None:
      break;
    case ControlChange::Return:
    case ControlChange::Continue:
    case ControlChange::Break:
      return result;
    }
  }
  return ExecutionResult::None;
}
IfPtr If::NoElse(const int &loc, const int &col, ExpressionPtr &&condition, BlockPtr &&block) {
  return make_unique<If>(loc,col, std::move(condition), std::move(block));
}
IfPtr If::WithElse(const int &loc, const int &col, ExpressionPtr &&condition, BlockPtr &&block, ElsePtr &&elseStmnt) {
  return make_unique<If>(loc,col, std::move(condition), std::move(block),
                         std::move(elseStmnt));
}
ElsePtr Else::New(const int &loc, const int &col, IfPtr &&ifStmnt) {
  auto elseStmnt = make_unique<Else>(loc,col);
  elseStmnt->ifStmnt = std::move(ifStmnt);
  return elseStmnt;
}
ElsePtr Else::NoIf(const int &loc, const int &col, BlockPtr &&block) {
  auto elseStmnt = make_unique<Else>(loc,col);
  ;
  elseStmnt->block = std::move(block);
  return elseStmnt;
}
Import::Import(const int &loc, const int &col, const string &name, const bool isWildcard) : Statement(loc,col), symbols({}), moduleName(name), isWildcard(isWildcard) {
  
};
Import::Import(const int &loc, const int &col, const string &name, vector<string> &symbols)
    : Statement(loc,col), symbols(symbols), moduleName(name), isWildcard(false){};

UnaryStatement::UnaryStatement(const int &loc, const int &col, ExpressionPtr &&expr) : Statement(loc,col), expr(std::move(expr)) {}

CompoundAssignment::CompoundAssignment(const int &loc, const int &col, ExpressionPtr &&cmpAssignExpr)
    : Statement(loc,col),  expr(std::move(cmpAssignExpr)) {}
RangeBasedFor::RangeBasedFor(const int &loc, const int &col, IdentifierPtr &&lhs, ExpressionPtr &&rhs,
                             BlockPtr &&block)
    : Statement(loc,col), valueName(std::move(lhs)), rhs(std::move(rhs)), block(std::move(block)) {}
CompAssignExpr::CompAssignExpr(const int &loc, const int &col,
                               ExpressionPtr &&left, ExpressionPtr &&right,
                               TType op)
    : Expression(loc, col), left(std::move(left)), right(std::move(right)),
      op(op) {}
    
ExecutionResult Program::Execute() {
  for (auto &statement : statements) {
    Debug::WaitForBreakpoint(statement.get());
    try {
      auto result = statement->Execute();
      switch (result.controlChange) {
      case ControlChange::None:
        continue;
      default:
        throw std::runtime_error("Uncaught " + CC_ToString(result.controlChange));
      }
    }
    catch (std::runtime_error err) {
      std::cout << err.what() << std::endl;
    }
  }
  return ExecutionResult::None;
}
Value Operand::Evaluate() {
  return value;
}
Value Identifier::Evaluate() {
  if (name == "this") {
    return Ctx::CreateObject(ASTNode::context.scopes.back());
  }
  auto value = ASTNode::context.Find(name);
  if (value != nullptr) {
    return value;
  } else if (NativeFunctions::Exists(name)) {
    return NativeFunctions::GetCallable(name);
  }
  return Value_T::Undefined;
}
Value Arguments::Evaluate() {
  // do nothing here.
  return Value_T::Null;
}
ExecutionResult Parameters::Execute() {
  return ExecutionResult::None;
}
ExecutionResult Continue::Execute() {
  return ExecutionResult::Continue;
}
ExecutionResult Break::Execute() {
  return ExecutionResult::Break;
}
ExecutionResult Return::Execute() {
  return ExecutionResult(ControlChange::Return, value->Evaluate());
}
ExecutionResult Block::Execute() {
  scope = ASTNode::context.PushScope();
  for (auto &statement : statements) {
    Debug::WaitForBreakpoint(statement.get());
    try {
      auto result = statement->Execute();
      switch (result.controlChange) {
      case ControlChange::Continue:
      case ControlChange::Break:
      case ControlChange::Return:
        ASTNode::context.PopScope();
        return result;
      case ControlChange::None:
        continue;
      }
    } catch (std::runtime_error err) {
      std::cout << err.what() << std::endl;
    }
  }
  ASTNode::context.PopScope();
  return ExecutionResult::None;
}
Value ObjectInitializer::Evaluate() {
  auto controlChange = block->Execute().controlChange;
  if (controlChange != ControlChange::None) {
    throw std::runtime_error(CC_ToString(controlChange) +
                             " not allowed in object initialization.");
  }
  return Object_T::New(block->scope);
}
vector<Value> Call::GetArgsValueList(ArgumentsPtr &args) {
  vector<Value> values = {};
  for (auto &expr : args->values) {
    values.push_back(expr->Evaluate());
  }
  return values;
}
Value Call::Evaluate() {
  auto lvalue = operand->Evaluate();
  if (lvalue->GetType() == ValueType::Callable) {
    auto callable = static_cast<Callable_T *>(lvalue.get());
    return callable->Call(args);
  }
  return Value_T::Undefined;
}
ExecutionResult Call::Execute() {
  Evaluate();
  return ExecutionResult::None;
}
ExecutionResult Else::Execute() {
  if (ifStmnt != nullptr) {
    return ifStmnt->Execute();
  } else {
    return block->Execute();
  }
}
ExecutionResult For::Execute() {
  context.PushScope(scope);
  if (decl != nullptr) {
    auto result = decl->Execute();
    switch (result.controlChange) {
    case ControlChange::None:
      break;
    case ControlChange::Continue:
    case ControlChange::Break:
    case ControlChange::Return:
      return result;
    }
  }

  if (condition != nullptr) {
    while (true) {
      auto conditionResult = condition->Evaluate();

      if (conditionResult->GetType() != ValueType::Bool) {
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
      case ControlChange::Break:
        context.PopScope();
        return ExecutionResult::None;
      }
      result = increment->Execute();
      switch (result.controlChange) {
      case ControlChange::None:
        break;
      case ControlChange::Continue:
      case ControlChange::Return:
      case ControlChange::Break:
        throw std::runtime_error(CC_ToString(result.controlChange) +
                                 " not allowed in for initialization.");
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
          throw std::runtime_error(CC_ToString(result.controlChange) +
                                   " not allowed in for initialization.");
        }
      }
      auto result = block->Execute();
      switch (result.controlChange) {
      case ControlChange::None:
      case ControlChange::Continue:
        break;
      case ControlChange::Return:
      case ControlChange::Break:
        context.PopScope();
        return ExecutionResult::None;
      }
    }
  }
  context.PopScope();
  return ExecutionResult::None;
}
ExecutionResult Assignment::Execute() {
  context.Insert(iden->name, expr->Evaluate());
  return ExecutionResult::None;
}

Value DotExpr::Evaluate() {
  auto lvalue = left->Evaluate();

  if (lvalue->GetType() != ValueType::Object) {
    throw std::runtime_error("invalid lhs on dot operation : " + TypeToString(lvalue->GetType()));
  }
  ASTNode::context.PushScope(static_cast<Object_T *>(lvalue.get())->scope);
  auto result = right->Evaluate();
  ASTNode::context.PopScope();
  return result;
}
void DotExpr::Assign(Value value) {
  auto lvalue = left->Evaluate();
  
  if (lvalue->GetType() != ValueType::Object) {
    throw std::runtime_error("invalid lhs on dot operation : " + TypeToString(lvalue->GetType()));
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
ExecutionResult DotAssignment::Execute() {
  if (auto dot = dynamic_cast<DotExpr *>(this->dot.get()))
    dot->Assign(value->Evaluate());
  return ExecutionResult::None;
}
ExecutionResult DotCallStmnt::Execute() {
  dot->Evaluate();
  return ExecutionResult::None;
}
Value Subscript::Evaluate() {
  auto lvalue = left->Evaluate();
  return lvalue->Subscript(index->Evaluate());
}
ExecutionResult SubscriptAssignStmnt::Execute() {
  auto subscript = dynamic_cast<Subscript *>(this->subscript.get());
  if (!subscript) {
    throw std::runtime_error("invalid subscript");
  }
  auto lvalue = subscript->left->Evaluate();
  auto idx = subscript->index->Evaluate();
  lvalue->SubscriptAssign(idx, value->Evaluate());
  return ExecutionResult::None;
}
Value UnaryExpr::Evaluate() {
  auto lvalue = left->Evaluate();
  switch (op) {
  case TType::Sub:
    return lvalue->Negate();
  case TType::Not:
    return lvalue->Not();
  case TType::Increment:
    lvalue->Set(lvalue->Add(Ctx::CreateInt(1)));
    return lvalue;
  case TType::Decrement:
    lvalue->Set(lvalue->Subtract(Ctx::CreateInt(1)));
    return lvalue;
  default:
    throw std::runtime_error("invalid operator in unary expression " +
                             TTypeToString(op));
  }
  return Value_T::Null;
}
Value BinExpr::Evaluate() {
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
  case TType::GreaterEq:
    return left->GreaterEquals(right);
  case TType::LessEq:
    return left->LessEquals(right);
  case TType::Equals:
    return Bool_T::New(left->Equals(right));
  case TType::NotEquals: {
    auto result = left->Equals(right);
    return Bool_T::New(!result);
  }
  case TType::Assign:
    left->Set(right);
    return left;
  default:
    throw std::runtime_error("invalid operator in binary expresison " +
                             TTypeToString(op));
  }
};
ExecutionResult Import::Execute() {
  auto path = moduleRoot + moduleName + ".dll";
  auto module = LoadScritModule(moduleName, path);
  // we do this even when we ignore the object becasue it registers the native
  // callables.
  auto object = ScritModDefAsObject(module);
  if (!isWildcard && symbols.empty()) {
    ASTNode::context.Insert(moduleName, object);
  } else {
    vector<Value> values = {};
    
    if (!symbols.empty()) {
      for (const auto &name : symbols) {
        auto value = module->context->Find(name);
        if (value == Value_T::Undefined) {
          throw std::runtime_error("invalid import statement. could not find " + name);
        }
        
        ASTNode::context.Insert(name, value);
      }
    } else {
      for (const auto &[key, var] : object->scope->variables) {
        ASTNode::context.Insert(key, var);
      }
    }
  }
  delete module;
  return ExecutionResult::None;
}


ExecutionResult RangeBasedFor::Execute() {
  ASTNode::context.PushScope();
  auto lvalue = this->rhs->Evaluate();
  auto name = valueName->name;
  Array array;
  Object obj;
  string string;
  bool isString = false;
  if (Ctx::TryGetString(lvalue, string)) {
    isString = true;
  }
  
  if (!isString && !Ctx::TryGetArray(lvalue, array) && !Ctx::TryGetObject(lvalue, obj)) {
    throw std::runtime_error("invalid range-based for loop: the target container must be an array, object or string.");
  } 
  if (array) {
    for (auto &v: array->values) {
      ASTNode::context.Insert(name, v);
      auto result = block->Execute();
      switch (result.controlChange) {
      case ControlChange::None:
        break;
      case ControlChange::Return:
        return result;
      case ControlChange::Continue:
        continue;
      case ControlChange::Break:
        goto breakLoops;
        break;
      }
    }   
  } else if (obj) {
    auto kvp = Ctx::CreateObject();
    for (auto &[key, val] : obj->scope->variables) {
      kvp->scope->variables["key"] = Ctx::CreateString(key);
      kvp->scope->variables["value"] = val;
      ASTNode::context.Insert(name, kvp);
      auto result = block->Execute();
      switch (result.controlChange) {
      case ControlChange::None:
        break;
      case ControlChange::Return:
        return result;
      case ControlChange::Continue:
        continue;
      case ControlChange::Break:
        goto breakLoops;
      }
    }   
  } else if (isString) {
     for (auto c : string) {
      ASTNode::context.Insert(name, Ctx::CreateString(std::string() + c));
      auto result = block->Execute();
      switch (result.controlChange) {
      case ControlChange::None:
        break;
      case ControlChange::Return:
        return result;
      case ControlChange::Continue:
        continue;
      case ControlChange::Break:
        goto breakLoops;
      }
    }   
  }
  breakLoops:
  ASTNode::context.PopScope();
  return ExecutionResult::None;
}

ExecutionResult CompoundAssignment::Execute() {
  auto _ = expr->Evaluate();
  return ExecutionResult::None;
}


Value CompAssignExpr::Evaluate() {
  auto lvalue = left->Evaluate();
  auto rvalue = right->Evaluate();
  switch (op) {
  case TType::AddEq:
    lvalue->Set(lvalue->Add(rvalue));
    return lvalue;
    break;
  case TType::DivEq:
    lvalue->Set(lvalue->Divide(rvalue));
    return lvalue;
    break;
  case TType::SubEq:
    lvalue->Set(lvalue->Subtract(rvalue));
    return lvalue;
    break;
  case TType::MulEq:
    lvalue->Set(lvalue->Multiply(rvalue));
    return lvalue;
    break;
  default:
    throw std::runtime_error("invalid operator : " + TTypeToString(op) +
                             " in compound assignment statement");
  }
}
