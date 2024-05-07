#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

struct Value_T;
struct Object_T;
struct NativeCallable_T;
struct Context;

typedef std::shared_ptr<Value_T> NativeCallable;
typedef std::shared_ptr<Value_T> Value;
typedef std::shared_ptr<Object_T> Object;
typedef Value (*NativeFunctionPtr)(std::vector<Value>);

struct ScritModDef {
  std::string description;
  ScritModDef();
  ~ScritModDef();
  Context *context;
  std::unordered_map<std::string, NativeFunctionPtr> functions;

  void AddFunction(const std::string &name, NativeFunctionPtr function);
  void AddVariable(const std::string &name, Value value);
  Object AsObject();

private:
  void m_InstantiateCallables();
};
typedef ScritModDef (*ScritModInitFunc)();

struct NativeFunctions {
  static std::unordered_map<std::string, NativeCallable> instantiatedCallables;
  static std::unordered_map<std::string, NativeFunctionPtr> &GetRegistry();
  static bool Exists(const std::string &name);
  static NativeCallable GetCallable(const std::string &name);
  static NativeCallable MakeCallable(const NativeFunctionPtr fn);
};

void RegisterFunction(const std::string &name, NativeFunctionPtr function);

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

void LoadScritModule(const std::string &name, const std::string &path);