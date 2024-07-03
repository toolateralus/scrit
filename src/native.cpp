#include "native.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "value.hpp"
#include <cstdlib>

#include "type.hpp"
#include "value.hpp"
#include <stdexcept>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

std::unordered_map<std::string, NativeCallable>
    FunctionRegistry::cachedCallables = {};

Object ScritModDefAsObject(ScritModDef *mod) {
  return Ctx::CreateObject(mod->scopes->front());
}

void m_InstantiateCallables(ScritModDef *module) {
  for (const auto &[key, func] : *module->functions) {
    module->scopes->front()->Set(key, FunctionRegistry::MakeCallable(func),
                    Mutability::Const);
  }
}

NativeCallable
FunctionRegistry::MakeCallable(const shared_ptr<NativeFunction> &fn) {
  return make_shared<NativeCallable_T>(fn);
}

NativeCallable FunctionRegistry::GetCallable(const std::string &name) {
  auto fIt = cachedCallables.find(name);
  if (fIt != cachedCallables.end() && fIt->second != nullptr) {
    return fIt->second;
  }

  auto registry = GetRegistry();
  auto it = registry.find(name);
  if (it != registry.end()) {
    auto func = it->second;
    auto callable = make_shared<NativeCallable_T>(func);
    cachedCallables[name] = callable;
    return callable;
  }
  return nullptr;
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

ScritModDef *LoadScritModule(const std::string &name, const std::string &path,
                             void *&out_handle) {
#ifdef __linux__
  out_handle = dlopen(path.c_str(), RTLD_NOW);
  if (!out_handle) {
    throw std::runtime_error(dlerror());
  }
  
  auto replaced = std::string(name);
  replaced = replaceAll(replaced, "::", "_SR_");
  
  auto fnName = "InitScritModule_" + replaced;
  void *func = dlsym(out_handle, fnName.c_str());
  if (!func) {
    throw std::runtime_error(dlerror());
  }
  ScriptModInitFuncPtr function = (ScriptModInitFuncPtr)func;

  if (!function) {
    dlclose(out_handle);
    throw std::runtime_error(
        "Invalid function signature on " + fnName +
        ". This function must return a ScritModDef* and take no arguments.");
  }

  auto mod = function();
  
  return mod;
#else
  HMODULE handle = LoadLibraryA(path.c_str());
  out_handle = (void *)handle if (!handle) {
    throw std::runtime_error("Failed to load module: " + path);
  }

  auto fnName = "InitScritModule_" + name;
  FARPROC func = GetProcAddress(handle, fnName.c_str());
  if (!func) {
    FreeLibrary(handle);
    throw std::runtime_error("Failed to find function: " + fnName);
  }
  ScriptModInitFuncPtr function = reinterpret_cast<ScriptModInitFuncPtr>(func);

  if (!function) {
    FreeLibrary(handle);
    throw std::runtime_error(
        "Invalid function signature on " + fnName +
        ". This function must return a ScritModDef* and take no arguments.");
  }

  auto mod = function();
  return mod;
#endif
}
ScritModDef *CreateModDef() {
  ScritModDef *mod = (ScritModDef *)malloc(sizeof(ScritModDef));
  mod->scopes = new std::vector<shared_ptr<Scope_T>> {make_shared<Scope_T>()};
  mod->description = new string();
  mod->functions =
      new std::unordered_map<std::string, shared_ptr<NativeFunction>>();
  mod->types = new std::unordered_map<std::string, Type>();
  mod->_namespace = nullptr;
  return mod;
}
std::unordered_map<std::string, shared_ptr<NativeFunction>> &
FunctionRegistry::GetRegistry() {
  static std::unordered_map<std::string, shared_ptr<NativeFunction>> reg;
  return reg;
}
bool FunctionRegistry::Exists(const std::string &name) {
  return GetRegistry().contains(name);
}

void RegisterFunction(const std::string &name,
                      const shared_ptr<NativeFunction> &function) {
  FunctionRegistry::GetRegistry()[name] = function;
}
void ScritModDef::AddFunction(const std::string &name,
                              const shared_ptr<NativeFunction> func) {
  (*functions)[name] = func;
}
void ScritModDef::AddVariable(const std::string &name, Value value,
                              const Mutability &mutability) {
  scopes->back()->Set(name, value, mutability);
}
ScritModDef::~ScritModDef() {
  delete description;
  delete scopes;
  delete types;
  delete functions;
}
void ScritModDef::AddType(const std::string &name, const Type type) {
  (*types)[name] = type;
}
void ScritModDef::SetNamespace(const string &name) {
  if (_namespace != nullptr) {
    throw std::runtime_error(
        "error in module: namespace cannot be set more than once");
  }
  _namespace = new std::string(name);
}
