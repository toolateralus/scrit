#include "ast.hpp"
#include "context.hpp"
#include "serializer.hpp"
#include "type.hpp"
#include "value.hpp"

#include <memory>

Value Object_T::GetMember(const string &name) {
  if (scope->Contains(name))
    return scope->GetValue(name);
  else
    return Value_T::Null;
}

void Object_T::SetMember(const string &name, Value value,
                         Mutability mutability) {
    scope->Declare(name, value, mutability);
}

string Object_T::ToString() const { return Writer::ToString(this, {}); }

bool Object_T::Equals(Value value) {
  static string op_key = "equals";
  
  if (!type->Scope().Contains(op_key)) {
    return value == shared_from_this();
  }
  
  auto result = CallOpOverload(value, op_key);

  return result && result->Equals(True);
}
Value Object_T::Subscript(Value key) {
  string strKey;
  if (Ctx::TryGetString(key, strKey)) {
    return GetMember(strKey);
  }

  return Null;
}

Value Object_T::SubscriptAssign(Value key, Value value) {
  string strKey;
  int64_t idx;
  // For map style behaviour, or 'associative arrays'..
  if (Ctx::TryGetString(key, strKey)) {
    if (scope->Contains(strKey)) {
      scope->Assign(strKey, value);
    } else {
      scope->Declare(strKey, value, Mutability::Mut);
    }
  } else if (Ctx::TryGetInt(key, idx)) {
    auto it = scope->Members().begin();
    std::advance(it, idx);
    if (it != scope->Members().end()) {
      it->second = value;
    }
  }
  return Null;
}

// Deep clone.
Value Object_T::Clone() {
  Scope scope = make_shared<Scope_T>(this->scope->parent.lock());
  for (const auto &[key, var] : this->scope->Members()) {
    scope->Declare(key.value, var->Clone(), key.mutability);
  }
  auto clone = Ctx::CreateObject(scope);
  clone->type = this->type;
  return clone;
}

bool Object_T::operator==(Object_T *other) {
  return scope->Members() == other->scope->Members() && this == other;
}

bool Object_T::HasMember(const string &name) { return scope->Contains(name); }

Object_T::Object_T(Scope scope)
    : Value_T(TypeSystem::Current().Find("object")) {
  this->scope = scope;
}

Value Object_T::CallOpOverload(Value &arg, const string &op_key) {
  if (!type->Scope().Contains(op_key) && !scope->Contains(op_key)) {
    throw std::runtime_error("Couldn't find operator overload: " + op_key);
  }
  
  
  Scope_T &_scope = type->Scope().Contains(op_key) ? type->Scope() : *scope;
  
  auto [it, found] = scope->Find(op_key);
  
  if (!found || it->second->GetPrimitiveType() != PrimitiveType::Callable) {
    throw std::runtime_error("Operator overload was not a callable");
  }
  auto member = it->second;
  auto callable = static_cast<Callable_T *>(member.get());
  auto args = std::vector<Value>{shared_from_this(), arg};
  auto result = callable->Call(args);
  return result;
}
Value Object_T::Add(Value other) {
  static const string op_key = "add";
  return CallOpOverload(other, op_key);
}
Value Object_T::Subtract(Value other) {
  static const string op_key = "sub";
  return CallOpOverload(other, op_key);
}
Value Object_T::Multiply(Value other) {
  static const string op_key = "mul";
  return CallOpOverload(other, op_key);
}
Value Object_T::Divide(Value other) {
  static const string op_key = "div";
  return CallOpOverload(other, op_key);
}
Bool Object_T::Less(Value other) {
  static const string op_key = "less";
  auto result = CallOpOverload(other, op_key);
  if (result->GetPrimitiveType() == PrimitiveType::Bool) {
    return std::dynamic_pointer_cast<Bool_T>(result);
  }
  return False;
}
Bool Object_T::Greater(Value other) {
  static const string op_key = "greater";
  auto result = CallOpOverload(other, op_key);
  if (result->GetPrimitiveType() == PrimitiveType::Bool) {
    return std::dynamic_pointer_cast<Bool_T>(result);
  }
  return False;
}
