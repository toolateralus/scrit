#include "value.hpp"
#include "ast.hpp"
#include <memory>
#include <sstream>

Bool Value_T::True = make_shared<::Bool_T>(true);
Bool Value_T::False = make_shared<::Bool_T>(false);
Value Value_T::Null = make_shared<::Null>();
Value Value_T::Undefined = make_shared<::Undefined>();
Value Object_T::GetMember(const string &name) { return scope->variables[name]; }
void Object_T::SetMember(const string &name, Value &value) {
  scope->variables[name] = value;
}
Value Callable_T::Call(unique_ptr<Arguments> args) {
  auto scope = ASTNode::context.PushScope();
  for (int i = 0; i < params->names.size(); ++i) {
    if (i < args->values.size()) {
      auto value = args->values[i]->Evaluate();
      scope->variables[params->names[i]] = value;
    }
  }
  auto result = block->EvaluateStatement();
  
  if (auto ret = dynamic_cast<Return*>(result.get())) {
    return ret->Evaluate();
  }
  return Value_T::Null;
}

Array_T::Array_T(vector<unique_ptr<Expression>> &&init) : Value_T(ValueType::Array){
  initializer = std::move(init);
  is_initialized = false;
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



Callable_T::Callable_T(unique_ptr<Block>&& block, unique_ptr<Parameters> &&params) : Value_T(ValueType::Callable), block(std::move(block)), params(std::move(params)) {
  

}
Array_T::Array_T() : Value_T(ValueType::Array) {
  is_initialized = true;
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
Array Array_T::New(vector<unique_ptr<Expression>> &&init) {
  return make_shared<Array_T>(std::move(init));
}
Array Array_T::New() { return make_shared<Array_T>(); }
