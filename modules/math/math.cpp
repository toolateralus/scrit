
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
  return Value_T::Undefined;
}
static Value toFloat(std::vector<Value> args) {
  int result;
  float fresult;
  if (args.empty()) {
    return Value_T::Undefined;
  }
  if (Ctx::TryGetInt(args[0], result)) {
    return Ctx::CreateFloat(result);
  }
  if (Ctx::TryGetFloat(args[0], fresult)) {
    return args[0];
  }
  return Value_T::Undefined;
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
  return Value_T::Undefined;
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
  return Value_T::Undefined;
}


extern "C" ScritModDef *InitScritModule_math() {
  ScritModDef *def = CreateModDef();
  *def->description = "basic math module.";
  ScritMod_AddFunction(def, "toInt", &toInt);
  ScritMod_AddFunction(def, "floor", &floor);
  ScritMod_AddFunction(def, "round", &round);
  ScritMod_AddFunction(def, "sqrt", &sqrt);
  ScritMod_AddFunction(def, "toFloat", &toFloat);
  return def;
}