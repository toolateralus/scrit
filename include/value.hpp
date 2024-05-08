#pragma once
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>
#include "context.hpp"
#include "lexer.hpp"
#include "native.hpp"

using std::string;
using std::vector;
using std::shared_ptr;
using std::make_shared;
using std::unique_ptr;


// forward declare value types
struct Value_T;
struct Bool_T;
struct Int_T;
struct Float_T;
struct String_T;
struct Object_T;
struct Array_T;
struct Scope_T;
struct Undefined_T;
struct Null_T;


// forward declare AST nodes.
struct Identifier;
struct Block;
struct Parameters;
struct Arguments;
struct Expression;

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

typedef unique_ptr<Expression> ExpressionPtr;
typedef unique_ptr<Block> BlockPtr;
typedef unique_ptr<Arguments> ArgumentsPtr;
typedef unique_ptr<Parameters> ParametersPtr;
typedef Value (*NativeFunctionPtr)(std::vector<Value>);

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
};


struct Value_T {
  static Null Null;
  static Undefined Undefined;
  static Value InvalidCastException;
  static Bool False;
  static Bool True;
  
  virtual ValueType GetType() = 0;
  virtual ~Value_T() {}
  Value_T() {}
  
  virtual string ToString() const = 0;
  virtual bool Equals(Value value) = 0;
  virtual Value Add(Value other) { return std::static_pointer_cast<Value_T>(Undefined); }
  virtual Value Subtract(Value other) { return std::static_pointer_cast<Value_T>(Undefined); }
  virtual Value Multiply(Value other) { return std::static_pointer_cast<Value_T>(Undefined); }
  virtual Value Divide(Value other) { return std::static_pointer_cast<Value_T>(Undefined); }
  virtual Bool Or(Value other) { return False; }
  virtual Bool And(Value other) { return False; }
  virtual Bool Less(Value other) { return False; }
  virtual Bool Greater(Value other) { return False; }
  virtual Bool GreaterEquals(Value other) { return False; }
  virtual Bool LessEquals(Value other) { return False; }
  virtual Bool Not() { return False; }
  virtual Value Negate() { return std::static_pointer_cast<Value_T>(Undefined); }
  virtual void Set(Value newValue) {}
  bool TypeEquals(Value other) { return typeid(other.get()) == typeid(*this); }
};
struct Null_T : Value_T {
  ValueType GetType() override {
    return ValueType::Null;
  }
  Null_T();
  string ToString() const override;
  bool Equals(Value value) override;
};
struct Undefined_T : Value_T{
  ValueType GetType() override {
    return ValueType::Undefined;
  }
  Undefined_T();
  string ToString() const override;
  bool Equals(Value value) override;
};
struct Int_T : Value_T {
  int value = 0;
  Int_T(int value);
  ~Int_T() {}
  Int_T() = delete;
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
  ValueType GetType() override {
    return ValueType::Int;
  }
};
struct Float_T : Value_T {
  float value = 0.0f;
  Float_T(float value);
  Float_T() = delete;
  ~Float_T() {}
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
  ValueType GetType() override {
    return ValueType::Float;
  }
};
struct String_T : Value_T {
  string value;
  String_T(const string &value);
  String_T() = delete;
  ~String_T() {}
  virtual bool Equals(Value value) override;
  virtual Value Add(Value other) override;
  virtual void Set(Value newValue) override;
  string ToString() const override;
  ValueType GetType() override {
    return ValueType::String;
  }
};
struct Bool_T : Value_T {
  bool value = false;
  Bool_T(bool value);
  Bool_T() = delete;
  ~Bool_T() {}
  virtual bool Equals(Value value) override;
  virtual Bool Or(Value other) override;
  virtual Bool And(Value other) override;
  virtual Bool Not() override;
  virtual void Set(Value newValue) override;
  virtual string ToString() const override;
  ValueType GetType() override {
    return ValueType::Bool;
  }
};
struct Object_T : Value_T {
  Object_T(Scope scope);
  Scope scope;
  Value GetMember(const string &name);
  void SetMember(const string &name, Value &value);
  virtual string ToString() const override;
  bool Equals(Value value) override;
  ValueType GetType() override {
    return ValueType::Object;
  }
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
  ValueType GetType() override {
    return ValueType::Callable;
  }
};
struct NativeCallable_T : Callable_T {
  NativeCallable_T() = delete;
  NativeCallable_T(NativeFunctionPtr ptr);
  NativeFunctionPtr function;
  Value Call(ArgumentsPtr &args) override;
  string ToString() const override;
  bool Equals(Value value) override;
  ValueType GetType() override {
    return ValueType::Callable;
  }
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
  ValueType GetType() override {
    return ValueType::Array;
  }
};