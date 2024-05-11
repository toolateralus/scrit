#pragma once
#include "helpers.hpp"
#include <string>
#include <vector>

using std::vector;

static vector<Model> loadedModels = {};
static vector<Texture> loadedTextures = {};

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


static Value drawText(std::vector<Value> args) {
  if (args.size() < 4) {
    return Ctx::CreateString("Invalid arguments");
  }
  int posX, posY, fontSize;
  Object colorObj;
  string text;
  if (!Ctx::TryGetInt(args[0], posX) || !Ctx::TryGetInt(args[1], posY) ||
      !Ctx::TryGetInt(args[2], fontSize) || !Ctx::TryGetString(args[3], text) ||
      !Ctx::TryGetObject(args[4], colorObj)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Color color;
  if (!TryGetColor(colorObj, color)) {
    return Ctx::CreateString("Invalid color object");
  }
  DrawText(text.c_str(), posX, posY, fontSize, color);
  return Value_T::Undefined;
}
static Value drawCircle(std::vector<Value> args) {
  if (args.size() < 4) {
    return Ctx::CreateString("Invalid arguments");
  }
  int centerX, centerY, radius;
  Object colorObj;
  if (!Ctx::TryGetInt(args[0], centerX) || !Ctx::TryGetInt(args[1], centerY) ||
      !Ctx::TryGetInt(args[2], radius) || !Ctx::TryGetObject(args[3], colorObj)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Color color;
  if (!TryGetColor(colorObj, color)) {
    return Ctx::CreateString("Invalid color object");
  }
  DrawCircle(centerX, centerY, radius, color);
  return Value_T::Undefined;
}
static Value drawLine(std::vector<Value> args) {
  if (args.size() < 5) {
    return Ctx::CreateString("Invalid arguments");
  }
  int startPosX, startPosY, endPosX, endPosY, thickness;
  Object colorObj;
  if (!Ctx::TryGetInt(args[0], startPosX) || !Ctx::TryGetInt(args[1], startPosY) ||
      !Ctx::TryGetInt(args[2], endPosX) || !Ctx::TryGetInt(args[3], endPosY) ||
      !Ctx::TryGetInt(args[4], thickness) || !Ctx::TryGetObject(args[5], colorObj)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Color color;
  if (!TryGetColor(colorObj, color)) {
    return Ctx::CreateString("Invalid color object");
  }
  DrawLine(startPosX, startPosY, endPosX, endPosY, color);
  return Value_T::Undefined;
}
static Value drawTriangle(std::vector<Value> args) {
  if (args.size() < 7) {
    return Ctx::CreateString("Invalid arguments");
  }
  float posX1, posY1, posX2, posY2, posX3, posY3;
  Object colorObj;
  if (!Ctx::TryGetFloat(args[0], posX1) || !Ctx::TryGetFloat(args[1], posY1) ||
      !Ctx::TryGetFloat(args[2], posX2) || !Ctx::TryGetFloat(args[3], posY2) ||
      !Ctx::TryGetFloat(args[4], posX3) || !Ctx::TryGetFloat(args[5], posY3) ||
      !Ctx::TryGetObject(args[6], colorObj)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Color color;
  if (!TryGetColor(colorObj, color)) {
    return Ctx::CreateString("Invalid color object");
  }
  DrawTriangle(Vector2{posX1, posY1}, Vector2{posX2, posY2}, Vector2{posX3, posY3}, color);
  return Value_T::Undefined;
}
static Value drawRectangleLines(std::vector<Value> args) {
  if (args.size() < 5) {
    return Ctx::CreateString("Invalid arguments");
  }
  int posX, posY, width, height, thickness;
  Object colorObj;
  if (!Ctx::TryGetInt(args[0], posX) || !Ctx::TryGetInt(args[1], posY) ||
      !Ctx::TryGetInt(args[2], width) || !Ctx::TryGetInt(args[3], height) ||
      !Ctx::TryGetInt(args[4], thickness) || !Ctx::TryGetObject(args[5], colorObj)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Color color;
  if (!TryGetColor(colorObj, color)) {
    return Ctx::CreateString("Invalid color object");
  }
  DrawRectangleLines(posX, posY, width, height, color);
  return Value_T::Undefined;
}

static Value loadTexture(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments");
  }
  string fileName;
  if (!Ctx::TryGetString(args[0], fileName)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Texture2D texture = LoadTexture(fileName.c_str());
  
  if (texture.id == 0) {
    return Ctx::CreateString("failed to load texture");
  }
  
  if (texture.id >= loadedTextures.size()) {
    loadedTextures.resize(texture.id + 1);
  }
  loadedTextures[texture.id] = texture;
  
  return CreateTexture(texture);
}
static Value drawTexture(std::vector<Value> args) {
  if (args.size() < 3) {
    return Ctx::CreateString("Invalid arguments");
  }
  int posX, posY, texID;
  if (!Ctx::TryGetInt(args[0], posX) || !Ctx::TryGetInt(args[1], posY) ||
      !Ctx::TryGetInt(args[2], texID)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  auto tex = loadedTextures[texID];
  DrawTexture(tex, posX, posY, WHITE);
  return Value_T::Undefined;
}


static std::map<std::string, Color> GetRaylibColorsMap() {
  std::map<std::string, Color> colorsMap;
  colorsMap["LIGHTGRAY"] = LIGHTGRAY;
  colorsMap["GRAY"] = GRAY;
  colorsMap["DARKGRAY"] = DARKGRAY;
  colorsMap["YELLOW"] = YELLOW;
  colorsMap["GOLD"] = GOLD;
  colorsMap["ORANGE"] = ORANGE;
  colorsMap["PINK"] = PINK;
  colorsMap["RED"] = RED;
  colorsMap["MAROON"] = MAROON;
  colorsMap["GREEN"] = GREEN;
  colorsMap["LIME"] = LIME;
  colorsMap["DARKGREEN"] = DARKGREEN;
  colorsMap["SKYBLUE"] = SKYBLUE;
  colorsMap["BLUE"] = BLUE;
  colorsMap["DARKBLUE"] = DARKBLUE;
  colorsMap["PURPLE"] = PURPLE;
  colorsMap["VIOLET"] = VIOLET;
  colorsMap["DARKPURPLE"] = DARKPURPLE;
  colorsMap["BEIGE"] = BEIGE;
  colorsMap["BROWN"] = BROWN;
  colorsMap["DARKBROWN"] = DARKBROWN;
  colorsMap["WHITE"] = WHITE;
  colorsMap["BLACK"] = BLACK;
  colorsMap["BLANK"] = BLANK;
  colorsMap["MAGENTA"] = MAGENTA;
  colorsMap["RAYWHITE"] = RAYWHITE;
  return colorsMap;
}

