#include <memory>
#include <scrit/native.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/value.hpp>
#include <sstream>

#define undefined Ctx::Undefined()

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

Value append(std ::vector<Value> args) {
  if (args.size() < 2 || args[0]->GetType() != Values::ValueType::Object) {
    return undefined;
  }
  
  if (auto ss = dynamic_cast<StringStream*>(args[0].get())) {
    auto str = args[1]->ToString();
    ss->stream << str;
  }
  return undefined;
}

Value create(std ::vector<Value> args) {
  return make_shared<StringStream>();
}

extern "C" ScritModDef *InitScritModule_sstream() {
  ScritModDef *def = CreateModDef();
  *def->description = "String stream module used to test the ability to create extended objects for Scrit.";
  def->AddFunction("append", append);
  def->AddFunction("create", create);
  return def;
}