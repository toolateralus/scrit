#include "context.hpp"
#include "native.hpp"
#include "value.hpp"
#include <algorithm>
#include <stdexcept>
#include <vector>

#pragma clang diagnostic ignored "-Wunused-parameter"

#define undefined Ctx::Undefined()
#define null Ctx::Null()

REGISTER_FUNCTION(contains) {
  if (args.size() < 2) {
    return undefined;
  }
  
  Array result; 
  if (!Ctx::TryGetArray(args[0], result)) {
    throw std::runtime_error("contains may only be used on arrays. for strings, use scontains");
  }
  for (const auto &value: result->values) {
    if (value->Equals(args[1])) {
      return Value_T::True;
    }
  }
  return Value_T::False; 
}

REGISTER_FUNCTION(scontains) {
  if (args.size() < 2) {
    return undefined;
  }
  string result, comparison;
  if (!Ctx::TryGetString(args[0], result) || !Ctx::TryGetString(args[1], comparison)) {
    throw std::runtime_error("scontains may only be used on string. for arrays, use contains");
  }
  for (const auto &value: result) {
    if (comparison[0] == value)  
      return Value_T::True;
  }
  return Value_T::False; 
}


REGISTER_FUNCTION(replace) {
  if (args.size() < 3 || args[0]->GetType() != Values::ValueType::String ||
      args[1]->GetType() != Values::ValueType::String ||
      args[2]->GetType() != Values::ValueType::String) {
    return undefined;
  }
  
  string string; 
  Ctx::TryGetString(args[0], string);
  
  auto pattern = static_cast<String_T *>(args[1].get());
  auto replacement = static_cast<String_T *>(args[2].get());
  
  size_t pos = 0;
  while ((pos = string.find(pattern->value, pos)) != std::string::npos) {
    string.replace(pos, pattern->value.length(), replacement->value);
    pos += replacement->value.length();
  }
  
  return Ctx::CreateString(string);
}

REGISTER_FUNCTION(remove) {
  if (args.empty()) {
    return undefined;
  }

  // Erase array element.
  if (args[0]->GetType() == Values::ValueType::Array) {
    int i;
    Array_T *a = static_cast<Array_T *>(args[0].get());
    if (args[1]->GetType() == Values::ValueType::Callable) {
      // Predicate.
      auto callable = static_cast<Callable_T *>(args[1].get());
      const auto lambda = [callable](Value value) -> bool {
        std::vector<Value> args = {value};
        return callable->Call(args)->Equals(Value_T::True);
      };
      auto new_end = std::remove_if(a->values.begin(), a->values.end(), lambda);
      a->values.erase(new_end, a->values.end());
    } else if (Ctx::TryGetInt(args[1], i)) {
      // At index.
      a->values.erase(a->values.begin(), a->values.begin() + i);
    } else {
      std::runtime_error("Invalid argument passed to {remove} :: expected "
                         "predicate function, or index of element to remove");
    }

  } else if (args[0]->GetType() == Values::ValueType::String) {
    int i;
    String_T *str = static_cast<String_T *>(args[0].get());
    if (args[1]->GetType() == Values::ValueType::Callable) {
      // Predicate.
      auto callable = static_cast<Callable_T *>(args[1].get());
      const auto lambda = [callable](char value) -> bool {
        std::vector<Value> args = {Ctx::CreateString(string(1, value))};
        return callable->Call(args)->Equals(Value_T::True);
      };
      auto new_end =
          std::remove_if(str->value.begin(), str->value.end(), lambda);
      str->value.erase(new_end, str->value.end());
    } else if (Ctx::TryGetInt(args[1], i)) {
      // At index.
      str->value.erase(str->value.begin(), str->value.begin() + i);
    } else {
      std::runtime_error("Invalid argument passed to {remove} :: expected "
                         "predicate function, or index of element to remove");
    }
  }

  return undefined;
}

REGISTER_FUNCTION(where) {
  if (args.size() < 2 || args[0]->GetType() != Values::ValueType::Array ||
      args[1]->GetType() != Values::ValueType::Callable) {
    return undefined;
  }
  Array_T *a = static_cast<Array_T *>(args[0].get());
  Callable_T *c = static_cast<Callable_T *>(args[1].get());
  std::vector<Value> values;
  std::vector<Value> c_args;
  for (const auto &element : a->values) {
    c_args = {element};
    if (c->Call(c_args)->Equals(Value_T::True)) {
      values.push_back(element);
    }
  }
  return Ctx::CreateArray(values);
}
