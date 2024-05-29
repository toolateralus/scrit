#include "ast.hpp"
#include "context.hpp"
#include "debug.hpp"
#include "native.hpp"
#include "parser.hpp"
#include "value.hpp"
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

auto programSourceInfo = SourceInfo{0, 0};

// Todo: add a scope that this was imported in so when we
// leave that scope we dlclose() the opened module.
// right now it just leaves it open till the end of the program.
std::vector<string> Import::importedModules = {};

Context ASTNode::context = {};
auto ExecutionResult::None =
    ExecutionResult(ControlChange::None, Value_T::UNDEFINED);
auto ExecutionResult::Break =
    ExecutionResult(ControlChange::Break, Value_T::UNDEFINED);
auto ExecutionResult::Continue =
    ExecutionResult(ControlChange::Continue, Value_T::UNDEFINED);

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
If::If(SourceInfo &info, ExpressionPtr &&condition, BlockPtr &&block,
       ElsePtr &&elseStmnt)
    : Statement(info) {
  this->condition = std::move(condition);
  this->block = std::move(block);
  this->elseStmnt = std::move(elseStmnt);
}
If::If(SourceInfo &info, ExpressionPtr &&condition, BlockPtr &&block)
    : Statement(info) {
  this->condition = std::move(condition);
  this->block = std::move(block);
}
Arguments::Arguments(SourceInfo &info, vector<ExpressionPtr> &&args)
    : Expression(info) {
  this->values = std::move(args);
}
Parameters::Parameters(SourceInfo &info, std::map<string, Value> &&params)
    : Statement(info) {
  this->map = std::move(params);
}
Identifier::Identifier(SourceInfo &info, string &name) : Expression(info) {
  this->name = name;
}
Operand::Operand(SourceInfo &info, Value value) : Expression(info) {
  this->value = value;
}
Program::Program(vector<StatementPtr> &&statements)
    : Executable(programSourceInfo) {
  this->statements = std::move(statements);
}
Return::Return(SourceInfo &info, ExpressionPtr &&value) : Statement(info) {
  this->value = std::move(value);
}
Block::Block(SourceInfo &info, vector<StatementPtr> &&statements)
    : Statement(info) {
  this->statements = std::move(statements);
}
ObjectInitializer::ObjectInitializer(SourceInfo &info, BlockPtr &&block,
                                     Scope scope)
    : Expression(info) {
  this->scope = make_shared<Scope_T>(scope.get());
  this->block = std::move(block);
}
Call::Call(SourceInfo &info, ExpressionPtr &&operand, ArgumentsPtr &&args)
    : Expression(info), Statement(info) {
  this->operand = std::move(operand);
  this->args = std::move(args);
}
For::For(SourceInfo &info, StatementPtr &&decl, ExpressionPtr &&condition,
         StatementPtr &&inc, BlockPtr &&block, Scope scope)
    : Statement(info) {
  this->decl = std::move(decl);
  this->condition = std::move(condition);
  this->increment = std::move(inc);
  this->block = std::move(block);
  this->scope = scope;
}
Assignment::Assignment(SourceInfo &info, IdentifierPtr &&iden,
                       ExpressionPtr &&expr)
    : Statement(info) {
  this->iden = std::move(iden);
  this->expr = std::move(expr);
}

DotExpr::DotExpr(SourceInfo &info, ExpressionPtr &&left, ExpressionPtr &&right)
    : Expression(info) {
  this->left = std::move(left);
  this->right = std::move(right);
}
DotAssignment::DotAssignment(SourceInfo &info, ExpressionPtr &&dot,
                             ExpressionPtr &&value)
    : Statement(info) {
  this->dot = std::move(dot);
  this->value = std::move(value);
}
DotCallStmnt::DotCallStmnt(SourceInfo &info, ExpressionPtr &&dot)
    : Statement(info) {
  this->dot = std::move(dot);
}
Subscript::Subscript(SourceInfo &info, ExpressionPtr &&left,
                     ExpressionPtr &&idx)
    : Expression(info) {
  this->left = std::move(left);
  this->index = std::move(idx);
}
SubscriptAssignStmnt::SubscriptAssignStmnt(SourceInfo &info,
                                           ExpressionPtr &&subscript,
                                           ExpressionPtr &&value)
    : Statement(info) {
  this->subscript = std::move(subscript);
  this->value = std::move(value);
}
UnaryExpr::UnaryExpr(SourceInfo &info, ExpressionPtr &&left, TType op)
    : Expression(info) {
  this->left = std::move(left);
  this->op = op;
}
BinExpr::BinExpr(SourceInfo &info, ExpressionPtr &&left, ExpressionPtr &&right,
                 TType op)
    : Expression(info) {
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
  } else if (elseStmnt) {
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
IfPtr If::NoElse(SourceInfo &info, ExpressionPtr &&condition,
                 BlockPtr &&block) {
  return make_unique<If>(info, std::move(condition), std::move(block));
}
IfPtr If::WithElse(SourceInfo &info, ExpressionPtr &&condition,
                   BlockPtr &&block, ElsePtr &&elseStmnt) {
  return make_unique<If>(info, std::move(condition), std::move(block),
                         std::move(elseStmnt));
}
ElsePtr Else::New(SourceInfo &info, IfPtr &&ifStmnt) {
  auto elseStmnt = make_unique<Else>(info);
  elseStmnt->ifStmnt = std::move(ifStmnt);
  return elseStmnt;
}
ElsePtr Else::NoIf(SourceInfo &info, BlockPtr &&block) {
  auto elseStmnt = make_unique<Else>(info);
  ;
  elseStmnt->block = std::move(block);
  return elseStmnt;
}
Import::Import(SourceInfo &info, const string &name, const bool isWildcard)
    : Statement(info), symbols({}), moduleName(name), isWildcard(isWildcard){

                                                      };
Import::Import(SourceInfo &info, const string &name, vector<string> &symbols)
    : Statement(info), symbols(symbols), moduleName(name), isWildcard(false){};

UnaryStatement::UnaryStatement(SourceInfo &info, ExpressionPtr &&expr)
    : Statement(info), expr(std::move(expr)) {}

CompoundAssignment::CompoundAssignment(SourceInfo &info,
                                       ExpressionPtr &&cmpAssignExpr)
    : Statement(info), expr(std::move(cmpAssignExpr)) {}
RangeBasedFor::RangeBasedFor(SourceInfo &info, IdentifierPtr &&lhs,
                             ExpressionPtr &&rhs, BlockPtr &&block)
    : Statement(info), valueName(std::move(lhs)), rhs(std::move(rhs)),
      block(std::move(block)) {}
CompAssignExpr::CompAssignExpr(SourceInfo &info, ExpressionPtr &&left,
                               ExpressionPtr &&right, TType op)
    : Expression(info), left(std::move(left)), right(std::move(right)), op(op) {
}

ExecutionResult Program::Execute() {
  int index = 0;
  for (auto &statement : statements) {
    Debug::m_hangUpOnBreakpoint(this, statement.get());
    try {
      auto result = statement->Execute();
      switch (result.controlChange) {
      case ControlChange::None:
        continue;
      default:
        throw std::runtime_error("Uncaught " +
                                 CC_ToString(result.controlChange));
      }
    } catch (std::runtime_error err) {
      std::cout << err.what() << std::endl;
    }
    index++;
  }
  return ExecutionResult::None;
}
Value Operand::Evaluate() { return value; }
Value Identifier::Evaluate() {
  // if (name == "this") {
  //   return Ctx::CreateObject(ASTNode::context.scopes.back());
  // }
  auto value = ASTNode::context.Find(name);
  if (value != nullptr) {
    return value;
  } else if (NativeFunctions::Exists(name)) {
    return NativeFunctions::GetCallable(name);
  }
  return Value_T::UNDEFINED;
}
Value Arguments::Evaluate() {
  // do nothing here.
  return Value_T::VNULL;
}
ExecutionResult Parameters::Execute() { return ExecutionResult::None; }
ExecutionResult Continue::Execute() { return ExecutionResult::Continue; }
ExecutionResult Break::Execute() { return ExecutionResult::Break; }
ExecutionResult Return::Execute() {
  return ExecutionResult(ControlChange::Return, value->Evaluate());
}
ExecutionResult Block::Execute() {
  if (scope == nullptr)
    scope = ASTNode::context.PushScope();
  else {
    ASTNode::context.PushScope(scope);
  }

  int index = 0;
  for (auto &statement : statements) {
    Debug::m_hangUpOnBreakpoint(this, statement.get());
    try {
      auto result = statement->Execute();
      switch (result.controlChange) {
      case ControlChange::Continue:
      case ControlChange::Break:
      case ControlChange::Return:
        ASTNode::context.PopScope();
        switch (result.value->GetType()) {
        case Values::ValueType::Invalid:
        case Values::ValueType::Null:
        case Values::ValueType::Undefined:
        case Values::ValueType::Object:
        case Values::ValueType::Array:
        case Values::ValueType::Callable:
        case Values::ValueType::Any:
          return result;
        case Values::ValueType::Float:
        case Values::ValueType::Int:
        case Values::ValueType::Bool:
        case Values::ValueType::String:
          result.value = result.value->Clone();
          return result;
        }
        return result;
      case ControlChange::None:
        continue;
      }
    } catch (std::runtime_error err) {
      std::cout << err.what() << std::endl;
    }
    index++;
  }
  ASTNode::context.PopScope();
  return ExecutionResult::None;
}
Value ObjectInitializer::Evaluate() {
  block->scope = this->scope;

  auto _this = Object_T::New();
  ;
  block->scope->variables["this"] = _this;

  auto controlChange = block->Execute().controlChange;

  if (controlChange != ControlChange::None) {
    throw std::runtime_error(CC_ToString(controlChange) +
                             " not allowed in object initialization.");
  }

  block->scope->variables.insert(_this->scope->variables.begin(),
                                 _this->scope->variables.end());
  block->scope->variables.erase("this");

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
    auto result = callable->Call(args);
    return result;
  }
  return Value_T::UNDEFINED;
}
ExecutionResult Call::Execute() {
  Evaluate();
  return ExecutionResult::None;
}
ExecutionResult Else::Execute() {
  if (ifStmnt != nullptr) {
    return ifStmnt->Execute();
  } else {
    // this should not be valid.
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
  auto result = expr->Evaluate();
  switch (result->GetType()) {
        case Values::ValueType::Invalid:
        case Values::ValueType::Null:
        case Values::ValueType::Undefined:
        case Values::ValueType::Object:
        case Values::ValueType::Array:
        case Values::ValueType::Callable:
        case Values::ValueType::Any:
          break;
          // clone data types.
        case Values::ValueType::Float:
        case Values::ValueType::Int:
        case Values::ValueType::Bool:
        case Values::ValueType::String:
          result = result->Clone();
        }
  
  context.Insert(iden->name, result);
  return ExecutionResult::None;
}

Value DotExpr::Evaluate() {
  auto lvalue = left->Evaluate();

  if (lvalue->GetType() != ValueType::Object) {
    throw std::runtime_error("invalid lhs on dot operation : " +
                             TypeToString(lvalue->GetType()));
  }
  auto object = static_cast<Object_T *>(lvalue.get());

  auto scope = object->scope;

  scope->variables["this"] = lvalue;
  ASTNode::context.PushScope(scope);

  auto result = right->Evaluate();
  ASTNode::context.PopScope();

  scope->variables.erase("this");

  return result;
}
void DotExpr::Assign(Value value) {
  auto lvalue = left->Evaluate();

  if (lvalue->GetType() != ValueType::Object) {
    throw std::runtime_error("invalid lhs on dot operation : " +
                             TypeToString(lvalue->GetType()));
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
  return Value_T::VNULL;
}
Value BinExpr::Evaluate() {
  auto left = this->left->Evaluate();
  auto right = this->right->Evaluate();

  switch (op) {
  case TType::NullCoalescing: {
    if (left->GetType() == ValueType::Null ||
        left->GetType() == ValueType::Undefined) {
      return right;
    }
    return left;
  };
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
  for (const auto &mod : importedModules) {
    if (moduleName == mod) {
      return ExecutionResult::None;
    }
  }

  importedModules.push_back(moduleName);

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
        if (value == Value_T::UNDEFINED) {
          throw std::runtime_error("invalid import statement. could not find " +
                                   name);
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

  if (!isString && !Ctx::TryGetArray(lvalue, array) &&
      !Ctx::TryGetObject(lvalue, obj)) {
    throw std::runtime_error("invalid range-based for loop: the target "
                             "container must be an array, object or string.");
  }
  if (array) {
    for (auto &v : array->values) {
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

  auto iden = dynamic_cast<Identifier *>(left.get());

  auto rvalue = right->Evaluate();
  switch (op) {
  case TType::AddEq: {
    auto result = lvalue->Add(rvalue);
    if (iden) {
      context.Insert(iden->name, result);
    } else {
      lvalue->Set(result);
    }
    return lvalue;
  }
  case TType::DivEq: {
    auto result = lvalue->Divide(rvalue);
    if (iden) {
      context.Insert(iden->name, result);
    } else {
      lvalue->Set(result);
    }
    return lvalue;
  }
  case TType::SubEq: {
    auto result = lvalue->Subtract(rvalue);
    if (iden) {
      context.Insert(iden->name, result);
    } else {
      lvalue->Set(result);
    }
    return lvalue;
  }
  case TType::MulEq: {
    auto result = lvalue->Multiply(rvalue);
    if (iden) {
      context.Insert(iden->name, result);
    } else {
      lvalue->Set(result);
    }
    return lvalue;
  }
  case TType::NullCoalescingEq: {
    if (lvalue->GetType() == ValueType::Undefined ||
        lvalue->GetType() == ValueType::Null) {
      auto result = rvalue;
      if (iden) {
        context.Insert(iden->name, rvalue);
      } else {
        lvalue->Set(rvalue);
      }
    }
    return lvalue;
  }
  default:
    throw std::runtime_error("invalid operator : " + TTypeToString(op) +
                             " in compound assignment statement");
  }
}
Else::Else(SourceInfo &info, IfPtr &&ifPtr, BlockPtr &&block)
    : Statement(info) {
  this->ifStmnt = std::move(ifPtr);
  this->block = std::move(block);
}
