#pragma once
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>

struct Value_T;
struct Object_T;
struct NativeCallable_T;
struct Context;

typedef std::shared_ptr<NativeCallable_T> NativeCallable;
typedef std::shared_ptr<Value_T> Value;
typedef std::shared_ptr<Object_T> Object;
typedef Value (*NativeFunctionPtr)(std::vector<Value>);

struct NativeFunction {
  NativeFunctionPtr func;
  std::string name;
  std::map<std::string, std::string> arguments;
  std::string returnType;
  
  static NativeFunction Create(const std::string &name, const NativeFunctionPtr &func) {
    return {
      .func = func,
      .name = name,
      .arguments = {},
      .returnType = "undefined",
    };
  }
  
  static NativeFunction Create(const std::string &name, const std::string &returnType, const NativeFunctionPtr &func, std::map<std::string,std::string> arguments = {}) {
    return {
      func,
      name,
      arguments,
      returnType
    };
  }
};

extern "C" struct ScritModDef {
  std::string *description;
  Context *context;
  std::vector<NativeFunction> *functions;
};

ScritModDef* CreateModDef();

extern "C" void AddFunction(ScritModDef *mod, NativeFunction function);
extern "C" void AddVariable(ScritModDef *mod, const std::string &name, Value value);

Object ScritModDefAsObject(ScritModDef *mod);
void m_InstantiateCallables(ScritModDef *mod);

typedef ScritModDef* (*ScriptModInitFuncPtr)();

struct NativeFunctions {
  static std::unordered_map<std::string, NativeCallable> instantiatedCallables;
  static std::unordered_map<std::string, NativeFunction> &GetRegistry();
  static bool Exists(const std::string &name);
  static NativeCallable GetCallable(const std::string &name);
  static NativeCallable MakeCallable(const NativeFunction &fn);
};

void RegisterFunction(const std::string &name, const NativeFunctionPtr &function, const std::string &returnType, const std::map<std::string, std::string> &arguments);

static std::pair<std::string, std::string> Argument(std::string &&type, std::string &&name) {
  return std::make_pair(type, name);
}

static std::map<std::string, std::string> CreateArgumentSignature(std::initializer_list<std::pair<std::string, std::string>> args) {
  std::map<std::string, std::string> signature;
  for (const auto& arg : args) {
    signature[arg.first] = arg.second;
  }
  return signature;
}

#define REGISTER_FUNCTION(name, returnType, arguments) \
  Value name(std::vector<Value>); \
  namespace { \
    struct name##_Register { \
      name##_Register() { \
        RegisterFunction(#name, name, returnType, arguments); \
      } \
    } name##_register; \
  } \
  Value name(std::vector<Value> args)

ScritModDef* LoadScritModule(const std::string &name, const std::string &path);