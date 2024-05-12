#pragma once
#include "lexer.hpp"
#include "native.hpp"
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_set>
#include <vector>

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

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



enum class ValueType {
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
  Any, // only used for signature declarations.
};

string TypeToString(ValueType type);

struct Value_T {
  static Null Null;
  static Undefined Undefined;
  static Value InvalidCastException;
  static Bool False;
  static Bool True;

  virtual ValueType GetType() const = 0;
  virtual ~Value_T() {}
  Value_T() {}

  virtual string ToString() const = 0;
  virtual bool Equals(Value) = 0;
  virtual Value Add(Value) {
    return std::static_pointer_cast<Value_T>(Undefined);
  }
  virtual Value Subtract(Value) {
    return std::static_pointer_cast<Value_T>(Undefined);
  }
  virtual Value Multiply(Value) {
    return std::static_pointer_cast<Value_T>(Undefined);
  }
  virtual Value Divide(Value) {
    return std::static_pointer_cast<Value_T>(Undefined);
  }
  virtual Bool Or(Value) { return False; }
  virtual Bool And(Value) { return False; }
  virtual Bool Less(Value) { return False; }
  virtual Bool Greater(Value) { return False; }
  virtual Bool GreaterEquals(Value) { return False; }
  virtual Bool LessEquals(Value) { return False; }
  virtual Bool Not() { return False; }
  virtual Value Negate() {
    return std::static_pointer_cast<Value_T>(Undefined);
  }
  virtual Value Subscript(Value key);
  virtual Value SubscriptAssign(Value key, Value value);
  virtual void Set(Value value) { *this = *value; }
  bool TypeEquals(Value other) { return typeid(other.get()) == typeid(*this); }
};



struct Null_T : Value_T {
  ValueType GetType() const override { return ValueType::Null; }
  Null_T();
  string ToString() const override;
  bool Equals(Value value) override;
};
struct Undefined_T : Value_T {
  ValueType GetType() const override { return ValueType::Undefined; }
  Undefined_T();
  string ToString() const override;
  bool Equals(Value value) override;
};
struct Int_T : Value_T {
  int value = 0;
  Int_T(int value);
  ~Int_T() {}
  Int_T() = delete;

  static Int New(int value = 0) { return make_shared<Int_T>(value); }

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
  virtual Bool GreaterEquals(Value other) override;
  virtual Bool LessEquals(Value other) override;
  virtual Value Negate() override;
  virtual string ToString() const override;
  ValueType GetType() const override { return ValueType::Int; }
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
  virtual Bool GreaterEquals(Value other) override;
  virtual Bool LessEquals(Value other) override;
  virtual Value Negate() override;
  virtual string ToString() const override;
  ValueType GetType() const override { return ValueType::Float; }
};
struct String_T : Value_T {
  string value;
  String_T(const string &value);
  String_T() = delete;
  ~String_T() {}
  static String New(string value = "") { return make_shared<String_T>(value); }
  virtual bool Equals(Value value) override;
  virtual Value Add(Value other) override;
  virtual void Set(Value newValue) override;
  string ToString() const override;
  ValueType GetType() const override { return ValueType::String; }
  Value Subscript(Value key) override;
  Value SubscriptAssign(Value key, Value value) override;
};
struct Bool_T : Value_T {
  bool value = false;
  Bool_T(bool value);
  Bool_T() = delete;
  static Bool New(bool value = false) { return make_shared<Bool_T>(value); }
  ~Bool_T() {}
  virtual bool Equals(Value value) override;
  virtual Bool Or(Value other) override;
  virtual Bool And(Value other) override;
  virtual Bool Not() override;
  virtual void Set(Value newValue) override;
  virtual string ToString() const override;
  ValueType GetType() const override { return ValueType::Bool; }
};
struct Object_T : Value_T {
  Object_T(Scope scope);
  Scope scope;
  static Object New(Scope scope = nullptr) {
    if (!scope)
      scope = make_shared<Scope_T>();
    return make_shared<Object_T>(scope);
  }
  string WriteMembers(std::unordered_set<const Value_T *> foundObjs) const;
  Value GetMember(const string &name);
  void SetMember(const string &name, Value &value);
  virtual string ToString() const override;
  string ToString(std::unordered_set<const Value_T *> foundValues) const;
  bool Equals(Value value) override;
  ValueType GetType() const override { return ValueType::Object; }
  Value Subscript(Value key) override;
  Value SubscriptAssign(Value key, Value value) override;
};
struct Callable_T : Value_T {
  ~Callable_T();
  Callable_T(); // for native callables only.
  Callable_T(BlockPtr &&block, ParametersPtr &&params);
  BlockPtr block;
  ParametersPtr params;
  virtual Value Call(ArgumentsPtr &args);
  string ToString() const override;
  bool Equals(Value value) override;
  ValueType GetType() const override { return ValueType::Callable; }
};
struct NativeCallable_T : Callable_T {
  NativeCallable_T() = delete;
  NativeCallable_T(const NativeFunction &ptr);
  NativeFunction function;

  Value Call(ArgumentsPtr &args) override;

  string ToString() const override;
  bool Equals(Value value) override;
  ValueType GetType() const override { return ValueType::Callable; }
};
struct Array_T : Value_T {
  vector<ExpressionPtr> initializer;

  static Array New();
  static Array New(vector<ExpressionPtr> &&init);
  static Array New(std::vector<Value> &values);
  Array_T() = delete;
  Array_T(vector<ExpressionPtr> &&init);
  Array_T(vector<Value> init);

  vector<Value> values;

  Value At(Int index);
  void Assign(Int index, Value value);
  void Push(Value value);
  void Insert(Int index, Value value);
  Value Pop();
  Value Remove(Int index);
  string ToString() const override;
  bool Equals(Value value) override;
  ValueType GetType() const override { return ValueType::Array; }

  Value Subscript(Value key) override;
  Value SubscriptAssign(Value key, Value value) override;
};
} // namespace Values

struct Ctx {
  Ctx() = delete;
  static Bool CreateBool(const bool value = false);
  static String CreateString(const string value = "");
  static Int CreateInt(const int value = 0);
  static Float CreateFloat(const float value = 0.0f);
  static Object CreateObject(shared_ptr<Scope_T> scope = nullptr);
  static Array CreateArray(vector<Value> values = {});

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
                        args->Equals(Value_T::Undefined)),
         0)...};
    return isUndefined;
  }

  template <typename... Args> static bool IsNull(Args &&...args) {
    bool isNull = false;
    (void)std::initializer_list<int>{
        (isNull =
             isNull || (std::is_base_of<Value_T, std::decay_t<Args>>::value &&
                        args->Equals(Value_T::Null)),
         0)...};
    return isNull;
  }
  static bool IsUndefined(Value value);
  static bool IsNull(Value value);
};