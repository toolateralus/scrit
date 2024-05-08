#include "value.hpp"
#include "ast.hpp"
#include "context.hpp"

#include <iostream>
#include <memory>
#include <sstream>

Bool Value_T::True = make_shared<::Bool_T>(true);
Bool Value_T::False = make_shared<::Bool_T>(false);
Null Value_T::Null = make_shared<::Null_T>();
Value Value_T::InvalidCastException = make_shared<::Null_T>();
Undefined Value_T::Undefined = make_shared<::Undefined_T>();
Value Object_T::GetMember(const string &name) { return scope->variables[name]; }
void Object_T::SetMember(const string &name, Value &value) {
  scope->variables[name] = value;
}
Value Callable_T::Call(ArgumentsPtr &args) {
  
  auto scope = ASTNode::context.PushScope();
  auto values = Call::GetArgsValueList(args);
  for (int i = 0; i < params->names.size(); ++i) {
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

  if (idx < 0 || idx >= values.size()) {
    throw std::out_of_range("Index out of range");
  }

  Value removedValue = values[idx];
  values.erase(values.begin() + idx);

  return removedValue;
}
void Array_T::Insert(Int index, Value value) {
  int idx = index->value;
  auto &values = this->values;
  if (idx < 0 || idx > values.size()) {
    throw std::out_of_range("Index out of range");
  }
  values.insert(values.begin() + idx, value);
}
void Array_T::Assign(Int index, Value value) {
  int idx = index->value;
  auto &values = this->values;
  if (idx < 0 || idx >= values.size()) {
    throw std::out_of_range("Index out of range");
  }
  values[idx] = value;
}


string Object_T::ToString() const {
  std::stringstream ss = {};
  ss << "{";
  for (const auto [key, var] : scope->variables) {
    ss << '\"' << key <<  "\" : " << var->ToString() << "\n";
  }
  ss << "}";
  return ss.str();
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
  std::stringstream ss = {};
  ss << "[";
  for (const auto value : values) {
    ss << value->ToString();
    if (value != values.back()) {
      ss << ", ";
    }
  }
  ss << "]";
  return ss.str();
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
    auto i = make_shared<Int_T>(this->value +
                              static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::Null;
}
Value Int_T::Subtract(Value other) {
  if (other->GetType() == ValueType::Int) {
    return make_shared<Int_T>(this->value -
                              static_cast<Int_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Int_T::Multiply(Value other) {
  if (other->GetType() == ValueType::Int) {
    return make_shared<Int_T>(this->value *
                              static_cast<Int_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Int_T::Divide(Value other) {
  if (other->GetType() == ValueType::Int) {
    return make_shared<Int_T>(this->value /
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
    return make_shared<Bool_T>(this->value ||
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::And(Value other) {
  if (other->GetType() == ValueType::Int) {
    return make_shared<Bool_T>(this->value &&
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::Less(Value other) {
  if (other->GetType() == ValueType::Int) {
    return make_shared<Bool_T>(this->value <
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::Greater(Value other) {
  if (other->GetType() == ValueType::Int) {
    return make_shared<Bool_T>(this->value >
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::GreaterEquals(Value other) {
  if (other->GetType() == ValueType::Int) {
    return make_shared<Bool_T>(this->value >=
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Bool Int_T::LessEquals(Value other) {
  if (other->GetType() == ValueType::Int) {
    return make_shared<Bool_T>(this->value <=
                               static_cast<Int_T *>(other.get())->value);
  }
  return False;
}
Value Int_T::Negate() { return make_shared<Int_T>(-value); }
string Int_T::ToString() const { return std::to_string(value); }

bool Float_T::Equals(Value value) {
  if (value->GetType() == ValueType::Float) {
    return static_cast<Float_T *>(value.get())->value == this->value;
  }
  return false;
}
Value Float_T::Add(Value other) {
  if (other->GetType() == ValueType::Float) {
    return make_shared<Float_T>(this->value +
                                static_cast<Float_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Float_T::Subtract(Value other) {
  if (other->GetType() == ValueType::Float) {
    return make_shared<Float_T>(this->value -
                                static_cast<Float_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Float_T::Multiply(Value other) {
  if (other->GetType() == ValueType::Float) {
    return make_shared<Float_T>(this->value *
                                static_cast<Float_T *>(other.get())->value);
  }
  return Value_T::Null;
}
Value Float_T::Divide(Value other) {
  if (other->GetType() == ValueType::Float) {
    return make_shared<Float_T>(this->value /
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
    return make_shared<Bool_T>(this->value ||
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::And(Value other) {
  if (other->GetType() == ValueType::Float) {
    return make_shared<Bool_T>(this->value &&
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::Less(Value other) {
  if (other->GetType() == ValueType::Float) {
    return make_shared<Bool_T>(this->value <
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::Greater(Value other) {
  if (other->GetType() == ValueType::Float) {
    return make_shared<Bool_T>(this->value >
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::GreaterEquals(Value other) {
  if (other->GetType() == ValueType::Float) {
    return make_shared<Bool_T>(this->value >=
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Bool Float_T::LessEquals(Value other) {
  if (other->GetType() == ValueType::Float) {
    return make_shared<Bool_T>(this->value <=
                               static_cast<Float_T *>(other.get())->value);
  }
  return False;
}
Value Float_T::Negate() { return make_shared<Float_T>(-value); }
string Float_T::ToString() const { return std::to_string(value); }


bool String_T::Equals(Value value) {
  if (value->GetType() == ValueType::String) {
    return static_cast<String_T *>(value.get())->value == this->value;
  }
  return false;
}
Value String_T::Add(Value other) {
  return make_shared<String_T>(value + other->ToString());
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
    return make_shared<Bool_T>(this->value ||
                               static_cast<Bool_T *>(other.get())->value);
  }
  return False;
}
Bool Bool_T::And(Value other) {
  if (other->GetType() == ValueType::Bool) {
    return make_shared<Bool_T>(this->value &&
                               static_cast<Bool_T *>(other.get())->value);
  }
  return False;
}
Bool Bool_T::Not() { return make_shared<Bool_T>(!value); }
void Bool_T::Set(Value newValue) {
  if (newValue->GetType() == ValueType::Bool) {
    this->value = static_cast<Bool_T *>(newValue.get())->value;
  }
}


string Bool_T::ToString() const { return std::to_string(value); }
string Undefined_T::ToString() const { return "undefined"; }
string Null_T::ToString() const { return "null"; }


Array Array_T::New(vector<Value> &values) {
  auto array = make_shared<Array_T>(values);
  return array;
}
bool Array_T::Equals(Value value) { return value.get() == this; }
bool NativeCallable_T::Equals(Value value) { return value.get() == this; }
bool Callable_T::Equals(Value value) { return value.get() == this; }
bool Object_T::Equals(Value value) { return value.get() == this; }
bool Undefined_T::Equals(Value value) {
  std::cout << value << std::endl;
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
