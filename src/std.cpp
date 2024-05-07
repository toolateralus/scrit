#include "native.hpp"
#include "value.hpp"

REGISTER_FUNCTION(println) {
  for (const auto arg: args) {
    printf("%s\n", arg->ToString().c_str());;
  }
  return Value_T::Undefined;
}

REGISTER_FUNCTION(push) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  if (args[0]->type != ValueType::Array) {
    return Value_T::Undefined;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  for (int i = 1; i < args.size(); i++) {
    array->Push(args[i]);
  }
  return Value_T::Undefined;
}

REGISTER_FUNCTION(pop) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  if (args[0]->type != ValueType::Array) {
    return Value_T::Undefined;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  return array->Pop();
}

REGISTER_FUNCTION(len) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  if (args[0]->type != ValueType::Array) {
    return Value_T::Undefined;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  return make_shared<Int_T>(array->values.size());
}
