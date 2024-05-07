#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

struct Value_T;
typedef std::shared_ptr<Value_T> Value;

typedef Value (*NativeFunction)(std::vector<Value>);

struct NativeFunctions {
  static std::unordered_map<std::string, NativeFunction>& GetRegistry() {
    static std::unordered_map<std::string, NativeFunction> reg;
    return reg;
  }
};

static void RegisterFunction(const std::string& name, NativeFunction function) {
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
  
  
  