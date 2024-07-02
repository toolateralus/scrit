#pragma once

#include "ast.hpp"
#include "type.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Values {
struct Value_T;
struct Object_T;
struct NativeCallable_T;
enum struct PrimitiveType;
typedef std::shared_ptr<NativeCallable_T> NativeCallable;
typedef std::shared_ptr<Value_T> Value;
typedef std::shared_ptr<Object_T> Object;
}; // namespace Values
using namespace Values;

struct Context;
struct ScritModDef;

typedef ScritModDef *(*ScriptModInitFuncPtr)();
typedef Value (*NativeFunctionPtr)(std::vector<Value>);

enum struct Mutability;

struct NativeFunction {
  NativeFunction(const string &name, const Type returnType,
                 const vector<Type> &parameterTypes, NativeFunctionPtr ptr)
      : name(std::move(name)), returnType(returnType),
        parameterTypes(parameterTypes), ptr(ptr) {}
  string name;
  Type returnType;
  vector<Type> parameterTypes;
  NativeFunctionPtr ptr;
  static shared_ptr<NativeFunction> Create(const string &name,
                                           const Type returnType,
                                           const vector<Type> &parameterTypes,
                                           const NativeFunctionPtr ptr) {
    return make_shared<NativeFunction>(name, returnType, parameterTypes, ptr);
  }
};

extern "C" struct ScritModDef {
  std::string *description;
  Context *context;

  std::unordered_map<std::string, shared_ptr<NativeFunction>> *functions;
  std::unordered_map<std::string, Type> *types;

  void AddFunction(const std::string &name,
                   const shared_ptr<NativeFunction> func);
  void AddVariable(const std::string &name, Value value, const Mutability &mut);
  void AddType(const std::string &name, const Type type);

  ~ScritModDef();
};

ScritModDef *CreateModDef();
Object ScritModDefAsObject(ScritModDef *mod);
void m_InstantiateCallables(ScritModDef *mod);
ScritModDef *LoadScritModule(const std::string &name, const std::string &path,
                             void *&handle);

void RegisterFunction(const std::string &name,
                      const shared_ptr<NativeFunction> &function);

struct FunctionRegistry {
  static std::unordered_map<std::string, NativeCallable> cachedCallables;
  static std::unordered_map<std::string, shared_ptr<NativeFunction>> &
  GetRegistry();
  static bool Exists(const std::string &name);
  static NativeCallable GetCallable(const std::string &name);
  static NativeCallable MakeCallable(const shared_ptr<NativeFunction> &fn);
};

#pragma GCC diagnostic push

#define CREATE_FUNCTION(name, returnType, parameterTypes...)                   \
  NativeFunction::Create(#name, TypeSystem::Current().Find(returnType),        \
                         TypeSystem::Current().GetVector(parameterTypes),      \
                         name)

#define CREATE_CALLABLE(name, returnType, parameterTypes...)                   \
  FunctionRegistry::MakeCallable(                                              \
      CREATE_FUNCTION(name, returnType, parameterTypes))

#define REGISTER_FUNCTION(name, returnType, parameterTypes...)                 \
  Value name(std::vector<Value> args);                                         \
  namespace {                                                                  \
  struct name##_Register {                                                     \
    name##_Register() {                                                        \
      auto _returnType = TypeSystem::Current().Find(returnType);               \
      auto _parameterTypes = TypeSystem::Current().GetVector(parameterTypes);  \
      auto func =                                                              \
          NativeFunction::Create(#name, _returnType, _parameterTypes, name);   \
      RegisterFunction(#name, func);                                           \
    }                                                                          \
  } name##_register;                                                           \
  }                                                                            \
  Value name(std::vector<Value> args)

#pragma GCC diagnostic pop