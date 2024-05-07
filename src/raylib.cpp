#include "raylib.h"
#include "native.hpp"
#include "value.hpp"


REGISTER_FUNCTION(InitWindow) {
  
  if (args.size() != 3) {
    return Value_T::Undefined;
  }
  
  auto wV = args[0];
  auto hV = args[1];
  auto tV = args[2];
  
  int w, h;
  if (wV->type == ValueType::Int) {
    w = static_cast<Int_T*>(wV.get())->value;
  } else {
    return Value_T::Undefined;
  }
  if (hV->type == ValueType::Int) {
    h = static_cast<Int_T*>(hV.get())->value;
  } else {
    return Value_T::Undefined;
  }
  const char * str;
  if (tV->type == ValueType::String) {
    auto xv = static_cast<String_T*>(tV.get());
    str = xv->value.c_str();
  }
  InitWindow(w, h, str);
  return Value_T::Undefined;
}
