#pragma once
#include "ast.hpp"
#include "lexer.hpp"
#include "native.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

using namespace Values;

// forward declare AST nodes.
struct Identifier;
struct Block;
struct Parameters;
struct Arguments;
struct Expression;
struct Scope_T;
typedef shared_ptr<Scope_T> Scope;
typedef unique_ptr<Expression> ExpressionPtr;
typedef unique_ptr<Block> BlockPtr;
typedef unique_ptr<Arguments> ArgumentsPtr;
typedef unique_ptr<Parameters> ParametersPtr;
typedef Value (*NativeFunctionPtr)(std::vector<Value>);


namespace Values {

// forward declare value types
struct Value_T;
struct Bool_T;
struct Int_T;
struct Float_T;
struct String_T;
struct Object_T;
struct Array_T;
struct Undefined_T;
struct Null_T;

// ease of use typedef.
typedef shared_ptr<Value_T> Value;
typedef shared_ptr<Null_T> Null;
typedef shared_ptr<Undefined_T> Undefined;
typedef shared_ptr<Bool_T> Bool;
typedef shared_ptr<String_T> String;
typedef shared_ptr<Array_T> Array;
typedef shared_ptr<Int_T> Int;
typedef shared_ptr<Float_T> Float;
typedef shared_ptr<Object_T> Object;

enum class PrimitiveType {
  Invalid,
  Null,
  Undefined,
  Float,
  Int,
  Bool,
  String,
  Object,
  Array,
  Callable,
  Tuple,
  Lambda,
};

string TypeToString(PrimitiveType type);

struct Value_T : std::enable_shared_from_this<Value_T> {
  static Null VNULL;
  static Undefined UNDEFINED;
  static Bool False;
  static Bool True;
  
  Type type;
  
  virtual PrimitiveType GetPrimitiveType() const = 0;
  virtual ~Value_T() {
    
  }
  
  
  
  
  Value_T(const Type &type): type(type) {}
  
  virtual string ToString() const = 0;
  virtual bool Equals(Value) = 0;
  virtual Value Add(Value) {
    return std::static_pointer_cast<Value_T>(UNDEFINED);
  }
  virtual Value Subtract(Value) {
    return std::static_pointer_cast<Value_T>(UNDEFINED);
  }
  virtual Value Multiply(Value) {
    return std::static_pointer_cast<Value_T>(UNDEFINED);
  }
  virtual Value Divide(Value) {
    return std::static_pointer_cast<Value_T>(UNDEFINED);
  }
  virtual Bool Or(Value) { return False; }
  virtual Bool And(Value) { return False; }
  virtual Bool Less(Value) { return False; }
  virtual Bool Greater(Value) { return False; }
  virtual Bool Not() { return False; }
  virtual Value Negate() {
    return std::static_pointer_cast<Value_T>(UNDEFINED);
  }
  virtual Value Subscript(Value key);
  virtual Value SubscriptAssign(Value key, Value value);
  
  virtual void Set(Value) { }
  
  virtual Value Clone();
  
  template <typename T> T *Cast();
  template <typename T> PrimitiveType ValueTypeFromType();
};

struct Null_T : Value_T {
  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::Null; }
  Null_T();
  ~Null_T() override;
  string ToString() const override;
  bool Equals(Value value) override;
};
struct Undefined_T : Value_T {
  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::Undefined; }
  Undefined_T();
  ~Undefined_T() override;
  string ToString() const override;
  bool Equals(Value value) override;
};

struct Int_T : Value_T {
  int value = 0;
  Int_T(int value);
  ~Int_T() override {}
  Int_T() = delete;

  static Int New(int value = 0) { return make_shared<Int_T>(value); }
  virtual bool Equals(Value value) override;
  virtual void Set(Value newValue) override;
  virtual Bool Or(Value other) override;
  virtual Bool And(Value other) override;
  virtual Value Add(Value other) override;
  virtual Value Subtract(Value other) override;
  virtual Value Multiply(Value other) override;
  virtual Value Divide(Value other) override;
  virtual Bool Less(Value other) override;
  virtual Bool Greater(Value other) override;
  virtual Value Negate() override;
  virtual string ToString() const override;
  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::Int; }
  Value Clone() override;
};
struct Float_T : Value_T {
  float value = 0.0f;
  Float_T(float value);
  Float_T() = delete;
  ~Float_T() {}
  static Float New(float value = 0) { return make_shared<Float_T>(value); }
  virtual bool Equals(Value value) override;
  virtual Value Add(Value other) override;
  virtual Value Subtract(Value other) override;
  virtual Value Multiply(Value other) override;
  virtual Value Divide(Value other) override;
  virtual void Set(Value newValue) override;
  virtual Bool Or(Value other) override;
  virtual Bool And(Value other) override;
  virtual Bool Less(Value other) override;
  virtual Bool Greater(Value other) override;
  virtual Value Negate() override;
  virtual string ToString() const override;
  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::Float; }
  Value Clone() override;
};
struct String_T : Value_T {
  string value;
  String_T(const string &value);
  String_T() = delete;
  ~String_T() {}
  static String New(string value = std::string("")) { return make_shared<String_T>(value); }
  virtual bool Equals(Value value) override;
  virtual Value Add(Value other) override;
  virtual void Set(Value newValue) override;
  string ToString() const override;
  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::String; }
  Value Subscript(Value key) override;
  Value SubscriptAssign(Value key, Value value) override;
  Value Clone() override;
};
struct Bool_T : Value_T {
  bool value = false;
  Bool_T(bool value);
  Bool_T() = delete;
  static Bool New(bool value = false) { return make_shared<Bool_T>(value); }
  ~Bool_T() override;
  virtual bool Equals(Value value) override;
  virtual Bool Or(Value other) override;
  virtual Bool And(Value other) override;
  virtual Bool Not() override;
  virtual void Set(Value newValue) override;
  virtual string ToString() const override;
  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::Bool; }
  Value Clone() override;
};
struct Object_T : Value_T {
  Object_T(Scope scope);
  Object_T();
  ~Object_T() override;
  Scope scope;
  static Object New(Scope scope = nullptr);
  
  bool operator==(Object_T *other);
  
  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::Object; }
  
  virtual string ToString() const override;
  virtual Value GetMember(const string &name);
  virtual void SetMember(const string &name, Value value, Mutability mutability = Mutability::Const);
  virtual bool HasMember(const string &name);
  
  virtual bool Equals(Value value) override;
  virtual Value Subscript(Value key) override;
  virtual Value SubscriptAssign(Value key, Value value) override;
  Value CallOpOverload(Value &other, const string &op_key);
  virtual Value Add(Value other) override;
  virtual Value Subtract(Value other) override;
  virtual Value Multiply(Value other) override;
  virtual Value Divide(Value other) override;
  virtual Bool Less(Value other) override;
  virtual Bool Greater(Value other) override;
  virtual Value Clone() override;
};

struct Callable_T : Value_T {
  ~Callable_T() override;
  Callable_T(); // for native callables only.
  Callable_T(const Type &returnType, BlockPtr &&block, ParametersPtr &&params);
  BlockPtr block;
  ParametersPtr params;
  virtual Value Call(ArgumentsPtr &args);
  virtual Value Call(std::vector<Value> &args);
  string ToString() const override;
  bool Equals(Value value) override;
  Value Clone() override;

  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::Callable; }
};

struct NativeCallable_T : Callable_T {
  NativeCallable_T() = delete;
  NativeCallable_T(const NativeFunctionPtr &ptr);
  ~NativeCallable_T() override;
  NativeFunctionPtr function;
  Value Call(ArgumentsPtr &args) override;
  Value Call(std::vector<Value> &args) override;
  string ToString() const override;
  bool Equals(Value value) override;
  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::Callable; }
};
struct Array_T : Value_T {
  vector<ExpressionPtr> initializer;
  ~Array_T() override;

  static Array New();
  static Array New(vector<ExpressionPtr> &&init);
  static Array New(std::vector<Value> &values);
  Array_T() = delete;
  Array_T(vector<ExpressionPtr> &&init);
  Array_T(vector<Value> init);

  vector<Value> values = {};

  Value At(Int index);
  void Assign(Int index, Value value);
  void Push(Value value);
  void Insert(Int index, Value value);
  Value Pop();
  Value Remove(Int index);
  string ToString() const override;
  bool Equals(Value value) override;
  PrimitiveType GetPrimitiveType() const override { return PrimitiveType::Array; }
  
  Value Subscript(Value key) override;
  Value SubscriptAssign(Value key, Value value) override;
  Value Clone() override;
  
  std::vector<Value>::const_iterator begin() const {
    return values.begin();
  }
  std::vector<Value>::const_iterator end() const {
    return values.end();
  }
  std::vector<Value>::const_reverse_iterator rbegin() const {
    return values.rbegin();
  }
  std::vector<Value>::const_reverse_iterator rend() const {
    return values.rend();
  }
};

struct Tuple_T : Value_T {
  ~Tuple_T() override;
  vector<Value> values = {};
  Tuple_T(vector<Value> values);
  auto Deconstruct(vector<IdentifierPtr> &idens) const -> void;
  PrimitiveType GetPrimitiveType() const override {
    return PrimitiveType::Tuple;
  }
  string ToString() const override;
  bool Equals(Value other) override;
  Value Clone() override;
};

struct Lambda_T : Value_T {
  ExpressionPtr lambda;
  ~Lambda_T() override;
  Lambda_T(ExpressionPtr &&lambda) : Value_T(lambda->type), lambda(std::move(lambda)) {}
  Value Evaluate() {
    return lambda->Evaluate();
  }
  PrimitiveType GetPrimitiveType() const override {
    return PrimitiveType::Lambda;
  }
  string ToString() const override {
    return "property: " + lambda->Evaluate()->ToString();
  }
  bool Equals(Value other) override;
  Value Clone() override;
};

template <typename T> PrimitiveType Value_T::ValueTypeFromType() {
  auto &t = typeid(T);
  if (t == typeid(String_T)) {
    return PrimitiveType::String;
  } else if (t == typeid(Int_T)) {
    return PrimitiveType::Int;
  } else if (t == typeid(Float_T)) {
    return PrimitiveType::Float;
  } else if (t == typeid(Bool_T)) {
    return PrimitiveType::Bool;
  } else if (t == typeid(Object_T)) {
    return PrimitiveType::Object;
  } else if (t == typeid(Array_T)) {
    return PrimitiveType::Array;
  } else {
    throw std::runtime_error("Cannot deduce type for " + string(t.name()) + ". This function is used for extracting values, and type checking while doing so. Directly use GetType() for Callable, Undefined, and other immutable values.");
  }
  
}

template <typename T> T *Value_T::Cast() {
  if (ValueTypeFromType<T>() == GetPrimitiveType() && typeid(T) == typeid(*this)) {
    return static_cast<T*>(this);
  } 
  throw std::runtime_error(
      "invalid cast from : " + string(typeid(*this).name()) +
      "to : " + string(typeid(T).name()));
}


} // namespace Values

struct Ctx {
  Ctx() = delete;

  static Value Null();
  static Value Undefined();
  static Bool CreateBool(const bool value = false);
  static String CreateString(const string value = std::string(""));
  static Int CreateInt(const int value = 0);
  static Float CreateFloat(const float value = 0.0f);
  static Object CreateObject(shared_ptr<Scope_T> scope = nullptr);
  static Array CreateArray(vector<Value> values);
  static Array CreateArray();

  static Array FromFloatVector(vector<float> &values);
  static Array FromStringVector(vector<string> &values);
  static Array FromBoolVector(vector<bool> &values);
  static Array FromIntVector(vector<int> &values);

  static bool TryGetString(Value str, string &result);
  static bool TryGetInt(Value value, int &result);
  static bool TryGetFloat(Value value, float &result);
  static bool TryGetBool(Value value, bool &result);
  static bool TryGetObject(Value value, Object &result);
  static bool TryGetArray(Value value, Array &result);
  template <typename... Args> static bool IsUndefined(Args &&...args) {
    bool isUndefined = false;
    (void)std::initializer_list<int>{
        (isUndefined = isUndefined ||
                       (std::is_base_of<Value_T, std::decay_t<Args>>::value &&
                        args->Equals(Value_T::UNDEFINED)),
         0)...};
    return isUndefined;
  }
  template <typename... Args> static bool IsNull(Args &&...args) {
    bool isNull = false;
    (void)std::initializer_list<int>{
        (isNull =
             isNull || (std::is_base_of<Value_T, std::decay_t<Args>>::value &&
                        args->Equals(Value_T::VNULL)),
         0)...};
    return isNull;
  }
  static bool IsUndefined(Value value);
  static bool IsNull(Value value);
};

