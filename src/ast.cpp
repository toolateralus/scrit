#include "ast.hpp"
#include "ast_visitor.hpp"
#include "context.hpp"
#include "ctx.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "native.hpp"
#include "parser.hpp"
#include "value.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type.hpp>
#include <vector>

auto programSourceInfo = SourceInfo{0, 0, ""};

// Todo: add a scope that this was usinged in so when we
// leave that scope we dlclose() the opened module.
// right now it just leaves it open till the end of the program.
std::vector<string> Using::activeModules = {};

Context ASTNode::context = {};
ExecutionResult ExecutionResult::None =
    ExecutionResult(ControlChange::None, Value_T::UNDEFINED);
ExecutionResult ExecutionResult::Break =
    ExecutionResult(ControlChange::Break, Value_T::UNDEFINED);
ExecutionResult ExecutionResult::Continue =
    ExecutionResult(ControlChange::Continue, Value_T::UNDEFINED);

// ##############################################
// ##### CONSTRUCTORS AND DECONSTRUCTORS ########
// ##############################################
// ##############################################

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
    : Expression(info, TypeSystem::Current().Undefined) {
  this->values = std::move(args);
}
TypeArguments::TypeArguments(SourceInfo &info, vector<Type> &&types)
    : Expression(info, TypeSystem::Current().Undefined) {
  this->types = std::move(types);
}
Parameters::Parameters(SourceInfo &info, std::vector<Param> &&params)
    : Statement(info) {
  this->params = std::move(params);
}
TypeParameters::TypeParameters(SourceInfo &info, std::vector<TypeParam> &&params)
    : Statement(info) {
  this->params = std::move(params);
}
TypeParameters::TypeParameters(SourceInfo &info) : Statement(info) {}
Identifier::Identifier(SourceInfo &info, const Type &type, const string &name)
    : Expression(info, type), name(name) {}
Operand::Operand(SourceInfo &info, const Type &type, ExpressionPtr &&expr)
    : Expression(info, type), expression(std::move(expr)) {}
Program::Program(vector<StatementPtr> &&statements)
    : Executable(programSourceInfo) {
  this->statements = std::move(statements);
}
Return::Return(SourceInfo &info, ExpressionPtr &&value) : Statement(info) {
  this->value = std::move(value);
}
Block::Block(SourceInfo &info, vector<StatementPtr> &&statements, Scope scope)
    : Statement(info), scope(scope), statements(std::move(statements)) {}
ObjectInitializer::ObjectInitializer(SourceInfo &info, const Type &type,
                                     BlockPtr &&block)
    : Expression(info, type) {
  this->block = std::move(block);
}
Call::Call(SourceInfo &info, ExpressionPtr &&operand, ArgumentsPtr &&args, TypeArgsPtr &&type_args)
    : Expression(info, operand->type), Statement(info) {
  auto value = operand->Evaluate();

  if (!value) {
    // a bit of specific code for functor objects.
    if (auto object = std::dynamic_pointer_cast<Object_T>(value);
        object->HasMember("call")) {
      value = object->GetMember("call");
    } else {
      throw std::runtime_error("couldnt find function... call at\n" +
                               operand->srcInfo.ToString());
    }
  }

  auto callable_type = std::dynamic_pointer_cast<CallableType>(value->type);

  if (callable_type && callable_type->returnType) {
    this->type = callable_type->returnType;
  }
  this->type_args = std::move(type_args);
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
Assignment::Assignment(SourceInfo &info, const Type &type, IdentifierPtr &&iden,
                       ExpressionPtr &&expr)
    : Statement(info), iden(std::move(iden)), expr(std::move(expr)),
      type(type) {}

DotExpr::DotExpr(SourceInfo &info, const Type &type, ExpressionPtr &&left,
                 ExpressionPtr &&right)
    : Expression(info, type) {
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
Subscript::Subscript(SourceInfo &info, const Type &type, ExpressionPtr &&left,
                     ExpressionPtr &&idx)
    : Expression(info, type) {
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
UnaryExpr::UnaryExpr(SourceInfo &info, const Type &type, ExpressionPtr &&left,
                     TType op)
    : Expression(info, type) {
  this->operand = std::move(left);
  this->op = op;
}
BinExpr::BinExpr(SourceInfo &info, ExpressionPtr &&left, ExpressionPtr &&right,
                 TType op)
    : Expression(info, nullptr) {

  this->type = left->type;
  this->left = std::move(left);
  this->right = std::move(right);
  this->op = op;
}

Using::Using(SourceInfo &info, unique_ptr<ScopeResolution> &&resolution)
    : Statement(info) {

  // if (auto ns = context.FindNamespace(resolution->identifiers)) {
  //   context.ImportNamespace(resolution->identifiers);
  //   return;
  // }

  auto path = moduleRoot + resolution->full_path;
  Load(resolution->full_path);
}

UnaryStatement::UnaryStatement(SourceInfo &info, ExpressionPtr &&expr)
    : Statement(info), expr(std::move(expr)) {}

CompoundAssignment::CompoundAssignment(SourceInfo &info,
                                       ExpressionPtr &&cmpAssignExpr)
    : Statement(info), expr(std::move(cmpAssignExpr)) {}

RangeBasedFor::RangeBasedFor(SourceInfo &info, ExpressionPtr &&lhs,
                             ExpressionPtr &&rhs, BlockPtr &&block)
    : Statement(info), lhs(std::move(lhs)), rhs(std::move(rhs)),
      block(std::move(block)) {}

CompAssignExpr::CompAssignExpr(SourceInfo &info, ExpressionPtr &&left,
                               ExpressionPtr &&right, TType op)
    : Expression(info, nullptr), left(std::move(left)), right(std::move(right)),
      op(op) {
  this->type = this->left->type;
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
  }
  return "";
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
Else::Else(SourceInfo &info, IfPtr &&ifPtr, BlockPtr &&block)
    : Statement(info) {
  this->ifStmnt = std::move(ifPtr);
  this->block = std::move(block);
}
If::~If() {}
Else::~Else() {}
Delete::Delete(SourceInfo &info, ExpressionPtr &&dot)
    : Statement(info), dot(std::move(dot)) {}

Delete::Delete(SourceInfo &info, IdentifierPtr &&iden)
    : Statement(info), iden(std::move(iden)) {}

Delete::~Delete() {}
Property::Property(SourceInfo &info, const string &name, ExpressionPtr &&lambda,
                   const Mutability &mut)
    : Statement(info), name(std::move(name)), lambda(std::move(lambda)),
      mutability(mut) {}

// TODO: fix this.
Identifier::Identifier(SourceInfo &info, const string &name)
    : Expression(info, nullptr), name(name) {
  if (auto var = ASTNode::context.Find(name)) {
    type = var->type;
  }
}

TupleInitializer::TupleInitializer(SourceInfo &info,
                                   vector<ExpressionPtr> &&values)
    : Expression(info, nullptr), values(std::move(values)) {
  for (const auto &v : this->values) {
    this->types.push_back(v->type);
  }

  // If we failed to get a type for any of the expressions,
  // we try again and evaluate the expressions. This is somewhat common for
  // implicitly typed tuples of results of native functions or function
  // pointers.
  if (std::find(types.begin(), types.end(), nullptr) != types.end()) {
    types.clear();
    for (const auto &v : this->values) {
      this->types.push_back(v->Evaluate()->type);
    }
  }

  this->type = TypeSystem::Current().FromTuple(this->types);
}
ArrayInitializer::ArrayInitializer(SourceInfo &info, const Type &type,
                                   vector<ExpressionPtr> &&init)
    : Expression(info, type), init(std::move(init)) {}
Value AnonymousFunction::Evaluate() { return callable; }

AnonymousFunction::AnonymousFunction(SourceInfo &info, Type &type,
                                     Value callable)
    : Expression(info, type), callable(callable) {}
// IDEA:
// We should be inserting things into the context as we parse, so that
// identifiers can be typed at 'parse time' However, declarations are almost
// always the root of all execution. So this basically just offloads a ton of
// 'interpret time' behaviour into happening during the construction of the ast.
// We should seek a better design where we always have type information
// available without having to actually evaluate anything here, just put dummy
// data into the scope.
Declaration::Declaration(SourceInfo &info, const string &name,
                         ExpressionPtr &&expr, const Mutability &mut,
                         const Type &type)
    : Statement(info), name(name), expr(std::move(expr)), mut(mut), type(type) {

  context.CurrentScope()->ForwardDeclare(name, type, mut);
}

// ##############################################
// #######  EXECUTION AND EVALUATION ############
// ##############################################
// ##############################################

ExecutionResult Program::Execute() {

  // create an object called global, so we can bypass any shadowed variables.
  auto global =
      Object_T::New(ASTNode::context.current_namespace->root_scope);

  // include all of the native functions in this global object.
  for (const auto &[name, _] : FunctionRegistry::GetRegistry()) {
    global->SetMember(name, FunctionRegistry::GetCallable(name));
  }

  ASTNode::context.Insert("global", global, Mutability::Mut);

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
    } catch (TypeError err) {
      std::cout << "\033[1;31m" << err.what() << "\n"
                << statement->srcInfo.ToString() << std::endl;
      std::cout << "\033[0m";
    } catch (std::runtime_error err) {
      std::cout << "\033[1;31m" << err.what() << "\n"
                << statement->srcInfo.ToString() << std::endl;
      std::cout << "\033[0m";
    }
  }
  return ExecutionResult::None;
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

Value Operand::Evaluate() { return expression->Evaluate(); }
Value Identifier::Evaluate() {
  auto var = context.Find(name);
  if (var == nullptr) {
    var = Ctx::Undefined();
  }
  return var;
}
Value Arguments::Evaluate() {
  vector<Value> values;
  for (const auto &v : this->values) {
    values.push_back(v->Evaluate());
  }
  return make_shared<Tuple_T>(values);
}
Value TypeArguments::Evaluate() {
  return Ctx::Undefined();
}
ExecutionResult Parameters::Execute() { return ExecutionResult::None; }
ExecutionResult TypeParameters::Execute() { return ExecutionResult::None; }
ExecutionResult Continue::Execute() { return ExecutionResult::Continue; }
ExecutionResult Break::Execute() { return ExecutionResult::Break; }
ExecutionResult Return::Execute() {
  // return a value.
  if (value)
    return ExecutionResult(ControlChange::Return, value->Evaluate());
  // return undefined implicitly.
  else
    return ExecutionResult(ControlChange::Return, Value_T::UNDEFINED);
}
Value ReturnCopyIfNeeded(Value result) {
  switch (result->GetPrimitiveType()) {
  case Values::PrimitiveType::Invalid:
  case Values::PrimitiveType::Null:
  case Values::PrimitiveType::Undefined:
  case Values::PrimitiveType::Object:
  case Values::PrimitiveType::Array:
  case Values::PrimitiveType::Callable:
    return result;
  case Values::PrimitiveType::Tuple:
  case Values::PrimitiveType::Float:
  case Values::PrimitiveType::Int:
  case Values::PrimitiveType::Bool:
  case Values::PrimitiveType::String:
    return result->Clone();
  case Values::PrimitiveType::Lambda: {
    auto lambda = static_cast<Lambda_T *>(result.get());
    return lambda->Evaluate();
  }
  }
  // how? all cases are handled.
  return result;
}
void ApplyCopySemantics(Value &result) {
  switch (result->GetPrimitiveType()) {
  case Values::PrimitiveType::Invalid:
  case Values::PrimitiveType::Null:
  case Values::PrimitiveType::Undefined:
  case Values::PrimitiveType::Object:
  case Values::PrimitiveType::Array:
  case Values::PrimitiveType::Callable:
    break;
  case Values::PrimitiveType::Tuple:
  case Values::PrimitiveType::Float:
  case Values::PrimitiveType::Int:
  case Values::PrimitiveType::Bool:
  case Values::PrimitiveType::String:
    result = result->Clone();
    break;
  case Values::PrimitiveType::Lambda: {
    auto lambda = static_cast<Lambda_T *>(result.get());
    result = lambda->Evaluate();
    break;
  }
  }
}
void ApplyCopySemantics(ExecutionResult &result) {
  switch (result.value->GetPrimitiveType()) {
  case Values::PrimitiveType::Invalid:
  case Values::PrimitiveType::Null:
  case Values::PrimitiveType::Undefined:
  case Values::PrimitiveType::Object:
  case Values::PrimitiveType::Array:
  case Values::PrimitiveType::Callable:
    break;
  case Values::PrimitiveType::Tuple:
  case Values::PrimitiveType::Float:
  case Values::PrimitiveType::Int:
  case Values::PrimitiveType::Bool:
  case Values::PrimitiveType::String:
    result.value = result.value->Clone();
    break;
  case Values::PrimitiveType::Lambda: {
    auto lambda = static_cast<Lambda_T *>(result.value.get());
    result.value = lambda->Evaluate();
    break;
  }
  }
}
ExecutionResult Block::Execute(Scope scope) {
  
  auto last_scope = ASTNode::context.CurrentScope();
  ASTNode::context.SetCurrentScope(scope);
  
  for (auto &statement : statements) {
    Debug::m_hangUpOnBreakpoint(this, statement.get());
    try {
      auto result = statement->Execute();
      switch (result.controlChange) {
      case ControlChange::Continue:
      case ControlChange::Break:
      case ControlChange::Return:
        if (result.value != nullptr) {
          ApplyCopySemantics(result);
          context.SetCurrentScope(last_scope);
          return result;
        }
        
        return result;
      case ControlChange::None:
        continue;
      }
    } catch (std::runtime_error err) {
      std::cout << statement->srcInfo.ToString() << err.what() << std::endl;
    }
  }
  context.SetCurrentScope(last_scope);
  return ExecutionResult::None;
}
ExecutionResult Block::Execute() {
  scope->ClearVariables();
  for (auto &statement : statements) {
    Debug::m_hangUpOnBreakpoint(this, statement.get());
    auto result = statement->Execute();
    switch (result.controlChange) {
    case ControlChange::Continue:
    case ControlChange::Break:
    case ControlChange::Return:
      if (result.value != nullptr) {
        ApplyCopySemantics(result);
        return result;
      }

      return result;
    case ControlChange::None:
      continue;
    }
  }
  
  
  return ExecutionResult::None;
}
Value ObjectInitializer::Evaluate() {
  
  // pass the block it's own scope to prevent it from being cleared.
  // post execution.
  const auto exec_result = block->Execute(block->scope);
  const auto controlChange = exec_result.controlChange;
  
  if (controlChange != ControlChange::None) {
    throw std::runtime_error(
        CC_ToString(controlChange) +
        " not allowed in object initialization. did you mean to use a lambda? "
        ".. => { some body of code returning a value ..}");
  }
  
  auto object = Object_T::New(block->scope->Clone());
  block->scope->Clear();
  
  object->type = type;
  return object;
}
vector<Value> Call::GetArgsValueList(ArgumentsPtr &args) {
  vector<Value> values = {};
  for (auto &expr : args->values) {
    values.push_back(expr->Evaluate());
  }
  return values;
}
Value ArrayInitializer::Evaluate() {
  auto array = Array_T::New(init);
  ;
  array->type = type;
  return array;
}
Value Call::Evaluate() {
  auto lvalue = operand->Evaluate();
  if (lvalue->GetPrimitiveType() == PrimitiveType::Callable) {
    auto callable = std::static_pointer_cast<Callable_T>(lvalue);
    auto result = callable->Call(args, type_args);
    return result;
  } else {
    // TODO: put this somewhere where it makes sense. maybe it's own node for
    // operator overloads where an operator is directly applied to a type like
    // an object.

    // Here we overload the () operator. this is done in a special case because
    // we don't treat invocation of callables like a binary expression, it's its
    // own binary expr node.
    auto obj = std::dynamic_pointer_cast<Object_T>(lvalue);
    if (obj && obj->HasMember("call")) {
      auto fn = obj->GetMember("call");
      auto callable = std::dynamic_pointer_cast<Callable_T>(fn);
      if (callable) {
        auto args_values = GetArgsValueList(args);
        args_values.insert(args_values.begin(), obj);
        return callable->Call(args_values);
      }
    }
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

  if (block->statements.empty()) {
    return ExecutionResult::None;
  }

  
  if (decl) {
    auto _ = decl->Execute();
  }

  if (condition && condition->Evaluate()->Equals(Ctx::CreateBool(false))) {
    return ExecutionResult::None;
  }

  // for i < 250
  // for let i = 0, i < 250, ++i
  if (condition) {
    while (true) {
      auto conditionResult = condition->Evaluate();

      if (conditionResult->Equals(Bool_T::False)) {
        
        return ExecutionResult::None;
      }

      auto result = block->Execute();
      switch (result.controlChange) {
      case ControlChange::None:
      case ControlChange::Continue:
        break;
      case ControlChange::Return:
      case ControlChange::Break:
        
        return ExecutionResult::None;
      }

      if (increment) {
        increment->Execute();
      }
    }
  }

  // for {..}
  while (true) {
    auto result = block->Execute();
    switch (result.controlChange) {
    case ControlChange::None:
    case ControlChange::Continue:
      continue;
    case ControlChange::Return:
    case ControlChange::Break:
      
      return ExecutionResult::None;
    }
  }
  
  return ExecutionResult::None;
}
ExecutionResult Assignment::Execute() {
  auto var = ASTNode::context.Find(iden->name);
  if (!var) {
    throw std::runtime_error(
        "cannot assign a non-existent identifier.\nuse 'let ___ = "
        "...'\nor\nlet "
        "___ : type = ...' syntax. \n offending variable: " +
        iden->name);
  }

  auto result = expr->Evaluate();
  result->type = type;
  ApplyCopySemantics(result);

  // TODO: find a better way to query mutability of a variable.
  auto iter = ASTNode::context.FindIter(iden->name);
  context.Insert(iden->name, result, iter->first.mutability);
  return ExecutionResult::None;
}
Value EvaluateWithinObject(Scope &scope, Value object, ExpressionPtr &expr) {
  auto result = expr->Evaluate();
  return result;
}
Value EvaluateWithinObject(Scope &scope, Value object,
                           std::function<Value()> lambda) {
  auto result = lambda();
  return result;
}
Value DotExpr::Evaluate() {
  auto lvalue = left->Evaluate();

  if (lvalue->GetPrimitiveType() != PrimitiveType::Object) {
    throw std::runtime_error("invalid lhs on dot operation : " +
                             TypeToString(lvalue->GetPrimitiveType()));
  }

  auto object = static_cast<Object_T *>(lvalue.get());

  auto scope = object->scope;

  auto result = EvaluateWithinObject(scope, lvalue, right);

  return result;
}
void DotExpr::Assign(Value value) {
  auto lvalue = left->Evaluate();

  if (lvalue->GetPrimitiveType() != PrimitiveType::Object) {
    throw std::runtime_error("invalid lhs on dot operation : " +
                             TypeToString(lvalue->GetPrimitiveType()));
  }

  auto obj = static_cast<Object_T *>(lvalue.get());

  if (auto dotExpr = dynamic_cast<DotExpr *>(right.get())) {
    
    dotExpr->Assign(value);
    
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
  auto v_operand = operand->Evaluate();

  switch (op) {
  case TType::Sub:
    return v_operand->Negate();
  case TType::Not:
    return v_operand->Not();
  case TType::Increment:
    v_operand->Set(v_operand->Add(Ctx::CreateInt(1)));
    return v_operand;
  case TType::Decrement:
    v_operand->Set(v_operand->Subtract(Ctx::CreateInt(1)));
    return v_operand;
  default:
    throw std::runtime_error("invalid operator in unary expression " +
                             TTypeToString(op));
  }
  return Value_T::UNDEFINED;
}
Value BinExpr::Evaluate() {
  auto left = this->left->Evaluate();
  auto right = this->right->Evaluate();
  
  if (!left->type->Equals(right->type.get())) {
    throw TypeError(left->type, right->type);
  }
  
  switch (op) {
  case TType::NullCoalescing: {
    if (left->GetPrimitiveType() == PrimitiveType::Null ||
        left->GetPrimitiveType() == PrimitiveType::Undefined) {
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
    return Ctx::CreateBool(left->Greater(right)->Equals(Value_T::True) ||
                           left->Equals(right));
  case TType::LessEq:
    return Ctx::CreateBool(left->Less(right)->Equals(Value_T::True) ||
                           left->Equals(right));
  case TType::Equals:
    return Bool_T::New(left->Equals(right));
  case TType::NotEquals: {
    return Bool_T::New(!left->Equals(right));
  }
  case TType::Assign:
    left->Set(right);
    return left;
  default:
    throw std::runtime_error("invalid operator in binary expresison " +
                             TTypeToString(op));
  }
};
ExecutionResult Using::Execute() { return ExecutionResult::None; }
ExecutionResult RangeBasedFor::Execute() {
  auto collection = this->rhs->Evaluate();

  auto lhs = dynamic_cast<Identifier *>(this->lhs.get());

  if (!lhs && names.empty()) {
    throw std::runtime_error(
        "the left hand side of a range based for loop must be an identifier.\n"
        "example: for i : array/object/string {..}\n"
        "or a tuple deconstruction\n"
        "example: for k,v : someObject/tupleArray {}");
  }

  const auto SetVariable = [&](const Value &value) -> void {
    if (lhs) {
      context.CurrentScope()->Set(lhs->name, value, Mutability::Const);
    } else if (auto tuple = std::dynamic_pointer_cast<Tuple_T>(value);
               !names.empty()) {
      tuple->Deconstruct(names);
    }
  };

  Array array = nullptr;
  Object obj = nullptr;
  string string;
  bool isString = Ctx::TryGetString(collection, string);
  bool isObject = Ctx::TryGetObject(collection, obj);

  if (!isObject && !isString && !Ctx::TryGetArray(collection, array) &&
      !Ctx::TryGetObject(collection, obj)) {
    throw std::runtime_error("invalid range-based for loop: the target "
                             "container must be an array, object or string.");
  }

  if (array) {
    for (auto &v : array->values) {
      SetVariable(v);
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
  } else if (isObject) {
    auto tuple = make_shared<Tuple_T>(std::vector<Value>());
    for (auto &[key, val] : obj->scope->Members()) {
      tuple->values = {Ctx::CreateString(key.value), val};
      tuple->type = TypeSystem::Current().FromTuple(tuple->values);
      

      SetVariable(tuple);

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
      
      SetVariable(Ctx::CreateString(std::string() + c));
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
  
  
  return ExecutionResult::None;
}
ExecutionResult CompoundAssignment::Execute() {
  auto _ = expr->Evaluate();
  return ExecutionResult::None;
}
Value CompAssignExpr::Evaluate() {
  auto lvalue = left->Evaluate();

  auto iden = dynamic_cast<Identifier *>(left.get());

  auto it = context.FindIter(iden->name);

  if (it->second == nullptr) {
    throw std::runtime_error("variable did not exist: cannot compound assign");
  }

  Mutability mut = it->first.mutability;
  auto rvalue = right->Evaluate();

  switch (op) {
  case TType::AddEq: {
    auto result = lvalue->Add(rvalue);
    if (iden) {
      context.Insert(iden->name, result, mut);
    } else {
      lvalue->Set(result);
    }
    return lvalue;
  }
  case TType::DivEq: {
    auto result = lvalue->Divide(rvalue);
    if (iden) {
      context.Insert(iden->name, result, mut);
    } else {
      lvalue->Set(result);
    }
    return lvalue;
  }
  case TType::SubEq: {
    auto result = lvalue->Subtract(rvalue);
    if (iden) {
      context.Insert(iden->name, result, mut);
    } else {
      lvalue->Set(result);
    }
    return lvalue;
  }
  case TType::MulEq: {
    auto result = lvalue->Multiply(rvalue);
    if (iden) {
      context.Insert(iden->name, result, mut);
    } else {
      lvalue->Set(result);
    }
    return lvalue;
  }
  case TType::NullCoalescingEq: {
    if (lvalue->GetPrimitiveType() == PrimitiveType::Undefined ||
        lvalue->GetPrimitiveType() == PrimitiveType::Null) {
      auto result = rvalue;
      if (iden) {
        context.Insert(iden->name, rvalue, mut);
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
Value Match::Evaluate() {
  auto val = expr->Evaluate();
  size_t i = 0;

  // Check our expression's resulting value against the provided match cases.
  for (const auto &expr : patterns) {
    const auto branch_value = expr->Evaluate();

    // TODO: add | operator for several matches. Just like rust.
    if (branch_value->Equals(val)) {
      return expressions[i]->Evaluate();
    }
    i++;
  }
  // If we have not found a single match up to this point: hit our default case
  // if provided. If not, do nothing.
  if (branch_default)
    return branch_default->Evaluate();
  else
    return Value_T::UNDEFINED;
}
Value Lambda::Evaluate() {
  if (block) {
    auto result = block->Execute();
    if (result.controlChange != ControlChange::Return ||
        result.value == nullptr) {
      return Value_T::UNDEFINED;
    }
    return result.value;
  } else if (expr) {
    return expr->Evaluate();
  } else {
    throw std::runtime_error("invalid lambda");
  }
}
ExecutionResult FunctionDecl::Execute() {
  ASTNode::context.Insert(name, callable, Mutability::Const);
  return ExecutionResult::None;
}
ExecutionResult Delete::Execute() {
  // delete a plain identifier. easy!
  if (iden) {
    context.Erase(iden->name);
    return ExecutionResult::None;
  }

  // delete an an object behind a dot expression. ^.^
  if (auto *dot = dynamic_cast<DotExpr *>(this->dot.get())) {
    while (auto rdot = dynamic_cast<DotExpr *>(dot->right.get())) {
      dot = rdot;
    }

    Value host = dot->left->Evaluate();

    auto iden = dynamic_cast<Identifier *>(dot->right.get());
    if (!iden) {
      throw std::runtime_error("invalid dot expression in delete statement.");
    }
    auto &name = iden->name;

    if (auto host_obj = std::dynamic_pointer_cast<Object_T>(host)) {
      host_obj->scope->Erase(name);
    } else {
      throw std::runtime_error("invalid dot expression in delete statement.");
    }
  } else {
    throw std::runtime_error("invalid delete statement.");
  }
  return ExecutionResult::None;
}
Value TupleInitializer::Evaluate() {
  vector<Value> values;
  for (const auto &v : this->values) {
    auto value = v->Evaluate();
    values.push_back(value);
  }
  return make_shared<Tuple_T>(values);
}
ExecutionResult TupleDeconstruction::Execute() {
  auto tuple = this->tuple->Evaluate();
  auto value = std::dynamic_pointer_cast<Tuple_T>(tuple);

  if (value)
    value->Deconstruct(this->idens);

  return ExecutionResult::None;
}
ExecutionResult Property::Execute() {
  if (lambda) {
    Scope_T::Key key = Scope_T::Key(name, mutability);
    context.Insert(key, make_shared<Lambda_T>(std::move(lambda)));
  }
  return ExecutionResult::None;
}
ExecutionResult Declaration::Execute() {
  auto &scope = ASTNode::context.CurrentScope();
  
  if (scope->Contains(name) && !scope->Find(name)->first.forward_declared) {
    throw std::runtime_error(
        "cannot re-define an already existing variable.\noffending variable: " +
        name);
  }
  auto value = this->expr->Evaluate();
  
  // copy where needed
  ApplyCopySemantics(value);

  ASTNode::context.CurrentScope()->Set(name, value, mut);

  return ExecutionResult::None;
}

// Ideally this would be done when the node is interpreted.
// However, we have a problem where we do all of our symbol lookup during
// parsing.
void Using::Load(const std::string &moduleName) {

  for (const auto &mod : activeModules) {
    if (moduleName == mod) {
      return;
    }
  }

  activeModules.push_back(moduleName);

  auto path = moduleRoot + moduleName + ".dll";

  void *handle;
  auto module = LoadScritModule(moduleName, path, handle);

  auto _namespace = context.current_namespace;

  if (module->_namespace) {
    auto idens = Namespace::split(*module->_namespace);
    context.CreateNamespace(idens);
    context.SetCurrentNamespace(idens);
  }

  for (const auto &[name, t] : *module->types) {
    if (TypeSystem::Current().Exists(name)) {
      TypeSystem::Current().RegisterType(t, true);
    } else {
      ASTNode::context.CurrentScope()->InsertType(t->GetName(), t);
    }
  }

  for (const auto &symbol : *module->functions) {
    ASTNode::context.CurrentScope()->Set(
        symbol.first, FunctionRegistry::MakeCallable(symbol.second),
        Mutability::Const);
  }

  // revert to the original namespace.
  context.current_namespace = _namespace;
  if (module->_namespace) {
    context.ImportNamespace(Namespace::split(*module->_namespace));
  }

  free(module);

  ASTNode::context.RegisterModuleHandle(handle);
}

Value Literal::Evaluate() { return expression->Clone(); }
Value DefaultValue::Evaluate() { return type->Default(); }
void ASTNode::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Executable::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Statement::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Program::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Expression::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Operand::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Identifier::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Arguments::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void TypeArguments::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void TupleInitializer::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Property::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Parameters::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void TypeParameters::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Continue::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Break::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Return::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void DotExpr::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Delete::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Block::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void ObjectInitializer::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Call::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void If::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Else::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void For::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void RangeBasedFor::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Declaration::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Assignment::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void TupleDeconstruction::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void CompAssignExpr::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void CompoundAssignment::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void FunctionDecl::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Noop::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void DotAssignment::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void DotCallStmnt::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Subscript::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void SubscriptAssignStmnt::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void UnaryExpr::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void UnaryStatement::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void BinExpr::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Using::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Lambda::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Match::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void MatchStatement::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void DefaultValue::Accept(ASTVisitor *visitor) { visitor->visit(this); }
void Literal::Accept(ASTVisitor *visitor) { visitor->visit(this); }

TypeAlias::TypeAlias(SourceInfo &info, const string &alias, const Type &type)
    : Statement(info), type(type), alias(alias) {
  context.CurrentScope()->InsertType(alias, type);
}

Value MethodCall::Evaluate() { return callable->Call(this->args, this->type_args); }

MethodCall::MethodCall(SourceInfo &info, const Type &type,
    ExpressionPtr &&operand, ArgumentsPtr &&args, TypeArgsPtr &&type_args)
    : Statement(info), Expression(info, type), args(std::move(args)),
      operand(std::move(operand)), type_args(std::move(type_args)) {
  callable = FindCallable();
  auto t = std::dynamic_pointer_cast<CallableType>(callable->type);
  this->type = t->returnType;
}

shared_ptr<Callable_T> MethodCall::FindCallable() {
  auto identifier = dynamic_cast<Identifier *>(operand.get());
  if (!identifier) {
    throw std::runtime_error(
        "failed to call method: operand did not resolve to an identifier");
  }
  auto &args = this->args->values;
  if (args.empty()) {
    throw std::runtime_error("Too few arguments for a method call: the caller "
                             "object was not in the argument list");
  }

  // This causes a double evaulation if we do
  // obj.somemethod().println()
  // or something to that effect.

  Type type;
  auto &caller = args[0];

  if (!caller->type) {
    type = caller->Evaluate()->type;
  } else {
    type = caller->type;
  }

  if (auto method = type->Get(identifier->name);
      auto callable = std::dynamic_pointer_cast<Callable_T>(method)) {
    if (callable != nullptr) {
      return callable;
    }
  }

  if (auto obj = ASTNode::context.Find(identifier->name);
      auto callable = std::dynamic_pointer_cast<Callable_T>(obj)) {
    return callable;
  }

  if (auto callable = FunctionRegistry::GetCallable(identifier->name)) {
    return callable;
  }

  throw std::runtime_error("invalid method call: couldn't find method : " +
                           identifier->name);
}
void MethodCall::Accept(ASTVisitor *visitor) { visitor->visit(this); }

StructDeclaration::StructDeclaration(SourceInfo &info, const string &name,
                                     vector<StatementPtr> &&statements,
                                     vector<string> &template_args, Scope scope)
    : Statement(info), type_name(name), template_args(template_args), scope(scope) {
  
  
  vector<unique_ptr<Declaration>> declarations;

  for (auto &statement : statements) {
    if (auto decl = dynamic_cast<Declaration *>(statement.get())) {
      declarations.push_back(std::unique_ptr<Declaration>(
          static_cast<Declaration *>(statement.release())));
    }
  }
  auto type =
      make_shared<StructType>(name, std::move(declarations), template_args, scope);

  for (auto &statement : statements) {
    if (auto decl = dynamic_cast<FunctionDecl *>(statement.get())) {
      type->Scope().Set(decl->name, decl->callable, Mutability::Const);
    }
  }

  context.CurrentScope()->OverwriteType(name, type);
}
ExecutionResult StructDeclaration::Execute() { return ExecutionResult::None; }
Constructor::Constructor(SourceInfo &info, const Type &type,
                         ArgumentsPtr &&args)
    : Expression(info, type), args(std::move(args)) {}
Value Constructor::Evaluate() {

  auto structType = std::dynamic_pointer_cast<StructType>(type);
  if (!structType && args->values.empty()) {
    return type->Default();
  } else if (!structType) {
    // todo: provide more global type specific constructors. Tuples, etc.
    // we can also use this noed to do functional style casting.
  }

  return structType->Construct(args);
}
auto Parameters::Clone() -> unique_ptr<Parameters> {
  auto clone = std::make_unique<Parameters>(this->srcInfo);
  clone->params.reserve(params.size());
  for (const auto &param : params) {
    Param clonedParam = {param.name, param.default_value, param.type};
    clone->params.push_back(std::move(clonedParam));
  }
  return clone;
}
auto TypeParameters::Clone() -> unique_ptr<TypeParameters> {
  auto clone = std::make_unique<TypeParameters>(this->srcInfo);
  clone->params.reserve(params.size());
  for (const auto &param : params) {
    clone->params.push_back(param);
  }
  return clone;
}

Parameters::Parameters(SourceInfo &info) : Statement(info) {}

void Parameters::Apply(Scope scope, std::vector<ExpressionPtr> &values) {

  if (values.size() > params.size()) {
    throw std::runtime_error("too many arguments provided to function call");
  }

  for (size_t i = 0; i < params.size(); ++i) {
    auto &param = params[i];

    const auto has_default = param.default_value != nullptr;
    const auto has_value = i < values.size();

    if (has_value) {
      scope->Set(Scope_T::Key(param.name, param.mutability),
                 values[i]->Evaluate());
    } else if (has_default) {
      scope->Set(Scope_T::Key(param.name, param.mutability),
                 param.default_value);
    } else
      throw std::runtime_error("too few args provided to function");
  }
}
void TypeParameters::Apply(vector<Type> &type_args) {
  auto types_size = type_args.size();
  if (types_size != params.size()) {
    throw std::runtime_error("wrong number of type args passed to type parameters");
  } 
  for (size_t i = 0; i < types_size; i++) {
    params[i]->type = type_args[i];
  }
}
void Parameters::Apply(Scope scope, std::vector<Value> &values) {
  for (size_t i = 0; i < params.size(); ++i) {
    auto &param = params[i];

    const auto has_default = param.default_value != nullptr;
    const auto has_value = i < values.size();

    if (has_value) {
      scope->Set(Scope_T::Key(param.name, param.mutability), values[i]);
    } else if (has_default) {
      scope->Set(Scope_T::Key(param.name, param.mutability),
                 param.default_value);
    } else
      throw std::runtime_error("too few args provided to function");
  }
}

void Parameters::ForwardDeclare(Scope scope) {
  for (const auto &param : params) {
    scope->ForwardDeclare(param.name, param.type, param.mutability);
  }
}
Value TypeIdentifier::Evaluate() { return Ctx::Undefined(); }

Value ScopeResolution::Evaluate() {
  auto resolution = context.Resolve(this);
  if (resolution.value) {
    return resolution.value;
  } else {
    throw std::runtime_error("unable to resolve scope for : " +
                             this->full_path);
  }
}

ScopeResolution::ScopeResolution(SourceInfo &info, vector<string> &identifiers)
    : Expression(info, TypeSystem::Current().Undefined) {
  for (const auto &iden : identifiers) {
    full_path += iden;

    if (iden != identifiers.back()) {
      full_path += "::";
    }
  }
  this->identifiers = identifiers;
}

FunctionDecl::FunctionDecl(SourceInfo &info, string &name,
                           const shared_ptr<Callable_T> &callable)
    : Statement(info), name(name) {
  this->callable = callable;
}
Block::~Block() {}
void TypeParameters::DeclareTypes() {
  for (auto &param : params) {
    auto type = make_shared<GenericType_T>(param);
    ASTNode::context.CurrentScope()->InsertType(param->name, type);
  }
}

