#include "raylib.hpp"
#include "helpers.hpp"
#include "input.hpp"
#include <scrit/native.hpp>
#include <scrit/value.hpp>

extern "C" ScritModDef *InitScritModule_raylib() {
  ScritModDef *def = CreateModDef();
  *def->description = "raylib bindings for scrit scripting lang";
  
  def->AddFunction("initWindow", &initWindow);
  def->AddFunction("windowShouldClose", &windowShouldClose);
  def->AddFunction("setModelTexture", &setModelTexture);
  
  // Rendering utils.
  def->AddFunction("beginDrawing", &beginDrawing);
  def->AddFunction("endDrawing", &endDrawing);
  def->AddFunction("loadModel", &loadModel);
  def->AddFunction("drawModel", &drawModel);
  def->AddFunction("unloadModel", &unloadModel);
  def->AddFunction("loadShader", &loadShader);
  def->AddFunction("unloadShader", &unloadShader);
  def->AddFunction("loadTexture", &loadTexture);
  def->AddFunction("drawTexture", &drawTexture);
  def->AddFunction("createCamera3D", &createCamera3D);
  def->AddFunction("destroyCamera3D", &destroyCamera3D);
  def->AddFunction("updateCamera3DPro", &updateCamera3DPro); // TODO: fix this signature
  def->AddFunction("beginMode3D", &beginMode3D);
  def->AddFunction("endMode3D", &endMode3D);
              
  // Rendering
  def->AddFunction("clearBackground", &clearBackground);
  def->AddFunction("drawText", &drawText);
  def->AddFunction("drawCircle", &drawCircle);
  def->AddFunction("drawLine", &drawLine);
  def->AddFunction("drawTriangle", &drawTriangle);
  def->AddFunction("drawRectangle", &drawRectangle);
  def->AddFunction("drawRectangleLines", &drawRectangleLines);
  
  // Input
  def->AddFunction("isKeyDown", &isKeyDown);
  def->AddFunction("isMouseButtonDown", &isMouseButtonDown);
  def->AddFunction("getMousePosition", &getMousePosition);
  def->AddFunction("setMousePosition", &setMousePosition);
  def->AddFunction("getMouseWheelMove", &getMouseWheelMove);
  def->AddFunction("drawFPS", &drawFPS);
  
  // keys enum
  Object keys = Ctx::CreateObject();
  for (const auto &[key, val] : GetKeysMap()) {
    auto value = std::dynamic_pointer_cast<Value_T>(Ctx::CreateInt(val));
    keys->SetMember(key, value);
  }
  def->AddVariable("keys", keys);
  
  // colors enum.
  Object colors = Ctx::CreateObject();
  for (const auto &[key, val] : GetRaylibColorsMap()) {
    colors->SetMember(key, CreateColor(val));
  }
  def->AddVariable("colors", colors);
  return def;
}
