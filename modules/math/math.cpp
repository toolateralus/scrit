
#include <scrit/native.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/value.hpp>
#include <cmath>

static Value toInt(std::vector<Value> args) {
  float fres;
  bool bres;
  int ires;
  if (!args.empty()) {
    if (Ctx::TryGetFloat(args[0], fres)) {
      return Ctx::CreateInt(fres);
    }
    if (Ctx::TryGetBool(args[0], bres)) {
      return Ctx::CreateInt(bres);
    }
    if (Ctx::TryGetInt(args[0], ires)) {
      return args[0];
    }
  }
  return Value_T::UNDEFINED;
}
static Value toFloat(std::vector<Value> args) {
  int result;
  float fresult;
  if (args.empty()) {
    return Value_T::UNDEFINED;
  }
  if (Ctx::TryGetInt(args[0], result)) {
    return Ctx::CreateFloat(result);
  }
  if (Ctx::TryGetFloat(args[0], fresult)) {
    return args[0];
  }
  return Value_T::UNDEFINED;
}
static Value floor(std::vector<Value> args) {
  return toInt(args);
}
static Value round(std::vector<Value> args) {
  float fres;
  bool bres;
  int ires;
  if (!args.empty()) {
    if (Ctx::TryGetFloat(args[0], fres)) {
      return Ctx::CreateInt(round(fres));
    }
    if (Ctx::TryGetInt(args[0], ires)) {
      return Ctx::CreateFloat(round(ires));
    }
  }
  return Value_T::UNDEFINED;
}
static Value sqrt(std::vector<Value> args) {
  float fres;
  bool bres;
  int ires;
  if (!args.empty()) {
    if (Ctx::TryGetFloat(args[0], fres)) {
      return Ctx::CreateFloat(sqrt(fres));
    }
    if (Ctx::TryGetInt(args[0], ires)) {
      return Ctx::CreateFloat(sqrt(ires));
    }
  }
  return Value_T::UNDEFINED;
}


extern "C" ScritModDef *InitScritModule_math() {
  ScritModDef *def = CreateModDef();
  *def->description = "basic math module.";
  AddFunction(def, "toInt", toInt, ValueType::Int, {Argument(ValueType::Any, "value")});
  AddFunction(def, "floor", floor, ValueType::Int, {Argument(ValueType::Any, "value")});
  AddFunction(def, "round", round, ValueType::Int, {Argument(ValueType::Any, "value")});
  AddFunction(def, "sqrt", sqrt, ValueType::Float, {Argument(ValueType::Any, "value")});
  AddFunction(def, "toFloat", toFloat, ValueType::Float, {Argument(ValueType::Any, "value")});
  return def;
}