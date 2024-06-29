#include "value.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "serializer.hpp"
#include "type.hpp"

#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>

Bool Value_T::True = Bool_T::New(true);
Bool Value_T::False = Bool_T::New(false);
Null Value_T::VNULL = make_shared<Null_T>();
Undefined Value_T::UNDEFINED = make_shared<::Undefined_T>();

Value Callable_T::Call(ArgumentsPtr &args) {

  auto scope = ASTNode::context.PushScope();
  auto values = Call::GetArgsValueList(args);

  size_t i = 0;
  for (const auto &[key, value] : params->map) {
    if (i >= values.size()) {
      if (value.value != nullptr) {
        scope->Set(key, value.value);
      }
      continue;
    }

    if (value.type == values[i]->type) {
      scope->Set(key, values[i]);
    } else {
      throw std::runtime_error(
          "invalid type argument for function call. expected: " +
          value.type->name + " got: " + values[i]->type->name);
    }
    i++;
  }

  auto result = block->Execute();

  ASTNode::context.PopScope();

  switch (result.controlChange) {
  case ControlChange::None:
    return Value_T::VNULL;
  case ControlChange::Return: {
    auto this_type = std::dynamic_pointer_cast<CallableType>(type);
    if (this_type && Type_T::Equals(result.value->type.get(), this_type->returnType.get())) {
      return result.value;
    } else if (this_type) {
      throw std::runtime_error("type error: function returned the wrong type.\nexpected: " + this_type->returnType->name + "\ngot: " + result.value->type->name);
    } else {
      throw std::runtime_error("interpreter error: function did not have a return type.");
    }
  }
  default:
    throw std::runtime_error("Uncaught " + CC_ToString(result.controlChange));
  }
}

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

Array Array_T::New(vector<ExpressionPtr> &&init) {
  return make_shared<Array_T>(std::move(init));
}
Array Array_T::New() {
  auto values = vector<Value>{};
  return make_shared<Array_T>(values);
}

Value NativeCallable_T::Call(std::vector<Value> &args) {
  ASTNode::context.PushScope();
  Value result;
  if (function != nullptr)
    result = function(args);

  ASTNode::context.PopScope();
  if (result == nullptr) {
    return UNDEFINED;
  } else {
    return result;
  }
}

Value NativeCallable_T::Call(unique_ptr<Arguments> &args) {
  ASTNode::context.PushScope();
  auto values = Call::GetArgsValueList(args);
  Value result;

  if (function != nullptr)
    result = function(values);

  ASTNode::context.PopScope();
  if (result == nullptr) {
    return UNDEFINED;
  } else {
    return result;
  }
}
NativeCallable_T::NativeCallable_T(const NativeFunctionPtr &function)
    : function(function) {}

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
  return Value_T::VNULL;
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
  return Value_T::VNULL;
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
  return Value_T::VNULL;
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
  return Value_T::VNULL;
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
  std::stringstream ss = {};
  ss << "callable(";
  size_t i = 0;
  for (const auto &[name, value] : params->map) {
    ss << name;
    if (i != params->map.size()) {
      ss << ", ";
    }
    i++;
  }
  ss << ")";
  return ss.str();
}
string Array_T::ToString() const { return Writer::ToString(this, {}); }
string NativeCallable_T::ToString() const {
  stringstream ss = {};
  ss << "native_callable()";
  return ss.str();
}

string Float_T::ToString() const { return std::to_string(value); }

string String_T::ToString() const { return value; }

string Bool_T::ToString() const {
  static string _TRUE = "true";
  static string _FALSE = "false";
  return value ? _TRUE : _FALSE;
}
string Undefined_T::ToString() const {
  static string undefined = "undefined";
  return undefined;
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

bool Undefined_T::Equals(Value value) {
  return value.get() == this ||
         value->GetPrimitiveType() == PrimitiveType::Undefined;
}
bool Null_T::Equals(Value value) {
  return value == Value_T::VNULL ||
         value->GetPrimitiveType() == PrimitiveType::Null;
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
Value Value_T::Subscript(Value) { return UNDEFINED; }
Value String_T::Subscript(Value key) {
  int index;
  if (!Ctx::TryGetInt(key, index) || (size_t)index > value.length()) {
    return UNDEFINED;
  }
  return Ctx::CreateString(std::string() + this->value[index]);
}

Value Value_T::SubscriptAssign(Value, Value) { return UNDEFINED; }
Value Array_T::Subscript(Value key) {
  int index;
  if (!Ctx::TryGetInt(key, index)) {
    return UNDEFINED;
  }
  // TODO: fix the issue where a default constructed array is of insane size.
  // 12391241792 type stuff.
  const size_t size = values.size();
  if ((size_t)index >= size) {
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
string TypeToString(PrimitiveType type) {
  switch (type) {

  case PrimitiveType::Invalid:
    return "invalid";
  case PrimitiveType::Null:
    return "null";
  case PrimitiveType::Undefined:
    return "undefined";
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

Value Value_T::Clone() { return Value_T::UNDEFINED; }

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

auto Tuple_T::Deconstruct(vector<IdentifierPtr> &idens) const -> void {
  // produce the maximum number of values available given
  // both arrays are at least that size.
  size_t max = std::min(idens.size(), this->values.size());

  // deconstruct the tuple into the fields.
  for (size_t i = 0; i < max; ++i) {
    auto &iden = idens[i];
    auto &value = values[i];
    ASTNode::context.Insert(iden->name, value, Mutability::Mut);
  }

  for (size_t i = max; i < idens.size(); ++i) {
    auto &iden = idens[i];
    ASTNode::context.Insert(iden->name, Ctx::Undefined(), Mutability::Mut);
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
Value Lambda_T::Clone() { return Ctx::Undefined(); }
bool Lambda_T::Equals(Value other) {
  auto o = std::dynamic_pointer_cast<Lambda_T>(other);
  if (o) {
    return o->lambda.get() == this->lambda.get();
  }
  return false;
}

Object Object_T::New(Scope scope) {
  if (!scope)
    scope = make_shared<Scope_T>();
  return make_shared<Object_T>(scope);
}

Object_T::Object_T() : Value_T(TypeSystem::Current().Get("object")) {}

Tuple_T::Tuple_T(vector<Value> values) : Value_T(nullptr), values(values) {
  auto types = vector<Type>();
  for (const auto &v : values) {
    types.push_back(v->type);
  }
  this->type = TypeSystem::Current().FromTuple(types);
}

Value Callable_T::Call(std::vector<Value> &values) {
  auto scope = ASTNode::context.PushScope();
  size_t i = 0;
  for (const auto &[key, value] : params->map) {
    if (i < values.size()) {
      scope->Set(key, values[i]);
    } else if (value.value != nullptr) {
      scope->Set(key, value.value);
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
Float_T::Float_T(float value) : Value_T(TypeSystem::Current().Get("float")) {
  this->value = value;
}
Array_T::Array_T(vector<ExpressionPtr> &&init)
    : Value_T(TypeSystem::Current().Get("array")) {
  initializer = std::move(init);
  for (auto &arg : initializer) {
    auto value = arg->Evaluate();
    Push(value);
  }
}
Callable_T::Callable_T(const Type &returnType, BlockPtr &&block,
                       ParametersPtr &&params)
    : Value_T(nullptr), block(std::move(block)), params(std::move(params)) {
  this->type = TypeSystem::Current().FromCallable(returnType,
                                                  this->params->ParamTypes());
}
String_T::String_T(const string &value)
    : Value_T(TypeSystem::Current().Get("string")) {
  this->value = value;
}
// this is only for native callables.
// Todo: implement native callable type.
Callable_T::Callable_T()
    : Value_T(TypeSystem::Current().Get("native_callable")) {}

Null_T::Null_T() : Value_T(TypeSystem::Current().Get("null")) {}
Undefined_T::Undefined_T() : Value_T(TypeSystem::Current().Get("undefined")) {}
Bool_T::Bool_T(bool value) : Value_T(TypeSystem::Current().Get("bool")) {
  this->value = value;
}
Array_T::Array_T(vector<Value> init) : Value_T(nullptr) {
  this->values = init;
  if (init.size() != 0) {
    this->type = TypeSystem::Current().GetOrCreateTemplate(
        "array<" + init[0]->type->name + ">",
        TypeSystem::Current().Get("array"), {init[0]->type});
  } else {
    this->type = TypeSystem::Current().Get("array");
  }
}

Callable_T::~Callable_T() {}

Tuple_T::~Tuple_T() {}
NativeCallable_T::~NativeCallable_T() {}
Object_T::~Object_T() {}
Bool_T::~Bool_T() {}
Undefined_T::~Undefined_T() {}
Null_T::~Null_T() {}
Lambda_T::~Lambda_T() {}
} // namespace Values

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
Value Ctx::Undefined() { return Value_T::UNDEFINED; }
Value Ctx::Null() { return Value_T::VNULL; }

Bool Ctx::CreateBool(const bool value) { return Bool_T::New(value); }
String Ctx::CreateString(const string value) { return String_T::New(value); }
Int Ctx::CreateInt(const int value) { return Int_T::New(value); }
Float Ctx::CreateFloat(const float value) { return Float_T::New(value); }
Object Ctx::CreateObject(Scope scope) { return Object_T::New(scope); }
Array Ctx::CreateArray(vector<Value> values) { return Array_T::New(values); }

bool Ctx::TryGetArray(Value value, Array &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Array) {
    result = std::dynamic_pointer_cast<Array_T>(value);
    return true;
  }
  return false;
}
bool Ctx::TryGetObject(Value value, Object &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Object) {
    result = std::dynamic_pointer_cast<Object_T>(value);
    return true;
  }
  return false;
}
bool Ctx::TryGetBool(Value value, bool &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Bool) {
    result = static_cast<Bool_T *>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::TryGetFloat(Value value, float &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Float) {
    result = static_cast<Float_T *>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::TryGetInt(Value value, int &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Int) {
    result = static_cast<Int_T *>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::TryGetString(Value value, string &result) {
  if (value->GetPrimitiveType() == PrimitiveType::String) {
    result = static_cast<String_T *>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::IsUndefined(Value value) { return value->Equals(Value_T::UNDEFINED); }
bool Ctx::IsNull(Value value) { return value->Equals(Value_T::VNULL); }

Values::Array Ctx::CreateArray() { return Array_T::New(); }
