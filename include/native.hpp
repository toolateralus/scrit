#pragma once
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>

namespace Values {
  struct Value_T;
  struct Object_T;
  struct NativeCallable_T;
  enum struct ValueType;
  typedef std::shared_ptr<NativeCallable_T> NativeCallable;
  typedef std::shared_ptr<Value_T> Value;
  typedef std::shared_ptr<Object_T> Object;
}
struct Context;
using namespace Values;

typedef Value (*NativeFunctionPtr)(std::vector<Value>);

extern "C" struct ScritModDef {
  std::string *description;
  Context *context;
  std::unordered_map<std::string, NativeFunctionPtr> *functions;
  void AddFunction(const std::string &name, const NativeFunctionPtr func);
  void AddVariable(const std::string &name, Value value);
};


ScritModDef* CreateModDef();
Object ScritModDefAsObject(ScritModDef *mod);
void m_InstantiateCallables(ScritModDef *mod);

typedef ScritModDef* (*ScriptModInitFuncPtr)();

struct NativeFunctions {
  static std::unordered_map<std::string, NativeCallable> cachedCallables;
  static std::unordered_map<std::string, NativeFunctionPtr> &GetRegistry();
  static bool Exists(const std::string &name);
  static NativeCallable GetCallable(const std::string &name);
  static NativeCallable MakeCallable(const NativeFunctionPtr &fn);
};

void RegisterFunction(const std::string &name, const NativeFunctionPtr &function);

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
  
  

ScritModDef* LoadScritModule(const std::string &name, const std::string &path);