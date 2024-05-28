#include "value.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "serializer.hpp"

#include <memory>
#include <sstream>

Bool Value_T::True = Bool_T::New(true);
Bool Value_T::False = Bool_T::New(false);
Null Value_T::VNULL = make_shared<Null_T>();
Value Value_T::InvalidCastException = make_shared<::Null_T>();
Undefined Value_T::UNDEFINED = make_shared<::Undefined_T>();
Value Object_T::GetMember(const string &name) { 
  if (scope->variables.contains(name))
    return scope->variables[name]; 
  else return Value_T::UNDEFINED;
}
void Object_T::SetMember(const string &name, Value &value) {
  scope->variables[name] = value;
}
Value Callable_T::Call(ArgumentsPtr &args) {
  
  auto scope = ASTNode::context.PushScope();
  auto values = Call::GetArgsValueList(args);
  
  int i = 0;
  for (const auto& [key, value] : params->map) {
    if (i < values.size()) {
      scope->variables[key] = values[i];
    } else if (value != nullptr) {
      scope->variables[key] = value;
    } else {
      break;
    }
    i++;
  }
  
  auto result = block->Execute();
  
  ASTNode::context.PopScope();
  
  switch (result.controlChange) {
  case ControlChange::None:
    return Value_T::VNULL;
  case ControlChange::Return:
    return result.value;
  default:
    throw std::runtime_error("Uncaught " + CC_ToString(result.controlChange));
  }
}

Value Array_T::At(Int index) { return values.at(index->value); }
void Array_T::Push(Value value) { values.push_back(value); }
Value Array_T::Pop() {
  auto val = values.back();
  values.pop_back();
  return val;
}
Value Array_T::Remove(Int index) {
  int idx = index->value;
  auto &values = this->values;

  if (idx < 0 || (size_t)idx >= values.size()) {
    throw std::out_of_range("Index out of range");
  }

  Value removedValue = values[idx];
  values.erase(values.begin() + idx);

  return removedValue;
}
void Array_T::Insert(Int index, Value value) {
  int idx = index->value;
  auto &values = this->values;
  if (idx < 0 || (size_t)idx > values.size()) {
    throw std::out_of_range("Index out of range");
  }
  values.insert(values.begin() + idx, value);
}
void Array_T::Assign(Int index, Value value) {
  int idx = index->value;
  auto &values = this->values;
  if (idx < 0 || (size_t)idx >= values.size()) {
    throw std::out_of_range("Index out of range");
  }
  values[idx] = value;
}

Array Array_T::New(vector<ExpressionPtr> &&init) {
  return make_shared<Array_T>(std::move(init));
}
Array Array_T::New() { 
  auto values = vector<Value>();
  return make_shared<Array_T>(values); }

Value NativeCallable_T::Call(unique_ptr<Arguments> &args) {
  ASTNode::context.PushScope();
  auto values = Call::GetArgsValueList(args);
  Value result;
  
  if (function.func)
    result = function.func(values);
  
  ASTNode::context.PopScope();
  if (result == nullptr) {
    return UNDEFINED;
  } else {
    return result;
  }
}
NativeCallable_T::NativeCallable_T(const NativeFunction &function) : function(function) {
  
}


Value Int_T::Add(Value other) {
  if (other->GetType() == ValueType::Int) {
    auto i = Int_T::New(this->value +
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  if (other->GetType() == ValueType::Float) {
    auto i = Float_T::New(this->value +
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
Value Int_T::Subtract(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Int_T::New(this->value -
                              static_cast<Int_T *>(other.get())->value);
  }
   if (other->GetType() == ValueType::Float) {
    auto i = Float_T::New(this->value -
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
Value Int_T::Multiply(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Int_T::New(this->value *
                              static_cast<Int_T *>(other.get())->value);
  }
   if (other->GetType() == ValueType::Float) {
    auto i = Float_T::New(this->value *
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
Value Int_T::Divide(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Int_T::New(this->value /
                              static_cast<Int_T *>(other.get())->value);
  }
   if (other->GetType() == ValueType::Float) {
    auto i = Float_T::New(this->value /
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
void Int_T::Set(Value newValue) {
  if (newValue->GetType() == ValueType::Int) {
    this->value = static_cast<Int_T *>(newValue.get())->value;
  }
  if (newValue->GetType() == ValueType::Float) {
    this->value = static_cast<Float_T*>(newValue.get())->value;
  }
}
Bool Int_T::Or(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value ||
                               static_cast<Int_T *>(other.get())->value);
  }
   if (other->GetType() == ValueType::Float) {
    auto i = Bool_T::New(this->value ||
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Int_T::And(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value &&
                               static_cast<Int_T *>(other.get())->value);
  }
   if (other->GetType() == ValueType::Float) {
    auto i = Bool_T::New(this->value &&
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Int_T::Less(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value <
                               static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Float) {
    auto i = Bool_T::New(this->value <
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Int_T::Greater(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value >
                               static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Float) {
    auto i = Bool_T::New(this->value >
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Int_T::GreaterEquals(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value >=
                               static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Float) {
    auto i = Bool_T::New(this->value >=
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Int_T::LessEquals(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value <=
                               static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Float) {
    auto i = Bool_T::New(this->value <=
                              static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}
Value Int_T::Negate() { return Int_T::New(-value); }


Value Float_T::Add(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Float_T::New(this->value +
                                static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Float_T::New(this->value +
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
Value Float_T::Subtract(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Float_T::New(this->value -
                                static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Float_T::New(this->value -
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
Value Float_T::Multiply(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Float_T::New(this->value *
                                static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Float_T::New(this->value *
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
Value Float_T::Divide(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Float_T::New(this->value /
                                static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Float_T::New(this->value /
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
void Float_T::Set(Value newValue) {
  if (newValue->GetType() == ValueType::Float) {
    this->value = static_cast<Float_T *>(newValue.get())->value;
  }
  if (newValue->GetType() == ValueType::Int) {
    this->value = static_cast<Int_T*>(newValue.get())->value;
  }
}
Bool Float_T::Or(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value ||
                               static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Bool_T::New(this->value ||
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Float_T::And(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value &&
                               static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Bool_T::New(this->value &&
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Float_T::Less(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value <
                               static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Bool_T::New(this->value <
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Float_T::Greater(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value >
                               static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Bool_T::New(this->value >
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Float_T::GreaterEquals(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value >=
                               static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Bool_T::New(this->value >=
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Float_T::LessEquals(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value <=
                               static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetType() == ValueType::Int) {
    auto i = Bool_T::New(this->value <=
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}
Value Float_T::Negate() { return Float_T::New(-value); }


Value String_T::Add(Value other) {
  return String_T::New(value + other->ToString());
}
void String_T::Set(Value newValue) {
  if (newValue->GetType() == ValueType::String) {
    this->value = static_cast<String_T *>(newValue.get())->value;
  }
}

Bool Bool_T::Or(Value other) {
  if (other->GetType() == ValueType::Bool) {
    return Bool_T::New(this->value ||
                               static_cast<Bool_T *>(other.get())->value);
  }
  return False;
}
Bool Bool_T::And(Value other) {
  if (other->GetType() == ValueType::Bool) {
    return Bool_T::New(this->value &&
                               static_cast<Bool_T *>(other.get())->value);
  }
  return False;
}
Bool Bool_T::Not() { return Bool_T::New(!value); }
void Bool_T::Set(Value newValue) {
  if (newValue->GetType() == ValueType::Bool) {
    this->value = static_cast<Bool_T *>(newValue.get())->value;
  }
  if (newValue->GetType() == ValueType::Int) {
    this->value = (bool)static_cast<Int_T *>(newValue.get());
  }
}

string Object_T::ToString() const {
  return Writer::ToString(this, {});
}
string Callable_T::ToString() const {
  std::stringstream ss = {};
  ss << "callable(";
  int i = 0;
  for (const auto &[name,value] : params->map) {
    ss << name;
    if (i != params->map.size()) {
      ss << ", ";
    }
    i++;
  }
  ss << ")";
  return ss.str();
}
string Array_T::ToString() const {
  return Writer::ToString(this, {});
}
string NativeCallable_T::ToString() const {
  stringstream ss = {};
  ss << "native_callable()";
  return ss.str();
}

string Float_T::ToString() const { return std::to_string(value); }
string Int_T::ToString() const { return std::to_string(value); }

string String_T::ToString() const { return value; }

string Bool_T::ToString() const { 
  static string TRUE = "true";
  static string FALSE = "false";
  return value ? TRUE : FALSE; 
}
string Undefined_T::ToString() const {
  static string undefined = "undefined";
  return undefined;
}
string Null_T::ToString() const { 
  static string null = "null";
  return null; 
}

bool Int_T::Equals(Value value) {
  if (value->GetType() == ValueType::Int) {
    return static_cast<Int_T *>(value.get())->value == this->value;
  }
  return false;
};
bool String_T::Equals(Value value) {
  if (value->GetType() == ValueType::String) {
    return static_cast<String_T *>(value.get())->value == this->value;
  }
  return false;
}
Array Array_T::New(vector<Value> &values) {
  auto array = make_shared<Array_T>(values);
  return array;
}
bool Float_T::Equals(Value value) {
  if (value->GetType() == ValueType::Float) {
    return static_cast<Float_T *>(value.get())->value == this->value;
  }
  if (value->GetType() == ValueType::Float) {
    auto i = this->value == static_cast<Float_T *>(value.get())->value;
    return i;
  }
  return false;
}
bool Bool_T::Equals(Value value) {
  if (value->GetType() == ValueType::Bool) {
    return static_cast<Bool_T *>(value.get())->value == this->value;
  }
  return false;
}
bool Array_T::Equals(Value value) { return value.get() == this; }
bool NativeCallable_T::Equals(Value value) { return value.get() == this; }
bool Callable_T::Equals(Value value) { return value.get() == this; }
bool Object_T::Equals(Value value) { return value.get() == this; }
bool Undefined_T::Equals(Value value) {
  return value.get() == this || value->GetType() == ValueType::Undefined;
}
bool Null_T::Equals(Value value) {
  return value == Value_T::VNULL || value->GetType() == ValueType::Null;
}

Value Value_T::Subscript(Value) { return UNDEFINED; }
Value String_T::Subscript(Value key) {
  int index;
  if (!Ctx::TryGetInt(key, index) || (size_t)index > value.length()) {
    return UNDEFINED;
  }
  return Ctx::CreateString(std::string() + this->value[index]);
}

Value Value_T::SubscriptAssign(Value, Value) {
  return UNDEFINED;
}
Value String_T::SubscriptAssign(Value key, Value value) {
  int idx;
  string string;
  if (Ctx::TryGetInt(key, idx) && Ctx::TryGetString(value, string)) {
    if (string.length() == 0) {
      this->value[idx] = string[0];
    } else {
      this->value.insert(idx, string);
    }
  }
  return UNDEFINED;
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
    scope->variables[strKey] = value;
  } else if (Ctx::TryGetInt(key, idx)) {
    auto it = scope->variables.begin();
    std::advance(it, idx);
    if (it != scope->variables.end()) {
      it->second = value;
    }
  }
  return UNDEFINED;
}
Value Array_T::Subscript(Value key) {
  int index;
  if (!Ctx::TryGetInt(key, index) || (size_t)index > values.size()) {
    return UNDEFINED;
  }
  return values[index];
}
Value Array_T::SubscriptAssign(Value key, Value value) {
  int idx;
  if (Ctx::TryGetInt(key, idx)) {
    values[idx] = value;
  }
  return UNDEFINED;
}

namespace Values {
  string TypeToString(ValueType type) {
    switch (type) {
    case ValueType::Invalid:
      return "invalid";
    case ValueType::Null:
      return "null";
    case ValueType::Undefined:
      return "undefined";
    case ValueType::Float:
      return "float";
    case ValueType::Int:
      return "int";
    case ValueType::Bool:
      return "bool";
    case ValueType::String:
      return "string";
    case ValueType::Object:
      return "object";
    case ValueType::Array:
      return "array";
    case ValueType::Callable:
      return "callable";
    case ValueType::Any:
      return "any";
    }
    return "";
  }

  } // namespace Values

Value Callable_T::Call(std::vector<Value> &values) {
  auto scope = ASTNode::context.PushScope();
  int i = 0;
  for (const auto& [key, value] : params->map) {
    if (i < values.size()) {
      scope->variables[key] = values[i];
    } else if (value != nullptr) {
      scope->variables[key] = value;
    } else {
      break;
    }
    i++;
  }
  auto result = block->Execute();
  ASTNode::context.PopScope();
  switch (result.controlChange) {
  case ControlChange::None:
    return Value_T::VNULL;
  case ControlChange::Return:
    return result.value;
  default:
    throw std::runtime_error("Uncaught " + CC_ToString(result.controlChange));
  }
}
Float_T::Float_T(float value) {
  this->value = value;
}
Array_T::Array_T(vector<ExpressionPtr> &&init) {
  initializer = std::move(init);
  for (auto &arg: initializer) {
    auto value = arg->Evaluate();
    Push(value);
  }
}
Callable_T::Callable_T(BlockPtr&& block, ParametersPtr &&params) :  block(std::move(block)), params(std::move(params)) {
}
String_T::String_T(const string &value) {
  this->value = value;
}
Callable_T::Callable_T() {}
Int_T::Int_T(int value){ this->value = value; }
Null_T::Null_T() {}
Undefined_T::Undefined_T()  {}
Bool_T::Bool_T(bool value) { this->value = value; }
Array_T::Array_T(vector<Value> init)  {
  this->values = init;
}
Object_T::Object_T(Scope scope) { 
  this->scope = scope; 
}
Callable_T::~Callable_T() {}


Bool Ctx::CreateBool(const bool value) { return Bool_T::New(value); }
String Ctx::CreateString(const string value) {
  return String_T::New(value);
}
Int Ctx::CreateInt(const int value) { return Int_T::New(value); }
Float Ctx::CreateFloat(const float value) {
  return Float_T::New(value);
}
Object Ctx::CreateObject(Scope scope) { return Object_T::New(scope); }
Array Ctx::CreateArray(vector<Value> values) {
  return Array_T::New(values);
}

bool Ctx::TryGetArray(Value value, Array &result) {
  if (value->GetType() == ValueType::Array) {
    result = std::dynamic_pointer_cast<Array_T>(value);
    return true;
  }
  return false;
}
bool Ctx::TryGetObject(Value value, Object &result) {
  if (value->GetType() == ValueType::Object) {
    result = std::dynamic_pointer_cast<Object_T>(value);
    return true;
  }
  return false;
}
bool Ctx::TryGetBool(Value value, bool &result) {
  if (value->GetType() == ValueType::Bool) {
    result = static_cast<Bool_T*>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::TryGetFloat(Value value, float &result) {
  if (value->GetType() == ValueType::Float) {
    result = static_cast<Float_T*>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::TryGetInt(Value value, int &result) {
  if (value->GetType() == ValueType::Int) {
    result = static_cast<Int_T*>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::TryGetString(Value value, string &result) {
  if (value->GetType() == ValueType::String) {
    result = static_cast<String_T*>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::IsUndefined(Value value) { return value->Equals(Value_T::UNDEFINED); }
bool Ctx::IsNull(Value value) { return value->Equals(Value_T::VNULL); }

Value Object_T::CallOpOverload(Value &arg, const string &op_key) {
  if (!scope->variables.contains(op_key)) {
    return Value_T::UNDEFINED;
  }
  auto member = scope->variables[op_key];
  if (member == nullptr || member->GetType() != ValueType::Callable) {
    return Value_T::UNDEFINED;
  }
  auto callable = static_cast<Callable_T *>(member.get());
  auto args = std::vector<Value>{shared_from_this(), arg};
  return callable->Call(args);
}
Value Object_T::Add(Value other) {
  static const string op_key = "add_operator";
  return CallOpOverload(other, op_key);
}
Value Object_T::Subtract(Value other) {
  static const string op_key = "sub_operator";
  return CallOpOverload(other, op_key);
}
Value Object_T::Multiply(Value other) {
    static const string op_key = "mul_operator";
  return CallOpOverload(other, op_key);
}
Value Object_T::Divide(Value other) {
    static const string op_key = "div_operator";
  return CallOpOverload(other, op_key);
}
Bool Object_T::Less(Value other) {
  static const string op_key = "less_operator";
  auto result = CallOpOverload(other, op_key);
  if (result->GetType() == ValueType::Bool) {
    return std::dynamic_pointer_cast<Bool_T>(result);
  }
  return False;
}
Bool Object_T::Greater(Value other) {
  static const string op_key = "greater_operator";
  auto result = CallOpOverload(other, op_key);
  if (result->GetType() == ValueType::Bool) {
    return std::dynamic_pointer_cast<Bool_T>(result);
  }
  return False;
}
Bool Object_T::GreaterEquals(Value other) {
  static const string op_key = "greater_eq_operator";
  auto result = CallOpOverload(other, op_key);
  if (result->GetType() == ValueType::Bool) {
    return std::dynamic_pointer_cast<Bool_T>(result);
  }
  return False;
}
Bool Object_T::LessEquals(Value other) {
  static const string op_key = "less_eq_operator";
  auto result = CallOpOverload(other, op_key);
  if (result->GetType() == ValueType::Bool) {
    return std::dynamic_pointer_cast<Bool_T>(result);
  }
  return False;
}
Values::Array Ctx::FromFloatVector(vector<float> &values) {
  Array array = CreateArray();
  for (const auto &value : values) {
    array->Push(Ctx::CreateFloat(value));
  }
  return array;
}
Values::Array Ctx::FromStringVector(vector<string> &values) {
  Array array = CreateArray();
  for (const auto &value : values) {
    array->Push(Ctx::CreateString(value));
  }
  return array;
}
Values::Array Ctx::FromBoolVector(vector<bool> &values) {
  Array array = CreateArray();
  for (const auto &value : values) {
    array->Push(Ctx::CreateBool(value));
  }
  return array;
}
Values::Array Ctx::FromIntVector(vector<int> &values) {
  Array array = CreateArray();
  for (const auto &value : values) {
    array->Push(Ctx::CreateInt(value));
  }
  return array;
}
