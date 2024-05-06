#pragma once

#include "value.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>

// This is the function type that will be used in your interpreter
typedef Value (*InterpreterFunction)(std::vector<Value>);


struct NativeFunctions {
  static std::unordered_map<std::string, InterpreterFunction>& GetRegistry() {
    static std::unordered_map<std::string, InterpreterFunction> reg;
    return reg;
  }
};


// This is the registry that will hold all the functions

// This is a function that will register a new function in the registry
static void registerFunction(const std::string& name, InterpreterFunction function) {
  std::cout << "registered " << name << "\n";
  NativeFunctions::GetRegistry()[name] = function;
}

// This is a macro that will automatically register a function when it's defined
#define REGISTER_FUNCTION(name) \
  Value name(std::vector<Value> args); \
  namespace { \
    struct name##_Register { \
      name##_Register() { \
        registerFunction(#name, name); \
      } \
    } name##_register; \
  } \
  Value name(std::vector<Value> args)

