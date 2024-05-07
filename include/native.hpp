#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

struct Value_T;
struct Object_T;
struct NativeCallable_T;
typedef std::shared_ptr<Value_T> NativeCallable;
typedef std::shared_ptr<Value_T> Value;
typedef std::shared_ptr<Object_T> Object;
typedef Value (*NativeFunctionPtr)(std::vector<Value>);
struct Context;


extern "C" struct ScritModDef {
  char * description;
  Context *context;
  char **funcNames;
  NativeFunctionPtr *funcs;
  size_t fnCount;
  size_t fnMax;
};

ScritModDef* CreateModDef();

extern "C" void ScritMod_AddFunction(ScritModDef *mod, const std::string &name, NativeFunctionPtr function);
extern "C" void ScritMod_AddVariable(ScritModDef *mod, const std::string &name, Value value);
Object ScritModDefAsObject(ScritModDef *mod);
void m_InstantiateCallables(ScritModDef *mod);

typedef ScritModDef* (*ScriptModInitFuncPtr)();

struct NativeFunctions {
  static std::unordered_map<std::string, NativeCallable> instantiatedCallables;
  static std::unordered_map<std::string, NativeFunctionPtr>& GetRegistry() {
    static std::unordered_map<std::string, NativeFunctionPtr> reg;
    return reg;
  }
  static bool Exists(const std::string &name) {
    return GetRegistry().contains(name);
  }
  static NativeCallable GetCallable(const std::string &name);
  static NativeCallable MakeCallable(const NativeFunctionPtr fn);
};

static void RegisterFunction(const std::string& name, NativeFunctionPtr function) {
  NativeFunctions::GetRegistry()[name] = function;
}

#define REGISTER_FUNCTION(name) \
  Value name(std::vector<Value> args); \
  namespace { \
    struct name##_Register { \
      name##_Register() { \
        RegisterFunction(#name, name); \
      } \
    } name##_register; \
  } \
  Value name(std::vector<Value> args)
  


ScritModDef* LoadScritModule(const std::string &name, const std::string& path);

