#include "native.hpp"
#include "context.hpp"
#include "value.hpp"
#include <memory>
#include <unordered_map>

std::unordered_map<string, NativeCallable> NativeFunctions::instantiatedCallables = {};

ScritModDef::ScritModDef() { 
  context = new Context(); 
}
ScritModDef::~ScritModDef() {
  delete context;
}

void ScritModDef::AddFunction(const std::string &name,
                              NativeFunctionPtr function) {
  functions[name] = function;
}

void ScritModDef::AddVariable(const std::string &name, Value value) {
  context->Insert(name, value);
}

Object ScritModDef::AsObject() {
  m_InstantiateCallables();
  auto object = std::make_shared<Object_T>();
  object->scope = context->scopes[0];
  return object;
}

void ScritModDef::m_InstantiateCallables() {
  for (const auto [name, func] : functions) {
    context->Insert(name, NativeFunctions::MakeCallable(func));
  }
}

NativeCallable NativeFunctions::MakeCallable(const NativeFunctionPtr fn) {
  return make_shared<NativeCallable_T>(fn);
}

NativeCallable NativeFunctions::GetCallable(const std::string &name) {
  auto fIt = instantiatedCallables.find(name);
  if (fIt != instantiatedCallables.end()) {
    return fIt->second;
  }
  auto registry = GetRegistry();
  auto it = registry.find(name);
  if (it != registry.end()) {
    auto func = it->second;
    auto callable = make_shared<NativeCallable_T>(func);;
    instantiatedCallables[name] = callable;
    return callable;
  }
  return nullptr;
}
