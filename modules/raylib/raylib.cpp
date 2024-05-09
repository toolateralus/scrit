
#include "raylib.h"
#include <iostream>
#include <ostream>
#include <scrit/context.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/value.hpp>
#include <vector>

static bool TryGetColor(Object value, Color &color) {
  if (value == nullptr || value->scope == nullptr) {
    return false;
  }

  auto scope = value->scope;

  auto rval = value->GetMember("r");
  auto gval = value->GetMember("g");
  auto bval = value->GetMember("b");
  auto aval = value->GetMember("a");

  if (Ctx::IsUndefined(rval, gval, bval, aval) ||
      Ctx::IsNull(rval, gval, bval, aval)) {
    return false;
  }

  int r, g, b, a;
  if (!Ctx::TryGetInt(rval, r) || !Ctx::TryGetInt(gval, g) ||
      !Ctx::TryGetInt(bval, b) || !Ctx::TryGetInt(aval, a)) {
    return false;
  }

  color = Color{(unsigned char)r, (unsigned char)g, (unsigned char)b,
                (unsigned char)a};

  return true;
}
static Value initWindow(std::vector<Value> args) {
  if (args.size() != 3) {
    return Ctx::CreateString("Invalid arguments");
  }
  int height, width;
  string name;
  if (!Ctx::TryGetInt(args[0], width) || !Ctx::TryGetInt(args[1], height) ||
      !Ctx::TryGetString(args[2], name)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  InitWindow(width, height, name.c_str());
  return Value_T::Undefined;
}
static Value beginDrawing(std::vector<Value> args) {
  BeginDrawing();
  return Value_T::Undefined;
}
static Value endDrawing(std::vector<Value> args) {
  EndDrawing();
  return Value_T::Undefined;
}
static Value drawRectangle(std::vector<Value> args) {
  int posX, posY, width, height;
  Object obj;

  if (args.size() != 5) {
    return Ctx::CreateString("invalid args; expected 5, got " +
                             std::to_string(args.size()));
  }

  if (!Ctx::TryGetInt(args[0], posX) || !Ctx::TryGetInt(args[1], posY) ||
      !Ctx::TryGetInt(args[2], width) || !Ctx::TryGetInt(args[3], height) ||
      !Ctx::TryGetObject(args[4], obj)) {
    return Ctx::CreateString("Invalid argument types.");
  }

  Color color = WHITE;

  if (obj == nullptr) {
    return Ctx::CreateString("failed to get color");
  }

  if (!TryGetColor(obj, color)) {
    return Ctx::CreateString("Invalid color object");
  }

  DrawRectangle(posX, posY, width, height, color);

  return Value_T::Undefined;
}
static Value windowShouldClose(std::vector<Value> args) {
  return Ctx::CreateBool(WindowShouldClose());
}
static Value clearBackground(std::vector<Value> args) {
  Object o;
  Color color = BLACK;
  if (!args.empty() && Ctx::TryGetObject(args[0], o) && TryGetColor(o, color)) {
    ClearBackground(color);
  } else {
    ClearBackground(BLACK);
  }
  return Value_T::Undefined;
}

extern "C" ScritModDef *InitScritModule_raylib() {
  ScritModDef *def = CreateModDef();
  *def->description = "raylib bindings for scrit scripting lang";
  ScritMod_AddFunction(def, "initWindow", &initWindow);
  ScritMod_AddFunction(def, "beginDrawing", &beginDrawing);
  ScritMod_AddFunction(def, "endDrawing", &endDrawing);
  ScritMod_AddFunction(def, "drawRectangle", &drawRectangle);
  ScritMod_AddFunction(def, "windowShouldClose", &windowShouldClose);
  return def;
}