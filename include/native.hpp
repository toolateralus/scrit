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
enum struct ValueType;

typedef std::shared_ptr<NativeCallable_T> NativeCallable;
typedef std::shared_ptr<Value_T> Value;
typedef std::shared_ptr<Object_T> Object;
typedef Value (*NativeFunctionPtr)(std::vector<Value>);

struct NativeFunction {
  NativeFunctionPtr func;
  std::string name;
  std::map<ValueType, std::string> arguments;
  ValueType returnType;

  static NativeFunction Create(const std::string &name,
                               const NativeFunctionPtr &func);
  
  static NativeFunction Create(const std::string &name,
                               const NativeFunctionPtr &func,
                               const ValueType returnType,
                               const std::map<ValueType, std::string> &arguments = {});
};

extern "C" struct ScritModDef {
  std::string *description;
  Context *context;
  std::vector<NativeFunction> *functions;
};

ScritModDef* CreateModDef();

extern "C" void AddFunction(ScritModDef *mod, const std::string &name,
                 const NativeFunctionPtr &ptr, const ValueType retType, std::map<ValueType, std::string> args = {});
                
extern "C" void AddFunctionNoInfo(ScritModDef *mod, const std::string &name,
          const NativeFunctionPtr &ptr);
          
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

void RegisterFunction(const std::string &name, const NativeFunctionPtr &function, const ValueType returnType, const std::map<ValueType, std::string> &arguments);

static std::pair<ValueType, std::string> Argument(ValueType &&type, std::string &&name) {
  return std::make_pair(type, name);
}

static std::map<ValueType, std::string> CreateArgumentSignature(std::initializer_list<std::pair<ValueType, std::string>> args) {
  std::map<ValueType, std::string> signature;
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