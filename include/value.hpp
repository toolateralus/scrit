#pragma once
#include <memory>

#include <string>
#include <typeinfo>
#include <vector>

using std::string;
using std::vector;
using std::shared_ptr;
using std::make_shared;

struct Bool;

struct Value : std::enable_shared_from_this<Value> {
  static shared_ptr<Value> Null;
  static shared_ptr<Value> Undefined;
  static shared_ptr<Bool> False;
  static shared_ptr<Bool> True;
  virtual ~Value() {}
  virtual bool Equals(shared_ptr<Value> value) { return value == Null; }
  virtual shared_ptr<Value> Add(shared_ptr<Value> other) { return Null; }
  virtual shared_ptr<Value> Subtract(shared_ptr<Value> other) { return Null; }
  virtual shared_ptr<Value> Multiply(shared_ptr<Value> other) { return Null; }
  virtual shared_ptr<Value> Divide(shared_ptr<Value> other) { return Null; }
  virtual shared_ptr<Bool> Or(shared_ptr<Value> other) { return False; }
  virtual shared_ptr<Bool> And(shared_ptr<Value> other) { return False; }
  virtual shared_ptr<Bool> Less(shared_ptr<Value> other) { return False; }
  virtual shared_ptr<Bool> Greater(shared_ptr<Value> other) { return False; }
  virtual shared_ptr<Bool> GreaterEquals(shared_ptr<Value> other) { return False; }
  virtual shared_ptr<Bool> LessEquals(shared_ptr<Value> other) { return False; }
  virtual shared_ptr<Value> Get() { return Null; }
  virtual void Set(shared_ptr<Value> newValue) {}
  bool TypeEquals(shared_ptr<Value> other) { return typeid(other.get()) == typeid(*this); }
};

struct Null : Value {};
struct Undefined : Value {};



struct Int : Value {
  int value = 0;
  Int(int value) {
    this->value = value;
  }
  ~Int() {
    
  }
  virtual bool Equals(Value* value) {
    if (auto i = dynamic_cast<Int*>(value)) {
      return i == value;
    }
    return false;
  };
  virtual shared_ptr<Value> Add(shared_ptr<Value> other) override {
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Int>(i->value + this->value);
    }
    return Value::Null;
  }
  virtual shared_ptr<Value> Subtract(shared_ptr<Value> other) override {
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Int>(i->value - this->value);
    }
    return Value::Null;
  }
  virtual shared_ptr<Value> Multiply(shared_ptr<Value> other) override {
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Int>(i->value * this->value);
    }
    return Value::Null;
  }
  virtual shared_ptr<Value> Divide(shared_ptr<Value> other) override {
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Int>(i->value / this->value);
    }
    return Value::Null;
  }
  virtual shared_ptr<Value> Get() override  {
    return shared_from_this();
  }
  virtual void Set(shared_ptr<Value> newValue) override  {
    if (auto i = dynamic_cast<Int*>(newValue.get())) {
      this->value = i->value;
    }
  }
  virtual shared_ptr<Bool> Or(shared_ptr<Value> other) override {
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Bool>(i->value || this->value);
    }
    return False;
  }
  virtual shared_ptr<Bool> And(shared_ptr<Value> other) override { 
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Bool>(i->value && this->value);
    }
    return False;
  }
  virtual shared_ptr<Bool> Less(shared_ptr<Value> other) override { 
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Bool>(i->value < this->value);
    }
    return False;
   }
  virtual shared_ptr<Bool> Greater(shared_ptr<Value> other) override  { 
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Bool>(i->value > this->value);
    }
    return False;
   }
  virtual shared_ptr<Bool> GreaterEquals(shared_ptr<Value> other) override { 
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Bool>(i->value >= this->value);
    }
    return False;
  }
  virtual shared_ptr<Bool> LessEquals(shared_ptr<Value> other) override { 
    if (auto i = dynamic_cast<Int*>(other.get())) {
      return make_shared<Bool>(i->value <= this->value);
    }
    return False;
  }
  
};
struct Float : Value {
  float value = 0.0f;
  Float(float value) {
    this->value = value;
  }
  ~Float() {}
  virtual bool Equals(shared_ptr<Value> value) override {
    if (auto f = dynamic_cast<Float*>(value.get())) {
      return f->value == this->value;
    }
    return false;
  }
  virtual shared_ptr<Value> Add(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Float>(f->value + this->value);
    }
    return Value::Null;
  }
  virtual shared_ptr<Value> Subtract(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Float>(f->value - this->value);
    }
    return Value::Null;
  }
  virtual shared_ptr<Value> Multiply(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Float>(f->value * this->value);
    }
    return Value::Null;
  }
  virtual shared_ptr<Value> Divide(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Float>(f->value / this->value);
    }
    return Value::Null;
  }
  virtual shared_ptr<Value> Get() override {
    return shared_from_this();
  }
  virtual void Set(shared_ptr<Value> newValue) override {
    if (auto f = dynamic_cast<Float*>(newValue.get())) {
      this->value = f->value;
    }
  }
  virtual shared_ptr<Bool> Or(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Bool>(f->value || this->value);
    }
    return False;
  }
  virtual shared_ptr<Bool> And(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Bool>(f->value && this->value);
    }
    return False;
  }
  virtual shared_ptr<Bool> Less(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Bool>(f->value < this->value);
    }
    return False;
  }
  virtual shared_ptr<Bool> Greater(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Bool>(f->value > this->value);
    }
    return False;
  }
  virtual shared_ptr<Bool> GreaterEquals(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Bool>(f->value >= this->value);
    }
    return False;
  }
  virtual shared_ptr<Bool> LessEquals(shared_ptr<Value> other) override {
    if (auto f = dynamic_cast<Float*>(other.get())) {
      return make_shared<Bool>(f->value <= this->value);
    }
    return False;
  }
};
struct String : Value {
  string value;
  String(const string& value) {
    this->value = value;
  }
  ~String() {}
  virtual bool Equals(shared_ptr<Value> value) override {
    if (auto s = dynamic_cast<String*>(value.get())) {
      return s->value == this->value;
    }
    return false;
  }
  virtual shared_ptr<Value> Add(shared_ptr<Value> other) override {
    if (auto s = dynamic_cast<String*>(other.get())) {
      return make_shared<String>(s->value + this->value);
    }
    return Value::Null;
  }
  virtual shared_ptr<Value> Get() override {
    return shared_from_this();
  }
  virtual void Set(shared_ptr<Value> newValue) override {
    if (auto s = dynamic_cast<String*>(newValue.get())) {
      this->value = s->value;
    }
  }
};
struct Bool : Value {
  bool value = false;
  Bool(bool value) {
    this->value = value;
  }
  ~Bool() {}
  virtual bool Equals(shared_ptr<Value> value) override {
    if (auto b = dynamic_cast<Bool*>(value.get())) {
      return b->value == this->value;
    }
    return false;
  }
  virtual shared_ptr<Bool> Or(shared_ptr<Value> other) override {
    if (auto b = dynamic_cast<Bool*>(other.get())) {
      return make_shared<Bool>(b->value || this->value);
    }
    return False;
  }
  virtual shared_ptr<Bool> And(shared_ptr<Value> other) override {
    if (auto b = dynamic_cast<Bool*>(other.get())) {
      return make_shared<Bool>(b->value && this->value);
    }
    return False;
  }
  virtual shared_ptr<Value> Get() override {
    return shared_from_this();
  }
  virtual void Set(shared_ptr<Value> newValue) override {
    if (auto b = dynamic_cast<Bool*>(newValue.get())) {
      this->value = b->value;
    }
  }
};