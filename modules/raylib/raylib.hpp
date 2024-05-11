#pragma once
#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <scrit/context.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/value.hpp>
#include <string>
#include <vector>
#include <map>

using std::vector;

static vector<Model> loadedModels = {};

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

static Object CreateVector3(Vector3 &v) {
  Object obj = Ctx::CreateObject();
  obj->scope->variables["x"] = Ctx::CreateFloat(v.x);
  obj->scope->variables["y"] = Ctx::CreateFloat(v.y);
  obj->scope->variables["z"] = Ctx::CreateFloat(v.z);
  return obj;
}
static Object CreateQuaternion(Quaternion &quat) {
  Object obj = Ctx::CreateObject();
  obj->scope->variables["x"] = Ctx::CreateFloat(quat.x);
  obj->scope->variables["y"] = Ctx::CreateFloat(quat.y);
  obj->scope->variables["z"] = Ctx::CreateFloat(quat.z);
  obj->scope->variables["w"] = Ctx::CreateFloat(quat.w);
  return obj;
}
static Object CreateTransform(Transform &transform) {
  Object obj = Ctx::CreateObject();
  obj->scope->variables["position"] = CreateVector3(transform.translation);
  obj->scope->variables["rotation"] = CreateQuaternion(transform.rotation);
  obj->scope->variables["scale"] = CreateVector3(transform.scale);
  return obj;
}
static Object CreateMatrix(Matrix &transform) {
  Object obj = Ctx::CreateObject();
  
  // Extract translation
  Vector3 translation = {transform.m12, transform.m13, transform.m14};
  obj->scope->variables["position"] = CreateVector3(translation);

  // Extract scale
  Vector3 scale;
  scale.x = Vector3Length((Vector3){transform.m0, transform.m1, transform.m2});
  scale.y = Vector3Length((Vector3){transform.m4, transform.m5, transform.m6});
  scale.z = Vector3Length((Vector3){transform.m8, transform.m9, transform.m10});
  obj->scope->variables["scale"] = CreateVector3(scale);

  // Normalize the matrix to extract rotation
  Matrix rotationMatrix = transform;
  rotationMatrix.m0 /= scale.x;
  rotationMatrix.m1 /= scale.x;
  rotationMatrix.m2 /= scale.x;
  rotationMatrix.m4 /= scale.y;
  rotationMatrix.m5 /= scale.y;
  rotationMatrix.m6 /= scale.y;
  rotationMatrix.m8 /= scale.z;
  rotationMatrix.m9 /= scale.z;
  rotationMatrix.m10 /= scale.z;

  // Convert the rotation matrix to a quaternion
  Quaternion rotation = QuaternionFromMatrix(rotationMatrix);
  obj->scope->variables["rotation"] = CreateQuaternion(rotation);

  return obj;
}

static bool TryGetVector3(Object value, Vector3& vector) {
  if (value == nullptr || value->scope == nullptr) {
    return false;
  }

  auto scope = value->scope;
  auto xval = scope->variables["x"];
  auto yval = scope->variables["y"];
  auto zval = scope->variables["z"];
  if (Ctx::IsUndefined(xval, yval, zval) ||
      Ctx::IsNull(xval, yval, zval)) {
    return false;
  }

  float x, y, z;
  if (!Ctx::TryGetFloat(xval, x) || !Ctx::TryGetFloat(yval, y) ||
      !Ctx::TryGetFloat(zval, z)) {
    return false;
  }

  vector = Vector3{x, y, z};
  return true;
}
static bool TryGetQuaternion(Object value, Quaternion& quat) {
  if (value == nullptr || value->scope == nullptr) {
    return false;
  }

  auto scope = value->scope;
  auto xval = scope->variables["x"];
  auto yval = scope->variables["y"];
  auto zval = scope->variables["z"];
  auto wval = scope->variables["w"];
  if (Ctx::IsUndefined(xval, yval, zval, wval) ||
      Ctx::IsNull(xval, yval, zval, wval)) {
    return false;
  }

  float x, y, z, w;
  if (!Ctx::TryGetFloat(xval, x) || !Ctx::TryGetFloat(yval, y) ||
      !Ctx::TryGetFloat(zval, z) || !Ctx::TryGetFloat(wval, w)) {
    return false;
  }

  quat = Quaternion{x, y, z, w};
  return true;
}
static bool TryGetMatrix(Object value, Matrix& matrix) {
  if (value == nullptr || value->scope == nullptr) {
    return false;
  }
  
  auto scope = value->scope;
  auto positionVal = scope->variables["position"];
  auto scaleVal = scope->variables["scale"];
  auto rotationVal = scope->variables["rotation"];
  
  Object posObj;
  Object scaleObj;
  Object rotationObj;
  if (!Ctx::TryGetObject(positionVal, posObj) || !Ctx::TryGetObject(scaleVal, scaleObj) ||
      !Ctx::TryGetObject(rotationVal, rotationObj)) {
    return false;
  }
  
  Vector3 position;
  Vector3 scale;
  Quaternion rotation;
  if (!TryGetVector3(posObj, position) ||
      !TryGetVector3(scaleObj, scale) ||
      !TryGetQuaternion(rotationObj, rotation)) {
    return false;
  }
  
  // Create the transformation matrix
  matrix = MatrixIdentity();
  matrix = MatrixMultiply(matrix, MatrixTranslate(position.x, position.y, position.z));
  matrix = MatrixMultiply(matrix, MatrixScale(scale.x, scale.y, scale.z));
  matrix = MatrixMultiply(matrix, QuaternionToMatrix(rotation));
  return true;
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

static Value loadModel(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments");
  }
  string fileName;
  if (!Ctx::TryGetString(args[0], fileName)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Model model = LoadModel(fileName.c_str());
  loadedModels.push_back(model);
  return Ctx::CreateInt(loadedModels.size() - 1);
}
static Value drawModel(std::vector<Value> args) {
  if (args.size() < 3) {
    return Ctx::CreateString("Invalid arguments");
  }
  int modelId;
  if (!Ctx::TryGetInt(args[0], modelId)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  if (modelId < 0 || modelId >= loadedModels.size()) {
    return Ctx::CreateString("Invalid model ID");
  }
  float posX, posY, posZ, scale;
  Object colorObj;
  Object positionObj;
  if (!Ctx::TryGetObject(args[0], positionObj) || !Ctx::TryGetFloat(args[1], scale) ||
      !Ctx::TryGetObject(args[2], colorObj)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Vector3 position;
  if (!TryGetVector3(positionObj, position)) {
    return Ctx::CreateString("Invalid position object");
  }
  Color color;
  if (!TryGetColor(colorObj, color)) {
    return Ctx::CreateString("Invalid color object");
  }
  DrawModel(loadedModels[modelId], position, scale, color);
  return Value_T::Undefined;
}
static Value unloadModel(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments");
  }
  int modelId;
  if (!Ctx::TryGetInt(args[0], modelId)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  if (modelId < 0 || modelId >= loadedModels.size()) {
    return Ctx::CreateString("Invalid model ID");
  }
  UnloadModel(loadedModels[modelId]);
  loadedModels.erase(loadedModels.begin() + modelId);
  return Value_T::Undefined;
}

static Value loadShader(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments");
  }
  string vsFileName, fsFileName;
  if (!Ctx::TryGetString(args[0], vsFileName) || !Ctx::TryGetString(args[1], fsFileName)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Shader shader = LoadShader(vsFileName.c_str(), fsFileName.c_str());
  return Ctx::CreateInt((int)shader.id);
}
static Value unloadShader(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments");
  }
  int shaderId;
  if (!Ctx::TryGetInt(args[0], shaderId)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Shader shader;
  shader.id = (unsigned int)shaderId;
  UnloadShader(shader);
  return Value_T::Undefined;
}
