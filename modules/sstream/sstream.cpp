#include <memory>
#include <scrit/native.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/value.hpp>
#include <sstream>
#include "scrit/type.hpp"

#define undefined Ctx::Undefined()
#define __args__ std::vector<Value> args

#pragma clang diagnostic ignored "-Wunused-parameter"

struct StringStream : Value_T {
  std::stringstream stream = std::stringstream("");
  StringStream() : Value_T(TypeSystem::Current().Get("array")) {}
  string ToString() const override {
    return stream.str();
  }
  PrimitiveType GetPrimitiveType() const override {
    // Todo: add a less janky way to do this.
    // It seems silly to convince the interpreter we're an object when we don't use any of that.
    return PrimitiveType::Object;
  }
  bool Equals(Value v) override {
    // compare pointers? hard to say here. We're basically abusing the interpreter by saying this is not an object (with inheritance).
    return this == v.get();
  }
};

Value append(__args__) {
  if (args.size() < 2) {
    return undefined;
  }
  
  if (auto ss = std::dynamic_pointer_cast<StringStream>(args[0])) {
    auto str = args[1]->ToString();
    ss->stream << str;
  }
  return undefined;
}

Value clear(__args__) {
  if (args.empty()) {
    return undefined;
  }
  if (auto ss = std::dynamic_pointer_cast<StringStream>(args[0])) {
    ss->stream = {}; // the clear method in the ::stringstream does not seem to actually clear the contents but the 'error state' whatever that is.
  }
  return Ctx::Undefined();
}

Value create(__args__) {
  auto stream = make_shared<StringStream>();
  if (!args.empty()) {
    for (const auto &arg: args) {
      stream->stream << arg->ToString();
    }
  }
  return stream;
}

extern "C" ScritModDef *InitScritModule_sstream() {
  ScritModDef *def = CreateModDef();
  
  *def->description = 
  "String stream module used to test the ability to create extended objects for Scrit.\n" 
  "usage:\n\tsstream.create(<optional default value string>) to create the string object.\n"
  "\tsstream.append(stream, value) to append any value to the stream\n"
  "\tsstream.clear(stream) to clear the stream.\n";
  def->AddFunction("append", append);
  def->AddFunction("create", create);
  def->AddFunction("clear", clear);
  def->AddVariable("sstream_description", Ctx::CreateString(*def->description), Mutability::Const);
  return def;
}