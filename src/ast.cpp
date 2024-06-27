#include "ast.hpp"
#include "context.hpp"
#include "debug.hpp"
#include "native.hpp"
#include "parser.hpp"
#include "value.hpp"
#include <iostream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>

auto programSourceInfo = SourceInfo{0, 0};

// Todo: add a scope that this was usinged in so when we
// leave that scope we dlclose() the opened module.
// right now it just leaves it open till the end of the program.
std::vector<string> Using::activeModules = {};

Context ASTNode::context = {};
ExecutionResult ExecutionResult::None = ExecutionResult(ControlChange::None, Value_T::UNDEFINED);
ExecutionResult ExecutionResult::Break = ExecutionResult(ControlChange::Break, Value_T::UNDEFINED);
ExecutionResult ExecutionResult::Continue = ExecutionResult(ControlChange::Continue, Value_T::UNDEFINED);

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
ObjectInitializer::ObjectInitializer(SourceInfo &info, BlockPtr &&block)
    : Expression(info) {
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
                       ExpressionPtr &&expr, const Mutability &mutability)
    : Statement(info) , iden(std::move(iden)), expr(std::move(expr)), mutability(mutability) {
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
  this->operand = std::move(left);
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
Using::Using(SourceInfo &info, const string &name, const bool isWildcard)
    : Statement(info), symbols({}), moduleName(name), isWildcard(isWildcard){

                                                      };
Using::Using(SourceInfo &info, const string &name, vector<string> &symbols)
    : Statement(info), symbols(symbols), moduleName(name), isWildcard(false){};

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
    : Expression(info), left(std::move(left)), right(std::move(right)), op(op) {
}

ExecutionResult Program::Execute() {
  
  // create an object called global, so we can bypass any shadowed variables.
  auto global = Object_T::New(ASTNode::context.scopes.front());
  
  // include all of the native functions in this global object.
  for(const auto &[name, _] : NativeFunctions::GetRegistry()) {
    global->SetMember(name, NativeFunctions::GetCallable(name));
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
    } catch (std::runtime_error err) {
      std::cout << statement->srcInfo.ToString() << err.what() << std::endl;
    }
  }
  return ExecutionResult::None;
}
Value Operand::Evaluate() { return value; }
Value Identifier::Evaluate() {  
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
  // return a value.
  if (value)
    return ExecutionResult(ControlChange::Return, value->Evaluate());
  // return undefined implicitly.
  else return ExecutionResult(ControlChange::Return, Value_T::UNDEFINED);
}

ExecutionResult Block::Execute(Scope scope) {
  ASTNode::context.PushScope(scope);
  
  for (auto &statement : statements) {
    Debug::m_hangUpOnBreakpoint(this, statement.get());
    try {
      auto result = statement->Execute();
      switch (result.controlChange) {
      case ControlChange::Continue:
      case ControlChange::Break:
      case ControlChange::Return:
        ASTNode::context.PopScope();
        
        if (result.value != nullptr) {
          switch (result.value->GetType()) {
            case Values::ValueType::Invalid:
            case Values::ValueType::Null:
            case Values::ValueType::Undefined:
            case Values::ValueType::Object:
            case Values::ValueType::Array:
            case Values::ValueType::Callable:
              return result;
            case Values::ValueType::Float:
            case Values::ValueType::Int:
            case Values::ValueType::Bool:
            case Values::ValueType::String:
              result.value = result.value->Clone();
              return result;
          }
        }
        
        return result;
      case ControlChange::None:
        continue;
      }
    } catch (std::runtime_error err) {
      std::cout << statement->srcInfo.ToString() << err.what() << std::endl;
    }
  }
  ASTNode::context.PopScope();
  return ExecutionResult::None;
}

ExecutionResult Block::Execute() {
  ASTNode::context.PushScope();
  for (auto &statement : statements) {
    Debug::m_hangUpOnBreakpoint(this, statement.get());
    try {
      auto result = statement->Execute();
      switch (result.controlChange) {
      case ControlChange::Continue:
      case ControlChange::Break:
      case ControlChange::Return:
        ASTNode::context.PopScope();
        
        if (result.value != nullptr) {
          switch (result.value->GetType()) {
            case Values::ValueType::Invalid:
            case Values::ValueType::Null:
            case Values::ValueType::Undefined:
            case Values::ValueType::Object:
            case Values::ValueType::Array:
            case Values::ValueType::Callable:
              return result;
            case Values::ValueType::Float:
            case Values::ValueType::Int:
            case Values::ValueType::Bool:
            case Values::ValueType::String:
              result.value = result.value->Clone();
              return result;
          }
        }
        
        return result;
      case ControlChange::None:
        continue;
      }
    } catch (std::runtime_error err) {
      std::cout << statement->srcInfo.ToString() << err.what() << std::endl;
    }
  }
  ASTNode::context.PopScope();
  return ExecutionResult::None;
}
Value ObjectInitializer::Evaluate() {
  static auto _this = Object_T::New();
  
  _this->scope->Clear();
  _this->scope->Set("this", _this, Mutability::Mut);
  
  const auto exec_result = block->Execute(_this->scope);
  const auto controlChange = exec_result.controlChange;
  
  if (controlChange != ControlChange::None) {
    throw std::runtime_error(CC_ToString(controlChange) +
                             " not allowed in object initialization. did you mean to use a lambda? .. => { some body of code returning a value ..}");
  }
  _this->scope->Erase("this");
  return Object_T::New(_this->scope);
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
    auto callable = std::static_pointer_cast<Callable_T>(lvalue);
    auto result = callable->Call(args);
    return result;
  } else {
    // Here we overload the () operator. this is done in a special case because we don't treat invocation of callables
    // like a binary expression, it's its own binary expr node.
    auto obj = std::dynamic_pointer_cast<Object_T>(lvalue);
    if (obj && obj->HasMember("op_call")) {
      auto fn = obj->GetMember("op_call");
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
      
      if (increment) {
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
          break;
          
        // clone value types.
        case Values::ValueType::Float:
        case Values::ValueType::Int:
        case Values::ValueType::Bool:
        case Values::ValueType::String:
          result = result->Clone();
        }
  
  context.Insert(iden->name, result, mutability);
  return ExecutionResult::None;
}
Value TryCallMethods(unique_ptr<Expression> &right, Value lvalue) {
  
  // recurse for bin expr.  
  if (auto binExpr = dynamic_cast<BinExpr*>(right.get())) {
    auto result = TryCallMethods(binExpr->left, lvalue);
    if (result == nullptr) {
      return binExpr->Evaluate();
    }
    auto expr = make_unique<Operand>(binExpr->srcInfo, result);
    binExpr->left = std::move(expr);
    return binExpr->Evaluate();
  }
  
  if (auto call = dynamic_cast<Call*>(right.get())) {
    if (auto name = dynamic_cast<Identifier*>(call->operand.get())) {
        Callable_T* callable = nullptr;
        auto obj = dynamic_cast<Object_T*>(lvalue.get());
        
        if (obj && obj->scope->Contains(name->name)) {
          callable = dynamic_cast<Callable_T*>(obj->scope->Get(name->name).get());
          // call the function from the object's scope.
          return EvaluateWithinObject(obj->scope, lvalue, [callable, call]() -> Value {
            return callable->Call(call->args);
          });
        }
        if (!callable) {
          callable =  dynamic_cast<Callable_T*>(ASTNode::context.Find(name->name).get());
        } 
        if (!callable && NativeFunctions::Exists(name->name)) {
          callable = NativeFunctions::GetCallable(name->name).get();
        }
        
        if (callable) {
          auto args = call->GetArgsValueList(call->args);
          // insert self as arg 0.
          args.insert(args.begin(), lvalue);
        
          if (auto nc = dynamic_cast<NativeCallable_T*>(callable)) {
            return nc->Call(args);
          } else if (auto c = dynamic_cast<Callable_T*>(callable)) {
            return c->Call(args);
          } else {
			throw std::runtime_error("invalid method call: " + name->name);
		  }
        } else throw std::runtime_error("invalid method call: " + name->name);
    }
  }
  return nullptr;
}

Value EvaluateWithinObject(Scope &scope, Value object, ExpressionPtr &expr) {
  scope->Set("this", object, Mutability::Mut);
  ASTNode::context.PushScope(scope);
  auto result = expr->Evaluate();
  ASTNode::context.PopScope();
  scope->Erase("this");
  return result;
}
Value EvaluateWithinObject(Scope &scope, Value object, std::function<Value()> lambda) {
  scope->Set("this", object, Mutability::Mut);
  ASTNode::context.PushScope(scope);
  auto result = lambda();
  ASTNode::context.PopScope();
  scope->Erase("this");
  return result;
}

Value DotExpr::Evaluate() {
  auto lvalue = left->Evaluate();
  
  
  // Try to call an ext method, or member method.
  auto ext_method_result = TryCallMethods(right, lvalue);
  if (ext_method_result != nullptr) {
    return ext_method_result;
  }
  
  // Below is field accessors.
  if (lvalue->GetType() != ValueType::Object) {
    throw std::runtime_error("invalid lhs on dot operation : " +
                             TypeToString(lvalue->GetType()));
  }
  
  auto object = static_cast<Object_T *>(lvalue.get());
  
  auto scope = object->scope;
  
  auto result = EvaluateWithinObject(scope, lvalue, right);
  
  
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
    return Ctx::CreateBool(left->Greater(right)->Equals(Value_T::True) || left->Equals(right));
  case TType::LessEq:
    return Ctx::CreateBool(left->Less(right)->Equals(Value_T::True) || left->Equals(right));
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
ExecutionResult Using::Execute() {
  for (const auto &mod : activeModules) {
    if (moduleName == mod) {
      return ExecutionResult::None;
    }
  }
  
  activeModules.push_back(moduleName);

  auto path = moduleRoot + moduleName + ".dll";
  
  void *handle;
  auto module = LoadScritModule(moduleName, path, handle);
  
  
  // we do this even when we ignore the object becasue it registers the native
  // callables.
  auto object = ScritModDefAsObject(module);
  
  if (!isWildcard && symbols.empty()) {
    ASTNode::context.Insert(moduleName, object, Mutability::Const);
  } else {
    vector<Value> values = {};
    if (!symbols.empty()) {
      for (const auto &name : symbols) {
        auto value = module->context->Find(name);
        if (value == Value_T::UNDEFINED) {
          throw std::runtime_error("invalid using statement. could not find " +
                                   name);
        }
        
        ASTNode::context.Insert(name, value, Mutability::Const);
      }
    } else {
      for (const auto &[key, var] : object->scope->Members()) {
        ASTNode::context.Insert(key.value, var, key.mutability);
        
      }
    }
  }

  delete module;
  
  ASTNode::context.RegisterModuleHandle(handle);
  
  return ExecutionResult::None;
}
ExecutionResult RangeBasedFor::Execute() {
  auto lvalue = this->rhs->Evaluate();
  
  auto lhs = dynamic_cast<Identifier *>(this->lhs.get());
  
  if (!lhs) {
    throw std::runtime_error("the left hand side of a range based for loop must be an identifier. example: for i : array/object/string {..}");
  }
  
  auto name = lhs->name;
  Array array = nullptr;
  Object obj = nullptr;
  string string;
  bool isString = Ctx::TryGetString(lvalue, string);
  bool isObject = Ctx::TryGetObject(lvalue, obj);
  
  if (!isObject && !isString && !Ctx::TryGetArray(lvalue, array) &&
      !Ctx::TryGetObject(lvalue, obj)) {
    throw std::runtime_error("invalid range-based for loop: the target "
                             "container must be an array, object or string.");
  }
  
  if (array) {
    for (auto &v : array->values) {
      
      ASTNode::context.PushScope();
      ASTNode::context.scopes.back()->Set(name, v, Mutability::Const);
      auto result = block->Execute();
      ASTNode::context.PopScope();
      
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
    auto kvp = Ctx::CreateObject();
    for (auto &[key, val] : obj->scope->Members()) {
      
      ASTNode::context.PushScope();
      kvp->scope->Erase("key");
      kvp->scope->Erase("value");
      kvp->scope->Set("key", Ctx::CreateString(key.value));
      kvp->scope->Set("value", val);
      ASTNode::context.scopes.back()->Set(name, kvp, Mutability::Const);
      auto result = block->Execute();
      ASTNode::context.PopScope();
      
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
      ASTNode::context.PushScope();
      ASTNode::context.scopes.back()->Set(name, Ctx::CreateString(std::string() + c), Mutability::Const);
      auto result = block->Execute();
      ASTNode::context.PopScope();
      
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
    if (lvalue->GetType() == ValueType::Undefined ||
        lvalue->GetType() == ValueType::Null) {
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
Else::Else(SourceInfo &info, IfPtr &&ifPtr, BlockPtr &&block)
    : Statement(info) {
  this->ifStmnt = std::move(ifPtr);
  this->block = std::move(block);
}
If::~If() {}
Else::~Else() {}


Value Match::Evaluate() {
  auto val = expr->Evaluate();

  size_t i = 0;
  
  // Check our expression's resulting value against the provided match cases.
  for (const auto &expr: branch_lhs) {
    const auto branch_value = expr->Evaluate();
    
    // TODO: add | operator for several matches. Just like rust.
    if (branch_value->Equals(val)) {
      return branch_rhs[i]->Evaluate();
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
  auto result = block->Execute();
  if (result.controlChange != ControlChange::Return || result.value == nullptr) {
    return Value_T::UNDEFINED;
  }
  return result.value;
}

ExecutionResult FunctionDecl::Execute() {
  ASTNode::context.Insert(
      name, make_shared<Callable_T>(std::move(block), std::move(parameters)),
      Mutability::Const);
  return ExecutionResult::None;
}

ExecutionResult Delete::Execute() {
  // delete a plain identifier. easy!
  if (iden) {
    context.Erase(iden->name);
    return ExecutionResult::None;
  } 
  
  // delete an an object behind a dot expression. ^.^  
  if (auto *dot = dynamic_cast<DotExpr*>(this->dot.get())) {
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
Delete::Delete(SourceInfo &info, ExpressionPtr &&dot)
    : Statement(info), dot(std::move(dot)) {}
Delete::Delete(SourceInfo &info, IdentifierPtr &&iden)
    : Statement(info), iden(std::move(iden)) {}
Delete::~Delete() {}
