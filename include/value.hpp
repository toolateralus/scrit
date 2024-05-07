#pragma once
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

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


// forward declare AST nodes.
struct Scope;
struct Identifier;
struct Block;
struct Parameters;
struct Arguments;
struct Expression;

// ease of use typedef.
typedef shared_ptr<Value_T> Value;
typedef shared_ptr<Bool_T> Bool;
typedef shared_ptr<String_T> String;
typedef shared_ptr<Array_T> Array;
typedef shared_ptr<Int_T> Int;
typedef shared_ptr<Float_T> Float;
typedef shared_ptr<Object_T> Object;

enum class ValueType {
  None,
  Float,
  Int,
  Bool,
  String,
  Object,
  Array,
  Callable,
};

struct Value_T : std::enable_shared_from_this<Value_T> {
  static Value Null;
  static Value Undefined;
  static Bool False;
  static Bool True;
  ValueType type = ValueType::None;
  
  Value_T(ValueType type) : type(type) {}
  
  virtual ~Value_T() {}
  virtual string ToString() const {
   return "null"; 
  }
  virtual bool Equals(Value value) { return value == Null; }
  virtual Value Add(Value other) { return Null; }
  virtual Value Subtract(Value other) { return Null; }
  virtual Value Multiply(Value other) { return Null; }
  virtual Value Divide(Value other) { return Null; }
  virtual Bool Or(Value other) { return False; }
  virtual Bool And(Value other) { return False; }
  virtual Bool Less(Value other) { return False; }
  virtual Bool Greater(Value other) { return False; }
  virtual Bool GreaterEquals(Value other) { return False; }
  virtual Bool LessEquals(Value other) { return False; }
  virtual Bool Not() { return False; }
  virtual Value Negate() { return Null; }
  virtual Value Get() { return Null; }
  virtual void Set(Value newValue) {}
  bool TypeEquals(Value other) { return typeid(other.get()) == typeid(*this); }
};



struct Null : Value_T {
  Null() : Value_T(ValueType::None){}
};

struct Undefined : Value_T{ 
  Undefined(): Value_T(ValueType::None){}
};

struct Int_T : Value_T {
  int value = 0;
  Int_T(int value) : Value_T(ValueType::Int) {
    this->value = value;
  }
  ~Int_T() {
    
  }
  virtual bool Equals(Value_T* value) {
    if (value->type == ValueType::Int) {
      return static_cast<Int_T*>(value)->value == this->value;
    }
    return false;
  };
  virtual Value Add(Value other) override {
    if (other->type == ValueType::Int) {
      return make_shared<Int_T>(this->value + static_cast<Int_T*>(other.get())->value);
    }
    return Value_T::Null;
  }
  virtual Value Subtract(Value other) override {
    if (other->type == ValueType::Int) {
      return make_shared<Int_T>(this->value - static_cast<Int_T*>(other.get())->value);
    }
    return Value_T::Null;
  }
  virtual Value Multiply(Value other) override {
    if (other->type == ValueType::Int) {
      return make_shared<Int_T>(this->value * static_cast<Int_T*>(other.get())->value);
    }
    return Value_T::Null;
  }
  virtual Value Divide(Value other) override {
    if (other->type == ValueType::Int) {
      return make_shared<Int_T>(this->value / static_cast<Int_T*>(other.get())->value);
    }
    return Value_T::Null;
  }
  virtual Value Get() override  {
    return shared_from_this();
  }
  virtual void Set(Value newValue) override  {
    if (newValue->type == ValueType::Int) {
      this->value = static_cast<Int_T*>(newValue.get())->value;
    }
  }
  virtual Bool Or(Value other) override {
    if (other->type == ValueType::Int) {
      return make_shared<Bool_T>(this->value || static_cast<Int_T*>(other.get())->value);
    }
    return False;
  }
  virtual Bool And(Value other) override { 
    if (other->type == ValueType::Int) {
      return make_shared<Bool_T>(this->value && static_cast<Int_T*>(other.get())->value);
    }
    return False;
  }
  virtual Bool Less(Value other) override { 
    if (other->type == ValueType::Int) {
      return make_shared<Bool_T>(this->value < static_cast<Int_T*>(other.get())->value);
    }
    return False;
   }
  virtual Bool Greater(Value other) override  { 
    if (other->type == ValueType::Int) {
      return make_shared<Bool_T>(this->value > static_cast<Int_T*>(other.get())->value);
    }
    return False;
   }
  virtual Bool GreaterEquals(Value other) override { 
    if (other->type == ValueType::Int) {
      return make_shared<Bool_T>(this->value >= static_cast<Int_T*>(other.get())->value);
    }
    return False;
  }
  virtual Bool LessEquals(Value other) override { 
    if (other->type == ValueType::Int) {
      return make_shared<Bool_T>(this->value <= static_cast<Int_T*>(other.get())->value);
    }
    return False;
  }
  virtual Value Negate() override { 
    return make_shared<Int_T>(-value);
  }
  virtual string ToString() const override {
   return std::to_string(value); 
  }
};
struct Float_T : Value_T {
  float value = 0.0f;
  Float_T(float value) : Value_T(ValueType::Float) {
    this->value = value;
  }
  ~Float_T() {}
  virtual bool Equals(Value value) override {
    if (value->type == ValueType::Float) {
      return static_cast<Float_T*>(value.get())->value == this->value;
    }
    return false;
  }
  virtual Value Add(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Float_T>(this->value + static_cast<Float_T*>(other.get())->value);
    }
    return Value_T::Null;
  }
  virtual Value Subtract(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Float_T>(this->value - static_cast<Float_T*>(other.get())->value);
    }
    return Value_T::Null;
  }
  virtual Value Multiply(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Float_T>(this->value * static_cast<Float_T*>(other.get())->value);
    }
    return Value_T::Null;
  }
  virtual Value Divide(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Float_T>(this->value / static_cast<Float_T*>(other.get())->value);
    }
    return Value_T::Null;
  }
  virtual Value Get() override {
    return shared_from_this();
  }
  virtual void Set(Value newValue) override {
    if (newValue->type == ValueType::Float) {
      this->value = static_cast<Float_T*>(newValue.get())->value;
    }
  }
  virtual Bool Or(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Bool_T>(this->value || static_cast<Float_T*>(other.get())->value);
    }
    return False;
  }
  virtual Bool And(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Bool_T>(this->value && static_cast<Float_T*>(other.get())->value);
    }
    return False;
  }
  virtual Bool Less(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Bool_T>(this->value < static_cast<Float_T*>(other.get())->value);
    }
    return False;
  }
  virtual Bool Greater(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Bool_T>(this->value > static_cast<Float_T*>(other.get())->value);
    }
    return False;
  }
  virtual Bool GreaterEquals(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Bool_T>(this->value >= static_cast<Float_T*>(other.get())->value);
    }
    return False;
  }
  virtual Bool LessEquals(Value other) override {
    if (other->type == ValueType::Float) {
      return make_shared<Bool_T>(this->value <= static_cast<Float_T*>(other.get())->value);
    }
    return False;
  }
  virtual Value Negate() override { 
    return make_shared<Float_T>(-value);
  }
  virtual string ToString() const override {
   return std::to_string(value); 
  }
 
};
struct String_T : Value_T {
  string value;
  String_T(const string& value) : Value_T(ValueType::String){
    this->value = value;
  }
  ~String_T() {}
  virtual bool Equals(Value value) override {
    if (value->type == ValueType::String) {
      return static_cast<String_T*>(value.get())->value == this->value;
    }
    return false;
  }
  virtual Value Add(Value other) override {
    if (other->type == ValueType::String) {
      return make_shared<String_T>(this->value + static_cast<String_T*>(other.get())->value);
    }
    return Value_T::Null;
  }
  virtual Value Get() override {
    return shared_from_this();
  }
  virtual void Set(Value newValue) override {
    if (newValue->type == ValueType::String) {
      this->value = static_cast<String_T*>(newValue.get())->value;
    }
  }
  string ToString() const override {
    return value;
  }
};
struct Bool_T : Value_T {
  bool value = false;
  Bool_T(bool value) : Value_T(ValueType::Bool){
    this->value = value;
  }
  ~Bool_T() {}
  virtual bool Equals(Value value) override {
    if (value->type == ValueType::Bool) {
      return static_cast<Bool_T*>(value.get())->value == this->value;
    }
    return false;
  }
  virtual Bool Or(Value other) override {
    if (other->type == ValueType::Bool) {
      return make_shared<Bool_T>(this->value || static_cast<Bool_T*>(other.get())->value);
    }
    return False;
  }
  virtual Bool And(Value other) override {
    if (other->type == ValueType::Bool) {
      return make_shared<Bool_T>(this->value && static_cast<Bool_T*>(other.get())->value);
    }
    return False;
  }
  virtual Value Get() override {
    return shared_from_this();
  }
  virtual Bool Not() override {
    return make_shared<Bool_T>(!value);
  }
  virtual void Set(Value newValue) override {
    if (newValue->type == ValueType::Bool) {
      this->value = static_cast<Bool_T*>(newValue.get())->value;
    }
  }
   virtual string ToString() const override {
   return std::to_string(value); 
  }
};


struct Object_T : Value_T {
  Object_T() : Value_T(ValueType::Object) {
    
  }
  shared_ptr<Scope> scope;
  Value GetMember(const string &name);
  void SetMember(const string &name, Value &value);
  virtual string ToString() const override;
};



struct Callable_T : Value_T {
  Callable_T(unique_ptr<Block> &&block, unique_ptr<Parameters> &&params);
  unique_ptr<Block> block;
  unique_ptr<Parameters> params;
  Value Call(unique_ptr<Arguments> args);
  string ToString() const override;
};

struct Array_T : Value_T {
  static Array New();
  static Array New(vector<unique_ptr<Expression>> &&init);
  Array_T();
  Array_T(vector<unique_ptr<Expression>> &&init);
  vector<unique_ptr<Expression>> initializer;
  vector<Value> values;
  Value At(Int index);
  void Assign(Int index, Value value);
  void Push(Value value);
  void Insert(Int index, Value value);
  Value Pop();
  Value Remove(Int index);
  string ToString() const override;
};
