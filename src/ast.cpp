#include "ast.hpp"
#include "native.hpp"
#include "value.hpp"
#include <stdexcept>
#include "context.hpp"

Context ASTNode::context = {};
auto ExecutionResult::None =
    ExecutionResult(ControlChange::None, Value_T::Undefined);
auto ExecutionResult::Break =
    ExecutionResult(ControlChange::Break, Value_T::Undefined);
auto ExecutionResult::Continue =
    ExecutionResult(ControlChange::Continue, Value_T::Undefined);

static Object MakeException(const string &msg, const string &type) {
  auto e = make_shared<Object_T>(make_shared<Scope_T>());
  e->scope->variables["msg"] = make_shared<String_T>(msg);
  e->scope->variables["type"] = make_shared<String_T>(type);
  return e;
}
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
ExecutionResult::ExecutionResult(ControlChange controlChange, Value value) {
  this->controlChange = controlChange;
  this->value = value;
}
If::If(ExpressionPtr &&condition, BlockPtr &&block, ElsePtr &&elseStmnt) {
  this->condition = std::move(condition);
  this->block = std::move(block);
  this->elseStmnt = std::move(elseStmnt);
}
If::If(ExpressionPtr &&condition, BlockPtr &&block) {
  this->condition = std::move(condition);
  this->block = std::move(block);
}
Arguments::Arguments(vector<ExpressionPtr> &&args) {
  this->values = std::move(args);
}
Parameters::Parameters(vector<string> &&names) {
  this->names = std::move(names);
}
Identifier::Identifier(string &name) {
  this->name = name;
}
Operand::Operand(Value value) {
  this->value = value;
}
Program::Program(vector<StatementPtr> &&statements) {
  this->statements = std::move(statements);
}
Return::Return(ExpressionPtr &&value) {
  this->value = std::move(value);
}
Block::Block(vector<StatementPtr> &&statements) {
  this->statements = std::move(statements);
}
ObjectInitializer::ObjectInitializer(BlockPtr block) {
  this->block = std::move(block);
}
Call::Call(ExpressionPtr &&operand, ArgumentsPtr &&args) {
  this->operand = std::move(operand);
  this->args = std::move(args);
}
For::For(StatementPtr &&decl, ExpressionPtr &&condition, StatementPtr &&inc, BlockPtr &&block, Scope scope) {
  this->decl = std::move(decl);
  this->condition = std::move(condition);
  this->increment = std::move(inc);
  this->block = std::move(block);
  this->scope = scope;
}
Assignment::Assignment(IdentifierPtr &&iden, ExpressionPtr &&expr) {
  this->iden = std::move(iden);
  this->expr = std::move(expr);
}
FuncDecl::FuncDecl(IdentifierPtr &&name, BlockPtr &&body, ParametersPtr &&parameters) {
  this->name = std::move(name);
  this->body = std::move(body);
  this->parameters = std::move(parameters);
}
DotExpr::DotExpr(ExpressionPtr &&left, ExpressionPtr &&right) {
  this->left = std::move(left);
  this->right = std::move(right);
}
DotAssignment::DotAssignment(ExpressionPtr &&dot, ExpressionPtr &&value) {
  this->dot = std::move(dot);
  this->value = std::move(value);
}
DotCallStmnt::DotCallStmnt(ExpressionPtr &&dot) {
  this->dot = std::move(dot);
}
Subscript::Subscript(ExpressionPtr &&left, ExpressionPtr &&idx) {
  this->left = std::move(left);
  this->index = std::move(idx);
}
SubscriptAssignStmnt::SubscriptAssignStmnt(ExpressionPtr &&subscript, ExpressionPtr &&value) {
  this->subscript = std::move(subscript);
  this->value = std::move(value);
}
UnaryExpr::UnaryExpr(ExpressionPtr &&left, TType op) {
  this->left = std::move(left);
  this->op = op;
}
BinExpr::BinExpr(ExpressionPtr &&left, ExpressionPtr &&right, TType op) {
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
    case ControlChange::Exception:
    case ControlChange::Continue:
    case ControlChange::Break:
      return result;
    case ControlChange::Goto:
    case ControlChange::ContinueLabel:
    case ControlChange::BreakLabel:
      // TODO: Check for label Here
      throw std::runtime_error(CC_ToString(result.controlChange) +
                               " not implemented");
    }
  }
  else if (elseStmnt) {
    auto result = elseStmnt->Execute();
    switch (result.controlChange) {
    case ControlChange::None:
      break;
    case ControlChange::Return:
    case ControlChange::Exception:
    case ControlChange::Continue:
    case ControlChange::Break:
      return result;
    case ControlChange::Goto:
    case ControlChange::ContinueLabel:
    case ControlChange::BreakLabel:
      // TODO: Check for label Here
      throw std::runtime_error(CC_ToString(result.controlChange) +
                               " not implemented");
    }
  }
  return ExecutionResult::None;
}
IfPtr If::NoElse(ExpressionPtr &&condition, BlockPtr &&block) {
  return make_unique<If>(std::move(condition), std::move(block));
}
IfPtr If::WithElse(ExpressionPtr &&condition, BlockPtr &&block, ElsePtr &&elseStmnt) {
  return make_unique<If>(std::move(condition), std::move(block),
                         std::move(elseStmnt));
}
ElsePtr Else::New(IfPtr &&ifStmnt) {
  auto elseStmnt = make_unique<Else>();
  elseStmnt->ifStmnt = std::move(ifStmnt);
  return elseStmnt;
}
ElsePtr Else::NoIf(BlockPtr &&block) {
  auto elseStmnt = make_unique<Else>();
  ;
  elseStmnt->block = std::move(block);
  return elseStmnt;
}
ExecutionResult Program::Execute() {
  for (auto &statement : statements) {
    auto result = statement->Execute();
    switch (result.controlChange) {
    case ControlChange::Exception:
      throw std::runtime_error("Uncaught Exception: " +
                               result.value->ToString());
    case ControlChange::Goto:
    case ControlChange::ContinueLabel:
    case ControlChange::BreakLabel:
      // TODO: Check for label Here
      throw std::runtime_error(CC_ToString(result.controlChange) +
                               " not implemented");
    case ControlChange::None:
      continue;
    default:
      throw std::runtime_error("Uncaught " + CC_ToString(result.controlChange));
    }
  }
  return ExecutionResult::None;
}
Value Operand::Evaluate() {
  return value;
}
Value Identifier::Evaluate() {
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
    auto result = statement->Execute();
    switch (result.controlChange) {
    case ControlChange::Goto:
    case ControlChange::ContinueLabel:
    case ControlChange::BreakLabel:
      // TODO: Check for label Here
      throw std::runtime_error(CC_ToString(result.controlChange) +
                               " not implemented");
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
Value ObjectInitializer::Evaluate() {
  auto controlChange = block->Execute().controlChange;
  if (controlChange != ControlChange::None) {
    throw std::runtime_error(CC_ToString(controlChange) +
                             " not allowed in object initialization.");
  }
  return make_shared<Object_T>(block->scope);
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
    case ControlChange::Goto:
    case ControlChange::ContinueLabel:
    case ControlChange::BreakLabel:
      throw std::runtime_error(CC_ToString(result.controlChange) +
                               " not allowed in for initialization.");
    case ControlChange::Return:
    case ControlChange::Exception:
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
        throw std::runtime_error(CC_ToString(result.controlChange) +
                                 " not implemented");
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
      case ControlChange::Exception:
        context.PopScope();
        return result;
      case ControlChange::Goto:
      case ControlChange::ContinueLabel:
      case ControlChange::BreakLabel:
        // TODO: Check for label Here
        throw std::runtime_error(CC_ToString(result.controlChange) +
                                 " not implemented");
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
        case ControlChange::Exception:
          context.PopScope();
          return result;
        case ControlChange::Goto:
        case ControlChange::ContinueLabel:
        case ControlChange::BreakLabel:
          // TODO: Check for label Here
          throw std::runtime_error(CC_ToString(result.controlChange) +
                                   " not implemented");
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
        throw std::runtime_error(CC_ToString(result.controlChange) +
                                 " not implemented");
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
ExecutionResult FuncDecl::Execute() {
  auto callable =
      make_shared<Callable_T>(std::move(body), std::move(parameters));
  context.Insert(name->name, callable);
  return ExecutionResult::None;
}
Value DotExpr::Evaluate() {
  auto leftValue = left->Evaluate();

  if (leftValue->GetType() != ValueType::Object) {
    throw std::runtime_error("invalid lhs on dot operation");
  }
  ASTNode::context.PushScope(static_cast<Object_T *>(leftValue.get())->scope);
  auto result = right->Evaluate();
  ASTNode::context.PopScope();
  return result;
}
void DotExpr::Assign(Value value) {
  auto lvalue = left->Evaluate();

  if (lvalue->GetType() != ValueType::Object) {
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
  if (lvalue->GetType() != ValueType::Array) {
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
ExecutionResult SubscriptAssignStmnt::Execute() {
  auto subscript = dynamic_cast<Subscript *>(this->subscript.get());

  if (!subscript) {
    throw std::runtime_error("invalid subscript");
  }

  auto lvalue = subscript->left->Evaluate();
  auto idx = subscript->index->Evaluate();

  auto array = static_cast<Array_T *>(lvalue.get());
  auto number = std::dynamic_pointer_cast<Int_T>(idx);

  if (array->GetType() != ValueType::Array || number->GetType() != ValueType::Int) {
    throw std::runtime_error(
        "cannot subscript a non array or with a non-integer value.");
  }

  array->Assign(number, value->Evaluate());
  return ExecutionResult::None;
}
Value UnaryExpr::Evaluate() {
  auto lvalue = left->Evaluate();

  if (op == TType::Sub) {
    return lvalue->Negate();
  } else if (op == TType::Not) {
    return lvalue->Not();
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
  case TType::GreaterEQ:
    return left->GreaterEquals(right);
  case TType::LessEQ:
    return left->LessEquals(right);
  case TType::Equals:
    return make_shared<Bool_T>(left->Equals(right));
  case TType::NotEquals: {
    auto result = left->Equals(right);
    return make_shared<Bool_T>(!result);
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
  if (symbols.empty()) {
    ASTNode::context.Insert(moduleName, object);
  } else {
    vector<Value> values = {};
    for (const auto &name : symbols) {
      auto value = module->context->Find(name);
      ASTNode::context.Insert(name, value);
    }
  }
  delete module;
  return ExecutionResult::None;
}

Import::Import(const string &name) : moduleName(name), symbols({}) {};
Import::Import(const string &name, vector<string> &symbols) : moduleName(name), symbols(symbols){};