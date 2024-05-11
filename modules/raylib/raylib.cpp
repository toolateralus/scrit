
#include "raylib.h"
#include <memory>
#include <scrit/context.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/value.hpp>
#include <vector>


static std::map<std::string, int> GetKeysMap() {
  std::map<std::string, int> keysMap;
  keysMap["NULL"] = 0;
  keysMap["APOSTROPHE"] = 39;
  keysMap["COMMA"] = 44;
  keysMap["MINUS"] = 45;
  keysMap["PERIOD"] = 46;
  keysMap["SLASH"] = 47;
  keysMap["ZERO"] = 48;
  keysMap["ONE"] = 49;
  keysMap["TWO"] = 50;
  keysMap["THREE"] = 51;
  keysMap["FOUR"] = 52;
  keysMap["FIVE"] = 53;
  keysMap["SIX"] = 54;
  keysMap["SEVEN"] = 55;
  keysMap["EIGHT"] = 56;
  keysMap["NINE"] = 57;
  keysMap["SEMICOLON"] = 59;
  keysMap["EQUAL"] = 61;
  keysMap["A"] = 65;
  keysMap["B"] = 66;
  keysMap["C"] = 67;
  keysMap["D"] = 68;
  keysMap["E"] = 69;
  keysMap["F"] = 70;
  keysMap["G"] = 71;
  keysMap["H"] = 72;
  keysMap["I"] = 73;
  keysMap["J"] = 74;
  keysMap["K"] = 75;
  keysMap["L"] = 76;
  keysMap["M"] = 77;
  keysMap["N"] = 78;
  keysMap["O"] = 79;
  keysMap["P"] = 80;
  keysMap["Q"] = 81;
  keysMap["R"] = 82;
  keysMap["S"] = 83;
  keysMap["T"] = 84;
  keysMap["U"] = 85;
  keysMap["V"] = 86;
  keysMap["W"] = 87;
  keysMap["X"] = 88;
  keysMap["Y"] = 89;
  keysMap["Z"] = 90;
  keysMap["LEFT_BRACKET"] = 91;
  keysMap["BACKSLASH"] = 92;
  keysMap["RIGHT_BRACKET"] = 93;
  keysMap["GRAVE"] = 96;
  keysMap["SPACE"] = 32;
  keysMap["ESCAPE"] = 256;
  keysMap["ENTER"] = 257;
  keysMap["TAB"] = 258;
  keysMap["BACKSPACE"] = 259;
  keysMap["INSERT"] = 260;
  keysMap["DELETE"] = 261;
  keysMap["RIGHT"] = 262;
  keysMap["LEFT"] = 263;
  keysMap["DOWN"] = 264;
  keysMap["UP"] = 265;
  keysMap["PAGE_UP"] = 266;
  keysMap["PAGE_DOWN"] = 267;
  keysMap["HOME"] = 268;
  keysMap["END"] = 269;
  keysMap["CAPS_LOCK"] = 280;
  keysMap["SCROLL_LOCK"] = 281;
  keysMap["NUM_LOCK"] = 282;
  keysMap["PRINT_SCREEN"] = 283;
  keysMap["PAUSE"] = 284;
  keysMap["F1"] = 290;
  keysMap["F2"] = 291;
  keysMap["F3"] = 292;
  keysMap["F4"] = 293;
  keysMap["F5"] = 294;
  keysMap["F6"] = 295;
  keysMap["F7"] = 296;
  keysMap["F8"] = 297;
  keysMap["F9"] = 298;
  keysMap["F10"] = 299;
  keysMap["F11"] = 300;
  keysMap["F12"] = 301;
  keysMap["LEFT_SHIFT"] = 340;
  keysMap["LEFT_CONTROL"] = 341;
  keysMap["LEFT_ALT"] = 342;
  keysMap["LEFT_SUPER"] = 343;
  keysMap["RIGHT_SHIFT"] = 344;
  keysMap["RIGHT_CONTROL"] = 345;
  keysMap["RIGHT_ALT"] = 346;
  keysMap["RIGHT_SUPER"] = 347;
  keysMap["KB_MENU"] = 348;
  keysMap["KP_0"] = 320;
  keysMap["KP_1"] = 321;
  keysMap["KP_2"] = 322;
  keysMap["KP_3"] = 323;
  keysMap["KP_4"] = 324;
  keysMap["KP_5"] = 325;
  keysMap["KP_6"] = 326;
  keysMap["KP_7"] = 327;
  keysMap["KP_8"] = 328;
  keysMap["KP_9"] = 329;
  keysMap["KP_DECIMAL"] = 330;
  keysMap["KP_DIVIDE"] = 331;
  keysMap["KP_MULTIPLY"] = 332;
  keysMap["KP_SUBTRACT"] = 333;
  keysMap["KP_ADD"] = 334;
  keysMap["KP_ENTER"] = 335;
  keysMap["KP_EQUAL"] = 336;
  keysMap["BACK"] = 4;
  keysMap["MENU"] = 5;
  keysMap["VOLUME_UP"] = 24;
  keysMap["VOLUME_DOWN"] = 25;
  return keysMap;
}

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
    auto arr = Ctx::CreateArray(args);
    return Ctx::CreateString("Invalid argument types." + arr->ToString());
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
static Value isKeyDown(std::vector<Value> args) {
  if (args.empty()) {
    return Value_T::False;
  }
  int key;
  if (!Ctx::TryGetInt(args[0], key)) {
    return Value_T::False;
  }
  return Ctx::CreateBool(IsKeyDown(key));
  
}

extern "C" ScritModDef *InitScritModule_raylib() {
  ScritModDef *def = CreateModDef();
  *def->description = "raylib bindings for scrit scripting lang";
  ScritMod_AddFunction(def, "initWindow", &initWindow);
  ScritMod_AddFunction(def, "beginDrawing", &beginDrawing);
  ScritMod_AddFunction(def, "endDrawing", &endDrawing);
  ScritMod_AddFunction(def, "drawRectangle", &drawRectangle);
  ScritMod_AddFunction(def, "windowShouldClose", &windowShouldClose);
  ScritMod_AddFunction(def, "clearBackground", &clearBackground);
  ScritMod_AddFunction(def, "isKeyDown", &isKeyDown);
  
  Object keys = Ctx::CreateObject();
  for (const auto &[key, val] : GetKeysMap()) {
    auto value = std::dynamic_pointer_cast<Value_T>(Ctx::CreateInt(val));
    keys->SetMember(key, value);
  }
  
  ScritMod_AddVariable(def, "keys", keys);
  
  return def;
}
