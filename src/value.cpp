#include "value.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "ctx.hpp"
#include "error.hpp"
#include "serializer.hpp"
#include "type.hpp"

#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>

const Bool Value_T::True = Bool_T::New(true);
const Bool Value_T::False = Bool_T::New(false);
const Null Value_T::Null = make_shared<Null_T>();

Value Array_T::At(Int index) {
  if (values.size() <= (size_t)index->value) {
    throw std::runtime_error("Array access out of bounds");
  }
  return values.at(index->value);
}
void Array_T::Push(Value value) { values.push_back(value); }
Value Array_T::Pop() {
  if (values.size() == 0) {
    throw std::runtime_error("Attempted to pop an already empty array.");
  }
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
Array Array_T::New(vector<ExpressionPtr> &init) {
  vector<Value> values;
  for (const auto &expr : init) {
    values.push_back(expr->Evaluate());
  }
  return make_shared<Array_T>(values);
}
Array Array_T::New() {
  auto values = vector<Value>{};
  return make_shared<Array_T>(values);
}

void NativeCallable_T::CheckParameterTypes(vector<Value> &values) {
  for (auto i = 0; i < values.size(); ++i) {
    if (function->parameterTypes.size() <= i) {
      throw std::runtime_error(
          "too few parameters provided to function. expected: " +
          std::to_string(function->parameterTypes.size()) +
          " got: " + std::to_string(values.size()));
    }
    auto value = values[i];
    auto paramType = function->parameterTypes[i];
    if (!paramType->Equals(value->type.get())) {
      throw TypeError(value->type, paramType, "invalid parameter types");
    }
  }
}

void Callable_T::CheckReturnType(Value &result) {
  if (!result->type->Equals(std::dynamic_pointer_cast<CallableType>(type)->returnType.get())) {
    throw TypeError(result->type, type,
                    "Invalid return type from function");
  }
}

void NativeCallable_T::CheckReturnType(Value &result) {
  if (!result->type->Equals(function->returnType.get())) {
    throw TypeError(result->type, function->returnType,
                    "Invalid return type from function " + function->name);
  }
}

NativeCallable_T::NativeCallable_T(const shared_ptr<NativeFunction> &function)
    : Callable_T(function->returnType, function->parameterTypes), function(function) {
}

Value Float_T::Add(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    return Float_T::New(this->value +
                        static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    auto i =
        Float_T::New(this->value + static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::Null;
}
Value Float_T::Subtract(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    return Float_T::New(this->value -
                        static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    auto i =
        Float_T::New(this->value - static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::Null;
}
Value Float_T::Multiply(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    return Float_T::New(this->value *
                        static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    auto i =
        Float_T::New(this->value * static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::Null;
}
Value Float_T::Divide(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    return Float_T::New(this->value /
                        static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    auto i =
        Float_T::New(this->value / static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return Value_T::Null;
}
void Float_T::Set(Value newValue) {
  if (newValue->GetPrimitiveType() == PrimitiveType::Float) {
    this->value = static_cast<Float_T *>(newValue.get())->value;
  }
  if (newValue->GetPrimitiveType() == PrimitiveType::Int) {
    this->value = static_cast<Int_T *>(newValue.get())->value;
  }
}
Bool Float_T::Or(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    return Bool_T::New(this->value ||
                       static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    auto i =
        Bool_T::New(this->value || static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Float_T::And(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    return Bool_T::New(this->value &&
                       static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    auto i =
        Bool_T::New(this->value && static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Float_T::Less(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    return Bool_T::New(this->value <
                       static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    auto i =
        Bool_T::New(this->value < static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Float_T::Greater(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    return Bool_T::New(this->value >
                       static_cast<Float_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    auto i =
        Bool_T::New(this->value > static_cast<Int_T *>(other.get())->value);
    return i;
  }
  return False;
}

Value Float_T::Negate() { return Float_T::New(-value); }

Value String_T::Add(Value other) {
  return String_T::New(value + other->ToString());
}
void String_T::Set(Value newValue) {
  if (newValue->GetPrimitiveType() == PrimitiveType::String) {
    this->value = static_cast<String_T *>(newValue.get())->value;
  }
}

Bool Bool_T::Or(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Bool) {
    return Bool_T::New(this->value ||
                       static_cast<Bool_T *>(other.get())->value);
  }
  return False;
}
Bool Bool_T::And(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Bool) {
    return Bool_T::New(this->value &&
                       static_cast<Bool_T *>(other.get())->value);
  }
  return False;
}
Bool Bool_T::Not() { return Bool_T::New(!value); }
void Bool_T::Set(Value newValue) {
  if (newValue->GetPrimitiveType() == PrimitiveType::Bool) {
    this->value = static_cast<Bool_T *>(newValue.get())->value;
  }
  if (newValue->GetPrimitiveType() == PrimitiveType::Int) {
    this->value = (bool)static_cast<Int_T *>(newValue.get());
  }
}

string Callable_T::ToString() const {
  std::stringstream ss;
  ss << "func";
  auto type = std::dynamic_pointer_cast<CallableType>(this->type);

  ss << "(";
  for (const auto &param : params->Params()) {
    ss << param.name << ": " << param.type->Name();
    if (&param != &params->Params().back()) {
      ss << ", ";
    }
  }
  ss << ")";
  auto returnType = type->returnType->Name();
  ss << " -> " << returnType;
  return ss.str();
}
string Array_T::ToString() const { return Writer::ToString(this, {}); }
string NativeCallable_T::ToString() const {
  stringstream ss = {};
  ss << TypeSystem::Current()
            .FromCallable(function->returnType, function->parameterTypes)
            ->Name();
  return ss.str();
}

string Float_T::ToString() const { return std::to_string(value); }

string String_T::ToString() const { return value; }

string Bool_T::ToString() const {
  static string _TRUE = "true";
  static string _FALSE = "false";
  return value ? _TRUE : _FALSE;
}

string Null_T::ToString() const {
  static string null = "null";
  return null;
}

bool String_T::Equals(Value value) {
  if (value->GetPrimitiveType() == PrimitiveType::String) {
    return static_cast<String_T *>(value.get())->value == this->value;
  }
  return false;
}
Array Array_T::New(vector<Value> &values) {
  auto array = make_shared<Array_T>(values);
  return array;
}
bool Float_T::Equals(Value value) {
  if (value->GetPrimitiveType() == PrimitiveType::Float) {
    return static_cast<Float_T *>(value.get())->value == this->value;
  }
  if (value->GetPrimitiveType() == PrimitiveType::Float) {
    auto i = this->value == static_cast<Float_T *>(value.get())->value;
    return i;
  }
  return false;
}
bool Bool_T::Equals(Value value) {
  if (value->GetPrimitiveType() == PrimitiveType::Bool) {
    return static_cast<Bool_T *>(value.get())->value == this->value;
  }
  return false;
}
bool Array_T::Equals(Value value) { return value.get() == this; }
bool NativeCallable_T::Equals(Value value) { return value.get() == this; }
bool Callable_T::Equals(Value value) { return value.get() == this; }

bool Null_T::Equals(Value value) {
  return value == Value_T::Null ||
         value->GetPrimitiveType() == PrimitiveType::Null;
}

Value String_T::SubscriptAssign(Value key, Value value) {
  int64_t idx;
  string string;
  if (Ctx::TryGetInt(key, idx) && Ctx::TryGetString(value, string)) {
    if (string.length() == 0) {
      this->value[idx] = string[0];
    } else {
      this->value.insert(idx, string);
    }
  }
  return Null;
}
Value Value_T::Subscript(Value) { return Null; }
Value String_T::Subscript(Value key) {
  int64_t index;
  if (!Ctx::TryGetInt(key, index) || (size_t)index > value.length()) {
    return Null;
  }
  return Ctx::CreateString(std::string() + this->value[index]);
}

Value Value_T::SubscriptAssign(Value, Value) { return Null; }
Value Array_T::Subscript(Value key) {
  int64_t index;
  if (!Ctx::TryGetInt(key, index)) {
    return Null;
  }
  BoundsCheck(index);
  return values[index];
}
Value Array_T::SubscriptAssign(Value key, Value value) {
  int64_t idx;

  Type element_type;
  
  // if this is a templated array, like array<int>
  // and the template is well formed (has typeargs)
  if (auto template_t = std::dynamic_pointer_cast<TemplateType>(type);
      !template_t->typenames.empty()) {

    auto array_t = template_t->typenames[0];

    if (!value->type->Equals(array_t.get())) {
      throw TypeError(value->type, array_t);
    }
    // if not a template, this a generic array
    // But, we have to make sure that it's got a valid generic array type.
  } else if (!std::dynamic_pointer_cast<ArrayType>(type)) {
    throw TypeError(
        type,
        "invalid type for array. Array_T Value does not have a valid array "
        "type. This is basically impossible and is a language bug.");
  }

  if (Ctx::TryGetInt(key, idx)) {
    BoundsCheck(idx);
    values[idx] = value;
  }
  return Null;
}

namespace Values {
string TypeToString(PrimitiveType type) {
  switch (type) {
  
  case PrimitiveType::Invalid:
    return "invalid";
  case PrimitiveType::Null:
    return "null";
  case PrimitiveType::Float:
    return "float";
  case PrimitiveType::Int:
    return "int";
  case PrimitiveType::Bool:
    return "bool";
  case PrimitiveType::String:
    return "string";
  case PrimitiveType::Object:
    return "object";
  case PrimitiveType::Array:
    return "array";
  case PrimitiveType::Callable:
    return "callable";
  case Values::PrimitiveType::Tuple:
    return "tuple";
  case PrimitiveType::Lambda:
    return "property";
  }
  return "";
}

Value Value_T::Clone() { return make_shared<Null_T>(); }

Value Float_T::Clone() { return Ctx::CreateFloat(value); }
Value String_T::Clone() { return Ctx::CreateString(string(value)); }
Value Bool_T::Clone() { return Ctx::CreateBool(value); }

Value Array_T::Clone() {
  Array array = Ctx::CreateArray();
  for (const auto &value : values) {
    array->Push(value->Clone());
  }
  return array;
}
Value Callable_T::Clone() { return shared_from_this(); }

Array_T::~Array_T() {}

// TODO: add typing to tuple deconstruction.
auto Tuple_T::Deconstruct(vector<string> &idens) const -> void {
  // produce the maximum number of values available given
  // both arrays are at least that size.
  size_t max = std::min(idens.size(), this->values.size());

  // deconstruct the tuple into the fields.
  for (size_t i = 0; i < max; ++i) {
    auto &iden = idens[i];
    auto &value = values[i];
    ASTNode::context.CurrentScope()->Declare(iden, value, Mutability::Mut);
  }
  
  for (size_t i = max; i < idens.size(); ++i) {
    auto &iden = idens[i];
    ASTNode::context.CurrentScope()->Declare(iden, Ctx::Null(), Mutability::Mut);
  }
}

bool Tuple_T::Equals(Value other) {
  auto other_tuple = std::dynamic_pointer_cast<Tuple_T>(other);

  if (!other_tuple) {
    return false;
  }

  auto other_vals = other_tuple->values;

  if (other_vals.size() != this->values.size()) {
    return false;
  }

  size_t i = 0;
  for (const auto &v : other_vals) {
    if (!v->Equals(this->values[i])) {
      throw std::runtime_error(string(v->ToString() + " did not equal " +
                                      this->values[i]->ToString()));
      return false;
    }
    ++i;
  }

  return true;
}
Value Tuple_T::Clone() {
  vector<Value> values;
  for (const auto &v : this->values) {
    values.push_back(v->Clone());
  }

  return make_shared<Tuple_T>(values);
}
string Tuple_T::ToString() const {
  vector<string> strings;

  for (const auto &v : values) {
    strings.push_back(v->ToString());
  }

  if (!strings.empty()) {
    return "(" +
           std::accumulate(
               std::next(strings.begin()), strings.end(), strings[0],
               [](const string &a, const string &b) { return a + ", " + b; }) +
           ")";
  } else {
    return "()";
  }
}
Value Lambda_T::Clone() { return Ctx::Null(); }
bool Lambda_T::Equals(Value other) {
  auto o = std::dynamic_pointer_cast<Lambda_T>(other);
  if (o) {
    return o->lambda.get() == this->lambda.get();
  }
  return false;
}

Object Object_T::New(Scope scope) {
  // todo: validate this.
  if (!scope)
    scope = make_shared<Scope_T>(ASTNode::context.CurrentScope());
  return make_shared<Object_T>(scope);
}

Object_T::Object_T() : Value_T(TypeSystem::Current().Find("object")) {}

Tuple_T::Tuple_T(vector<Value> values) : Value_T(nullptr), values(values) {
  auto types = vector<Type>();
  for (const auto &v : values) {
    types.push_back(v->type);
  }
  this->type = TypeSystem::Current().FromTuple(types);
}

Value NativeCallable_T::Call(std::vector<Value> &args) {
  
  Value result;

  CheckParameterTypes(args);

  if (function->ptr)
    result = function->ptr(args);
  
  
  // we shouldn't need to do this
  //CheckReturnType(result);

  
  if (result == nullptr) {
    return Null;
  } else {
    return result;
  }
}
Value Callable_T::Call(ArgumentsPtr &args, TypeArgsPtr &type_args) {
  auto values = Call::GetArgsValueList(args);
  
  auto last_scope = ASTNode::context.CurrentScope();
  ASTNode::context.SetCurrentScope(scope);
  
  if (type_params)
    type_params->Apply(type_args->types);
  
  params->Apply(block->scope, values);
  auto result = block->Execute();
  
  
  
  ASTNode::context.SetCurrentScope(last_scope);
  switch (result.controlChange) {
  case ControlChange::None:
    return Value_T::Null;
  case ControlChange::Return: {
    CheckReturnType(result.value);
    return result.value;
  }
  default:
    throw std::runtime_error("Uncaught " + CC_ToString(result.controlChange));
  }
}

Value NativeCallable_T::Call(unique_ptr<Arguments> &args, TypeArgsPtr &type_args) {
  auto values = Call::GetArgsValueList(args);
  Value result;
  
  CheckParameterTypes(values);
  
  if (function->ptr)
    result = function->ptr(values);
  
  //CheckReturnType(result);
  
  
  if (result == nullptr) {
    return Null;
  } else {
    return result;
  }
}
Value Callable_T::Call(std::vector<Value> &values) {
  size_t i = 0;
  auto last_scope = ASTNode::context.CurrentScope();
  ASTNode::context.SetCurrentScope(scope);
  params->Apply(block->scope, values);
  auto result = block->Execute();
  ASTNode::context.SetCurrentScope(last_scope); 
  
  switch (result.controlChange) {
  case ControlChange::None:
    return Value_T::Null;
  case ControlChange::Return:
    return result.value;
  default:
    throw std::runtime_error("Uncaught " + CC_ToString(result.controlChange));
  }
}

Float_T::Float_T(double value) : Value_T(TypeSystem::Current().Float) {
  this->value = value;
}

Callable_T::Callable_T(const Type &returnType, BlockPtr &&block,
    ParametersPtr &&params, Scope scope, TypeParamsPtr &&type_params)
    : Value_T(TypeSystem::Current().FromCallable(returnType, params->ParamTypes())), block(std::move(block)), params(std::move(params)),
    type_params(std::move(type_params)), scope(scope) {
}
String_T::String_T(const string &value)
    : Value_T(TypeSystem::Current().String) {
  this->value = value;
}

Null_T::Null_T() : Value_T(TypeSystem::Current().Null) {}
Bool_T::Bool_T(bool value) : Value_T(TypeSystem::Current().Bool) {
  this->value = value;
}
Array_T::Array_T(vector<Value> init) : Value_T([&](){
  if (init.size() != 0) {
    return TypeSystem::Current().FindOrCreateTemplate(
        "array<" + init[0]->type->Name() + ">",
        TypeSystem::Current().Find("array"), {init[0]->type});
  } else {
    return TypeSystem::Current().Find("array");
  }
}()) {
  values = init;
}

Callable_T::~Callable_T() {}

Tuple_T::~Tuple_T() {}
NativeCallable_T::~NativeCallable_T() {}
Object_T::~Object_T() {}
Bool_T::~Bool_T() {}
Null_T::~Null_T() {}
Lambda_T::~Lambda_T() {}

Value Null_T::Clone() { return Ctx::Null(); }

Value Value_T::NullOfType(const Type &type) {
  auto null = make_shared<Null_T>();
  null->type = type;
  return null;
}

Callable_T::Callable_T(const Type &returnType, const vector<Type> &paramTypes) : Value_T(TypeSystem::Current().FromCallable(returnType,  paramTypes)) {
  
}
} // namespace Values
