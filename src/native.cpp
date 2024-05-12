#include "native.hpp"
#include "context.hpp"
#include "value.hpp"
#include <cstdlib>
#include <dlfcn.h>
#include <stdexcept>
#include <unordered_map>
#include "value.hpp"

std::unordered_map<std::string, NativeCallable>
    NativeFunctions::instantiatedCallables = {};


extern "C" void AddFunction(ScritModDef *mod, const string &name,
                const NativeFunctionPtr &ptr, const ValueType retType, std::vector<std::pair<ValueType, std::string>> args) {
  (*mod->functions).push_back(NativeFunction::Create(name, ptr, retType, args));
}
extern "C" void AddFunctionNoInfo(ScritModDef *mod, const string &name,
                const NativeFunctionPtr &ptr) {
  (*mod->functions).push_back(NativeFunction::Create(name, ptr));
}



extern "C" void AddVariable(ScritModDef *mod, const std::string &name, Value value) {
  mod->context->Insert(name, value);
}

Object ScritModDefAsObject(ScritModDef *mod) {
  m_InstantiateCallables(mod);
  auto object = Object_T::New(mod->context->scopes[0]);
  return object;
}

void m_InstantiateCallables(ScritModDef *module) {
  auto context = module->context;
  for (const auto &func : *module->functions) {
    context->Insert(func.name, NativeFunctions::MakeCallable(func));
  }
}

NativeCallable NativeFunctions::MakeCallable(const NativeFunction &fn) {
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
    auto callable = make_shared<NativeCallable_T>(func);
    ;
    instantiatedCallables[name] = callable;
    return callable;
  }
  return nullptr;
}

ScritModDef* LoadScritModule(const std::string &name, const std::string &path) {
  void *handle = dlopen(path.c_str(), RTLD_NOW);
  if (!handle) {
    throw std::runtime_error(dlerror());
  }
  
  auto fnName = "InitScritModule_" + name;
  void *func = dlsym(handle, fnName.c_str());
  if (!func) {
    throw std::runtime_error(dlerror());
  }
  ScriptModInitFuncPtr function = (ScriptModInitFuncPtr)func;

  if (!function) { 
    dlclose(handle);
    throw std::runtime_error(
        "Invalid function signature on " + fnName +
        ". This function must return a ScritModDef* and take no arguments.");
  }
  
  auto mod = function();
  return mod;
}
ScritModDef* CreateModDef() { 
  ScritModDef *mod = (ScritModDef*)malloc(sizeof(ScritModDef));
  mod->context = new Context(); 
  mod->description = new string();
  mod->functions = new std::vector<NativeFunction>();
  return mod;
}
std::unordered_map<std::string, NativeFunction> &
NativeFunctions::GetRegistry() {
  static std::unordered_map<std::string, NativeFunction> reg;
  return reg;
}
bool NativeFunctions::Exists(const std::string &name) {
  return GetRegistry().contains(name);
}

void RegisterFunction(const std::string &name, const NativeFunctionPtr &function, const ValueType returnType, const std::initializer_list<std::pair<ValueType, std::string>> &arguments) {
  NativeFunctions::GetRegistry()[name] = NativeFunction::Create(name, function, returnType, arguments);
}

NativeFunction NativeFunction::Create(const std::string &name,
                                      const NativeFunctionPtr &func) {
  return {
      .func = func,
      .name = name,
      .arguments = {},
      .returnType = ValueType::Undefined,
  };
}
NativeFunction NativeFunction::Create(const std::string &name, const NativeFunctionPtr &func,
                       const ValueType returnType,
                       const std::vector<std::pair<ValueType, std::string>> &arguments) {
  return {func, name, arguments, returnType};
}

std::string NativeFunction::GetInfo() {
  std::stringstream ss = {};
  ss << "func " << name << "(";
  int i = 0;
  for (const auto &[type, name] : arguments) {
    ss << TypeToString(type) << " " << name;
    if (i != arguments.size()) {
      ss << ", ";
    }
    i++;
  }
  ss << ")";
  return ss.str();
}
