#include "value.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "native.hpp"
#include "serializer.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <vector>

Bool Value_T::True = Bool_T::New(true);
Bool Value_T::False = Bool_T::New(false);
Null Value_T::Null = make_shared<Null_T>();
Value Value_T::InvalidCastException = make_shared<::Null_T>();
Undefined Value_T::Undefined = make_shared<::Undefined_T>();
Value Object_T::GetMember(const string &name) { return scope->variables[name]; }
void Object_T::SetMember(const string &name, Value &value) {
  scope->variables[name] = value;
}
Value Callable_T::Call(ArgumentsPtr &args) {
  
  auto scope = ASTNode::context.PushScope();
  auto values = Call::GetArgsValueList(args);
  for (size_t i = 0; i < params->names.size(); ++i) {
    if (i < values.size()) 
      scope->variables[params->names[i]] = values[i];
  }
  auto result = block->Execute();
  
  ASTNode::context.PopScope();
  
  switch (result.controlChange) {
  case ControlChange::None:
    return Value_T::Null;
  case ControlChange::Return:
  case ControlChange::Exception:
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

string Object_T::WriteMembers(std::unordered_set<const Value_T*> foundObjs) const {
  std::stringstream ss = {};
  foundObjs.insert(this);
  ss << "{";
  int i = scope->variables.size();
  string delimter = ", ";
  for (const auto &[key, var] : scope->variables) {
    i--;
    if (i == 0) {
      delimter = "";
    }
    switch (var->GetType()) {
    case ValueType::Object: {
      auto obj = static_cast<Object_T*>(var.get());
      ss << '\"' << key <<  "\" : " << obj->ToString(foundObjs) << delimter;
      break;
    }
    default:
      ss << '\"' << key <<  "\" : " << var->ToString() << delimter;
      break;
    }
  }
  ss << "}";
  return ss.str();
};

string Object_T::ToString() const {
  return Writer::ToString(this, {});
}
string Object_T::ToString(std::unordered_set<const Value_T*> foundObjs) const {
  if (std::find(foundObjs.begin(), foundObjs.end(), this) != foundObjs.end()) {
    return "[Circular]";
  }
  return WriteMembers(foundObjs);
}
string Callable_T::ToString() const {
  std::stringstream ss = {};
  ss << "callable(";
  for (const auto &name : params->names) {
    ss << name;
    if (name != params->names.back()) {
      ss << ", ";
    }
  }
  ss << ")";
  return ss.str();
}
string Array_T::ToString() const {
  return Writer::ToString(this, {});
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
  if (function)
    result = function(values);
  ASTNode::context.PopScope();
  if (result == nullptr) {
    return Undefined;
  } else {
    return result;
  }
}
NativeCallable_T::NativeCallable_T(NativeFunctionPtr ptr) : function(ptr) {
  
}
Callable_T::~Callable_T() {}
string NativeCallable_T::ToString() const {
  stringstream ss = {};
  ss << "native_callable()";
  return ss.str();
}

bool Int_T::Equals(Value value) {
  if (value->GetType() == ValueType::Int) {
    return static_cast<Int_T *>(value.get())->value == this->value;
  }
  return false;
};
Value Int_T::Add(Value other) {
  if (other->GetType() == ValueType::Int) {
    auto i = Int_T::New(this->value +
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::Null;
}
Value Int_T::Subtract(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Int_T::New(this->value -
                              static_cast<Int_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Int_T::Multiply(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Int_T::New(this->value *
                              static_cast<Int_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Int_T::Divide(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Int_T::New(this->value /
                              static_cast<Int_T *>(other.get())->value);
  }
  return Value_T::Null;
}
void Int_T::Set(Value newValue) {
  if (newValue->GetType() == ValueType::Int) {
    this->value = static_cast<Int_T *>(newValue.get())->value;
  }
}
Bool Int_T::Or(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value ||
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::And(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value &&
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::Less(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value <
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::Greater(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value >
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::GreaterEquals(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value >=
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::LessEquals(Value other) {
  if (other->GetType() == ValueType::Int) {
    return Bool_T::New(this->value <=
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Value Int_T::Negate() { return Int_T::New(-value); }
string Int_T::ToString() const { return std::to_string(value); }

bool Float_T::Equals(Value value) {
  if (value->GetType() == ValueType::Float) {
    return static_cast<Float_T *>(value.get())->value == this->value;
  }
  return false;
}
Value Float_T::Add(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Float_T::New(this->value +
                                static_cast<Float_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Float_T::Subtract(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Float_T::New(this->value -
                                static_cast<Float_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Float_T::Multiply(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Float_T::New(this->value *
                                static_cast<Float_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Float_T::Divide(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Float_T::New(this->value /
                                static_cast<Float_T *>(other.get())->value);
  }
  return Value_T::Null;
}
void Float_T::Set(Value newValue) {
  if (newValue->GetType() == ValueType::Float) {
    this->value = static_cast<Float_T *>(newValue.get())->value;
  }
}
Bool Float_T::Or(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value ||
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::And(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value &&
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::Less(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value <
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::Greater(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value >
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::GreaterEquals(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value >=
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::LessEquals(Value other) {
  if (other->GetType() == ValueType::Float) {
    return Bool_T::New(this->value <=
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Value Float_T::Negate() { return Float_T::New(-value); }
string Float_T::ToString() const { return std::to_string(value); }


bool String_T::Equals(Value value) {
  if (value->GetType() == ValueType::String) {
    return static_cast<String_T *>(value.get())->value == this->value;
  }
  return false;
}
Value String_T::Add(Value other) {
  return String_T::New(value + other->ToString());
}
void String_T::Set(Value newValue) {
  if (newValue->GetType() == ValueType::String) {
    this->value = static_cast<String_T *>(newValue.get())->value;
  }
}
string String_T::ToString() const { return value; }

bool Bool_T::Equals(Value value) {
  if (value->GetType() == ValueType::Bool) {
    return static_cast<Bool_T *>(value.get())->value == this->value;
  }
  return false;
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
}


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


Array Array_T::New(vector<Value> &values) {
  auto array = make_shared<Array_T>(values);
  return array;
}
bool Array_T::Equals(Value value) { return value.get() == this; }
bool NativeCallable_T::Equals(Value value) { return value.get() == this; }
bool Callable_T::Equals(Value value) { return value.get() == this; }
bool Object_T::Equals(Value value) { return value.get() == this; }
bool Undefined_T::Equals(Value value) {
  return value.get() == this || value->GetType() == ValueType::Undefined;
}
bool Null_T::Equals(Value value) {
  return value == Value_T::Null || value->GetType() == ValueType::Null;
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
Object_T::Object_T(Scope scope) { this->scope = scope; }
Bool ValueFactory::CreateBool(const bool value) { return Bool_T::New(value); }
String ValueFactory::CreateString(const string value) {
  return String_T::New(value);
}
Int ValueFactory::CreateInt(const int value) { return Int_T::New(value); }
Float ValueFactory::CreateFloat(const float value) {
  return Float_T::New(value);
}
Object ValueFactory::CreateObject(Scope scope) { return Object_T::New(scope); }
Array ValueFactory::CreateArray(vector<Value> values) {
  return Array_T::New(values);
}
