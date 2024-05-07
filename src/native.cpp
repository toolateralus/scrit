#include "native.hpp"
#include "context.hpp"
#include "value.hpp"
#include <cstdlib>
#include <dlfcn.h>
#include <memory>
#include <stdexcept>
#include <unordered_map>

std::unordered_map<std::string, NativeCallable>
    NativeFunctions::instantiatedCallables = {};


extern "C" void ScritMod_AddFunction(ScritModDef *mod, const std::string &name,
                NativeFunctionPtr function) {
                
  if (mod->fnCount == mod->fnMax) {
  mod->funcs = (NativeFunctionPtr*)realloc(mod->funcs, sizeof(NativeFunctionPtr) * (mod->fnMax + 1));
  mod->funcNames = (char**)realloc(mod->funcNames, sizeof(char*) * (mod->fnMax + 1));
  }
  auto &count = mod->fnCount;
  mod->funcs[count] = function;
  mod->funcNames[count] = (char*)name.c_str();
  count++;
}

extern "C" void ScritMod_AddVariable(ScritModDef *mod, const std::string &name, Value value) {
  mod->context->Insert(name, value);
}

Object ScritModDefAsObject(ScritModDef *mod) {
  m_InstantiateCallables(mod);
  auto object = std::make_shared<Object_T>();
  object->scope = mod->context->scopes[0];
  return object;
}

void m_InstantiateCallables(ScritModDef *module) {
  auto context = module->context;
  auto funcs = module->funcs;
  auto funcCount = module->fnCount;
  
  for (size_t i = 0; i < funcCount; ++i) {
    auto name = module->funcNames[i];
    auto func = funcs[i];
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
  dlclose(handle);
  return mod;
}
ScritModDef* CreateModDef() { 
  ScritModDef *mod = (ScritModDef*)malloc(sizeof(ScritModDef));
  mod->context = new Context(); 
  mod->description = (char*)malloc(sizeof(char*));
  mod->fnCount = 0;
  mod->funcs = nullptr;
  mod->funcNames = nullptr;
  mod->fnMax = 10;
  mod->funcs = (NativeFunctionPtr*)malloc(sizeof(NativeFunctionPtr) * 10);
  mod->funcNames = (char**)malloc(sizeof(char*) * 10);
  
  
  return mod;
}
