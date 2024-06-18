#include <memory>
#include <scrit/native.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/value.hpp>
#include <sstream>

#define undefined Ctx::Undefined()
#define __args__ std::vector<Value> args

#pragma clang diagnostic ignored "-Wunused-parameter"

struct StringStream : Object_T {
  std::stringstream stream;
  StringStream() : Object_T() {
    scope = std::make_shared<Scope_T>();
    stream = {};
  }
  string ToString() const override {
    return stream.str();
  }
};

Value append(__args__) {
  if (args.size() < 2 || args[0]->GetType() != Values::ValueType::Object) {
    return undefined;
  }
  
  if (auto ss = dynamic_cast<StringStream*>(args[0].get())) {
    auto str = args[1]->ToString();
    ss->stream << str;
  }
  return undefined;
}

Value clear(__args__) {
  if (!args.empty() && args[0]->GetType() == Values::ValueType::Object) {
    auto ss = dynamic_cast<StringStream*>(args[0].get());
    ss->stream.clear();
  }
  return Ctx::Undefined();
}

Value create(__args__) {
  auto stream =  make_shared<StringStream>();
  if (!args.empty()) {
    for (const auto &arg: args) {
      stream->stream << arg->ToString();
    }
  }
  return stream;
}

Value str(__args__) {
  if (!args.empty() && args[0]->GetType() == Values::ValueType::Object) {
    auto ss = dynamic_cast<StringStream*>(args[0].get());
    return Ctx::CreateString(ss->stream.str());
  }
  return Ctx::Undefined();
}

extern "C" ScritModDef *InitScritModule_sstream() {
  ScritModDef *def = CreateModDef();
  *def->description = "String stream module used to test the ability to create extended objects for Scrit.";
  def->AddFunction("append", append);
  def->AddFunction("create", create);
  return def;
}