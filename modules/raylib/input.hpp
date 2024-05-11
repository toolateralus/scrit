#pragma once

#include <map>
#include <raylib.h>
#include <string>
#include "scrit/scritmod.hpp"

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
static Value isMouseButtonDown(std::vector<Value> args) {
  if (args.empty()) {
    return Value_T::False;
  }
  int button;
  if (!Ctx::TryGetInt(args[0], button)) {
    return Value_T::False;
  }
  return Ctx::CreateBool(IsMouseButtonDown(button));
}
static Value getMousePosition(std::vector<Value> args) {
  Vector2 position = GetMousePosition();
  Object result = Ctx::CreateObject();
  result->scope->variables["x"] = Ctx::CreateFloat(position.x);
  result->scope->variables["y"] = Ctx::CreateFloat(position.y);
  return result;
}
static Value setMousePosition(std::vector<Value> args) {
  if (args.size() != 2) {
    return Ctx::CreateString("Invalid arguments");
  }
  float x, y;
  if (!Ctx::TryGetFloat(args[0], x) || !Ctx::TryGetFloat(args[1], y)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  SetMousePosition((int)x, (int)y);
  return Value_T::Undefined;
}
static Value getMouseWheelMove(std::vector<Value> args) {
  return Ctx::CreateInt(GetMouseWheelMove());
}