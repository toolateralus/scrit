#include "value.hpp"
#include "ast.hpp"
#include "context.hpp"

#include <memory>
#include <sstream>

Bool Value_T::True = make_shared<::Bool_T>(true);
Bool Value_T::False = make_shared<::Bool_T>(false);
Value Value_T::Null = make_shared<::Null>();
Value Value_T::InvalidCastException = make_shared<::Null>();
Value Value_T::Undefined = make_shared<::Undefined>();
Value Object_T::GetMember(const string &name) { return scope->variables[name]; }
void Object_T::SetMember(const string &name, Value &value) {
  scope->variables[name] = value;
}
Value Callable_T::Call(ArgumentsPtr &args) {
  auto scope = ASTNode::context.PushScope();
  auto values = Call::GetArgsValueList(args);
  for (int i = 0; i < params->names.size(); ++i) {
    scope->variables[params->names[i]] = values[i];
  }
  auto result = block->Execute();
  
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

Array_T::Array_T(vector<ExpressionPtr> &&init) : Value_T(ValueType::Array){
  initializer = std::move(init);
  for (auto &arg: initializer) {
    auto value = arg->Evaluate();
    Push(value);
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

Callable_T::Callable_T(BlockPtr&& block, ParametersPtr &&params) : Value_T(ValueType::Callable), block(std::move(block)), params(std::move(params)) {
}
Array_T::Array_T() : Value_T(ValueType::Array) {
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
Array Array_T::New() { return make_shared<Array_T>(); }

Value NativeCallable_T::Call(unique_ptr<Arguments> &args) {
  auto values = Call::GetArgsValueList(args);
  if (function)
    return function(values);
  return Value_T::Undefined;
}
NativeCallable_T::NativeCallable_T(NativeFunctionPtr ptr) : function(ptr) {
  
}
Callable_T::Callable_T() : Value_T(ValueType::Callable) {}
Callable_T::~Callable_T() {}
string NativeCallable_T::ToString() const {
  stringstream ss = {};
  ss << "native_callable()";
  return ss.str();
}

