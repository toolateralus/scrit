#pragma once
#include "helpers.hpp"
#include <raylib.h>
#include <scrit/context.hpp>
#include <scrit/value.hpp>
#include <string>
#include <vector>

using std::vector;

static vector<Model> loadedModels = {};
static vector<Texture> loadedTextures = {};
static vector<Camera3D> loaded3dCameras = {};


static Value initWindow(std::vector<Value> args) {
  if (args.size() != 3) {
    return Ctx::CreateString("Invalid arguments to init window");
  }
  int height, width;
  string name;
  if (!Ctx::TryGetInt(args[0], width) || !Ctx::TryGetInt(args[1], height) ||
      !Ctx::TryGetString(args[2], name)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  InitWindow(width, height, name.c_str());
  return Value_T::UNDEFINED;
}
static Value beginDrawing(std::vector<Value> args) {
  BeginDrawing();
  return Value_T::UNDEFINED;
}
static Value endDrawing(std::vector<Value> args) {
  EndDrawing();
  return Value_T::UNDEFINED;
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

  return Value_T::UNDEFINED;
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
  return Value_T::UNDEFINED;
}

static Value setModelTexture(std::vector<Value> args) {
  if (args.size() < 2) {
    return Ctx::CreateString("Invalid arguments for setModelTexture");
  }
  int modelId, textureId;
  if (!Ctx::TryGetInt(args[0], modelId) || !Ctx::TryGetInt(args[1], textureId)) {
    return Ctx::CreateString("Invalid argument types., expected int, int got: " +
                 TypeToString(args[0]->GetPrimitiveType()) + ", " +
                 TypeToString(args[1]->GetPrimitiveType()));
  }
  if (modelId < 0 || modelId >= loadedModels.size()) {
    return Ctx::CreateString("Invalid model ID");
  }
  if (textureId < 0 || textureId >= loadedTextures.size()) {
    return Ctx::CreateString("Invalid texture ID");
  }
  SetMaterialTexture(&loadedModels[modelId].materials[0], MATERIAL_MAP_DIFFUSE, loadedTextures[textureId]);
  return Value_T::UNDEFINED;
}

static Value createCamera3D(std::vector<Value> args) {
  Vector3 position = Vector3One();
  Vector3 target = Vector3Zero();
  Vector3 up = {0, 1, 0};
  float fovy = 70;
  int projection = CAMERA_PERSPECTIVE;
  if (args.size() >= 1) {
    Object positionObj;
    if (Ctx::TryGetObject(args[0], positionObj)) {
      if (!TryGetVector3(positionObj, position)) {
        return Ctx::CreateString("Invalid position object");
      }
    }
  }
  if (args.size() >= 2) {
    Object targetObj;
    if (Ctx::TryGetObject(args[1], targetObj)) {
      if (!TryGetVector3(targetObj, target)) {
        return Ctx::CreateString("Invalid target object");
      }
    }
  }
  if (args.size() >= 3) {
    Object upObj;
    if (Ctx::TryGetObject(args[2], upObj)) {
      if (!TryGetVector3(upObj, up)) {
        return Ctx::CreateString("Invalid up object");
      }
    }
  }
  if (args.size() >= 4) {
    if (!Ctx::TryGetFloat(args[3], fovy)) {
      return Ctx::CreateString("Invalid fovy value");
    }
  }
  if (args.size() >= 5) {
    if (!Ctx::TryGetInt(args[4], projection)) {
      return Ctx::CreateString("Invalid projection value");
    }
  }
  auto id = loaded3dCameras.size();
  auto camera = Camera3D{position, target, up, fovy, projection};
  loaded3dCameras.push_back(camera);
  return Ctx::CreateInt(id);
}
static Value destroyCamera3D(std::vector<Value> args) {
  int id;
  if (args.size() > 0 && Ctx::TryGetInt(args[0], id) && id >= 0 &&
      id < loaded3dCameras.size()) {
    loaded3dCameras.erase(loaded3dCameras.begin() + id);
  } else {
    return Ctx::CreateString("invalid arguments to destroyCamera");
  }
  return Value_T::UNDEFINED;
}
static Value updateCamera3DPro(std::vector<Value> args) {
  if (args.size() != 4) {
    return Ctx::CreateString(
        "Invalid parameters to updateCameraPro : expected 4");
  }
  
  int id;
  Object translation;
  Object rotation;
  float zoom;
  if (!Ctx::TryGetInt(args[0], id) || !Ctx::TryGetObject(args[1], translation) ||
      !Ctx::TryGetObject(args[2], rotation) ||
      !Ctx::TryGetFloat(args[3], zoom)) {
    return Ctx::CreateString(
        "Invalid arguments to updateCameraPro: expected int cameraID, object "
        "position , object rotation, float zoom");
  }
  
  Vector3 translate;
  Vector3 rotate;
  if (!TryGetVector3(translation, translate) || !TryGetVector3(rotation, rotate)) {
    return Ctx::CreateString(
        "Invalid position or rotation object in updateCameraPro");
  }
  
  if (id > loaded3dCameras.size()) {
    return Ctx::CreateString(
        "Invalid camera id: exceeded bounds of available cameras");
  }
  
  auto &camera = loaded3dCameras[id];
  
  UpdateCameraPro(&camera, translate, rotate, zoom);
  
  return Value_T::UNDEFINED;
}
static Value beginMode3D(std::vector<Value> args) {
  int id;
  if (args.empty() || !Ctx::TryGetInt(args[0], id)) {
    return Ctx::CreateString(
        "Invalid arguments to beginMode3D: requires a cameraID integer");
  }

  if (loaded3dCameras.size() < id) {
    return Ctx::CreateString("Camera ID out of range of available cameras");
  }

  auto cam = loaded3dCameras[id];

  BeginMode3D(cam);

  return Value_T::UNDEFINED;
}
static Value endMode3D(std::vector<Value> args) {
  EndMode3D();
  return Value_T::UNDEFINED;
}

static Value loadModel(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments for loadModel");
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
  if (args.size() < 4) {
    return Ctx::CreateString("Invalid arguments for drawModel");
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
  if (!Ctx::TryGetObject(args[1], positionObj) ||
      !Ctx::TryGetFloat(args[2], scale) ||
      !Ctx::TryGetObject(args[3], colorObj)) {

    auto types = TypeToString(args[0]->GetPrimitiveType()) + ", " +
                 TypeToString(args[1]->GetPrimitiveType()) + ", " +
                 TypeToString(args[2]->GetPrimitiveType()) + ", " +
                 TypeToString(args[3]->GetPrimitiveType());

    return Ctx::CreateString(
        "Invalid argument types for drawModel.. expected "
        "int, object(x:float, y:float, z:float), float, object "
        "{r : int, g ; int, b : int, a : int}, got : \n" +
        types);
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
  return Value_T::UNDEFINED;
}
static Value unloadModel(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments for unloadModel");
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
  return Value_T::UNDEFINED;
}
static Value loadShader(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments for loadShader");
  }
  string vsFileName, fsFileName;
  if (!Ctx::TryGetString(args[0], vsFileName) ||
      !Ctx::TryGetString(args[1], fsFileName)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Shader shader = LoadShader(vsFileName.c_str(), fsFileName.c_str());
  return Ctx::CreateInt((int)shader.id);
}
static Value unloadShader(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments for unloadShader");
  }
  int shaderId;
  if (!Ctx::TryGetInt(args[0], shaderId)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Shader shader;
  shader.id = (unsigned int)shaderId;
  UnloadShader(shader);
  return Value_T::UNDEFINED;
}
static Value drawFPS(std::vector<Value> args) {
  if (args.size() < 2) {
    return Ctx::CreateString("Invalid args");
  }
  int x, y;
  if (!Ctx::TryGetInt(args[0], x) || !Ctx::TryGetInt(args[1], y)) {
    return Ctx::CreateString("invalid args types");
  }
  DrawFPS(x, y);
  return Value_T::UNDEFINED;
}
static Value drawText(std::vector<Value> args) {
  if (args.size() < 4) {
    return Ctx::CreateString("Invalid arguments for drawText");
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
  return Value_T::UNDEFINED;
}
static Value drawCircle(std::vector<Value> args) {
  if (args.size() < 4) {
    return Ctx::CreateString("Invalid arguments for drawCircle");
  }
  int centerX, centerY, radius;
  Object colorObj;
  if (!Ctx::TryGetInt(args[0], centerX) || !Ctx::TryGetInt(args[1], centerY) ||
      !Ctx::TryGetInt(args[2], radius) ||
      !Ctx::TryGetObject(args[3], colorObj)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Color color;
  if (!TryGetColor(colorObj, color)) {
    return Ctx::CreateString("Invalid color object");
  }
  DrawCircle(centerX, centerY, radius, color);
  return Value_T::UNDEFINED;
}
static Value drawLine(std::vector<Value> args) {
  if (args.size() < 5) {
    return Ctx::CreateString("Invalid arguments for drawLine");
  }
  int startPosX, startPosY, endPosX, endPosY, thickness;
  Object colorObj;
  if (!Ctx::TryGetInt(args[0], startPosX) ||
      !Ctx::TryGetInt(args[1], startPosY) ||
      !Ctx::TryGetInt(args[2], endPosX) || !Ctx::TryGetInt(args[3], endPosY) ||
      !Ctx::TryGetInt(args[4], thickness) ||
      !Ctx::TryGetObject(args[5], colorObj)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Color color;
  if (!TryGetColor(colorObj, color)) {
    return Ctx::CreateString("Invalid color object");
  }
  DrawLine(startPosX, startPosY, endPosX, endPosY, color);
  return Value_T::UNDEFINED;
}
static Value drawTriangle(std::vector<Value> args) {
  if (args.size() < 7) {
    return Ctx::CreateString("Invalid arguments for drawTriangle");
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
  DrawTriangle(Vector2{posX1, posY1}, Vector2{posX2, posY2},
               Vector2{posX3, posY3}, color);
  return Value_T::UNDEFINED;
}
static Value drawRectangleLines(std::vector<Value> args) {
  if (args.size() < 5) {
    return Ctx::CreateString("Invalid arguments drawRectangleLines");
  }
  int posX, posY, width, height, thickness;
  Object colorObj;
  if (!Ctx::TryGetInt(args[0], posX) || !Ctx::TryGetInt(args[1], posY) ||
      !Ctx::TryGetInt(args[2], width) || !Ctx::TryGetInt(args[3], height) ||
      !Ctx::TryGetInt(args[4], thickness) ||
      !Ctx::TryGetObject(args[5], colorObj)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  Color color;
  if (!TryGetColor(colorObj, color)) {
    return Ctx::CreateString("Invalid color object");
  }
  DrawRectangleLines(posX, posY, width, height, color);
  return Value_T::UNDEFINED;
}
static Value loadTexture(std::vector<Value> args) {
  if (args.empty()) {
    return Ctx::CreateString("Invalid arguments for loadTexture");
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
    return Ctx::CreateString("Invalid arguments for drawTexture");
  }
  int posX, posY, texID;
  if (!Ctx::TryGetInt(args[0], posX) || !Ctx::TryGetInt(args[1], posY) ||
      !Ctx::TryGetInt(args[2], texID)) {
    return Ctx::CreateString("Invalid argument types.");
  }
  auto tex = loadedTextures[texID];
  DrawTexture(tex, posX, posY, WHITE);
  return Value_T::UNDEFINED;
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
