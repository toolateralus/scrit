#include "value.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "serializer.hpp"
#include <memory>
#include <stdexcept>

Value Object_T::GetMember(const string &name) {
  if (scope->Contains(name))
    return scope->Get(name);
  else
    return Value_T::UNDEFINED;
}

void Object_T::SetMember(const string &name, Value value) {
  scope->Set(name, value);
}

string Object_T::ToString() const { return Writer::ToString(this, {}); }

bool Object_T::Equals(Value value) { 
  static string op_key = "equals";
  if (!HasMember(op_key)) {
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

  return UNDEFINED;
}

Value Object_T::SubscriptAssign(Value key, Value value) {
  string strKey;
  int idx;
  if (Ctx::TryGetString(key, strKey)) {
    scope->Set(strKey, value);
  } else if (Ctx::TryGetInt(key, idx)) {
    auto it = scope->Members().begin();
    std::advance(it, idx);
    if (it != scope->Members().end()) {
      it->second = value;
    }
  }
  return UNDEFINED;
}

// Deep clone.
Value Object_T::Clone() {
  Scope scope = make_shared<Scope_T>();
  for (const auto &[key, var] : this->scope->Members()) {
    scope->Set(key, var->Clone());
  }
  return Ctx::CreateObject(scope);
}
bool Object_T::operator==(Object_T *other) {
  return scope->Members() == other->scope->Members() && this == other;
}
bool Object_T::HasMember(const string &name) 
{ 
  return scope->Contains(name); 
}
Object_T::Object_T(Scope scope) { this->scope = scope; }

Value Object_T::CallOpOverload(Value &arg, const string &op_key) {
  if (!scope->Contains(op_key)) {
    throw std::runtime_error("Couldn't find operator overload: " + op_key);
  }
  
  auto member = GetMember(op_key);
  if (member == nullptr || member->GetType() != ValueType::Callable) {
    throw std::runtime_error("Operator overload was not a callable");
  }
  
  auto callable = static_cast<Callable_T *>(member.get());
  auto args = std::vector<Value>{shared_from_this(), arg};
  
  return callable->Call(args);
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
  if (result->GetType() == ValueType::Bool) {
    return std::dynamic_pointer_cast<Bool_T>(result);
  }
  return False;
}
Bool Object_T::Greater(Value other) {
  static const string op_key = "greater";
  auto result = CallOpOverload(other, op_key);
  if (result->GetType() == ValueType::Bool) {
    return std::dynamic_pointer_cast<Bool_T>(result);
  }
  return False;
}
