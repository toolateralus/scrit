#pragma once
#include "helpers.hpp"
#include "input.hpp"
#include <string>
#include <vector>
#include <map>

using std::vector;

static vector<Model> loadedModels = {};


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
