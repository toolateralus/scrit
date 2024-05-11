#include "raylib.hpp"
#include "input.hpp"
#include "helpers.hpp"
#include <scrit/native.hpp>


extern "C" ScritModDef *InitScritModule_raylib() {
  ScritModDef *def = CreateModDef();
  *def->description = "raylib bindings for scrit scripting lang";
  
  // windowing.
  ScritMod_AddFunction(def, "initWindow", &initWindow);
  ScritMod_AddFunction(def, "windowShouldClose", &windowShouldClose);
  
  // Rendering utils.
  ScritMod_AddFunction(def, "beginDrawing", &beginDrawing);
  ScritMod_AddFunction(def, "endDrawing", &endDrawing);
  ScritMod_AddFunction(def, "loadModel", &loadModel);
  ScritMod_AddFunction(def, "drawModel", &drawModel);
  ScritMod_AddFunction(def, "unloadModel", &unloadModel);
  ScritMod_AddFunction(def, "loadShader", &loadShader);
  ScritMod_AddFunction(def, "unloadShader", &unloadShader);
  ScritMod_AddFunction(def, "loadTexture", &loadTexture);
  ScritMod_AddFunction(def, "drawTexture", &drawTexture);
  
  // Rendering 
  ScritMod_AddFunction(def, "clearBackground", &clearBackground);
  ScritMod_AddFunction(def, "drawText", &drawText);
  ScritMod_AddFunction(def, "drawCircle", &drawCircle);
  ScritMod_AddFunction(def, "drawLine", &drawLine);
  ScritMod_AddFunction(def, "drawTriangle", &drawTriangle);
  ScritMod_AddFunction(def, "drawRectangle", &drawRectangle);
  ScritMod_AddFunction(def, "drawRectangleLines", &drawRectangleLines);
  
  // Input
  ScritMod_AddFunction(def, "isKeyDown", &isKeyDown);
  ScritMod_AddFunction(def, "isMouseButtonDown", &isMouseButtonDown);
  ScritMod_AddFunction(def, "getMousePosition", &getMousePosition);
  ScritMod_AddFunction(def, "setMousePosition", &setMousePosition);
  ScritMod_AddFunction(def, "getMouseWheelMove", &getMouseWheelMove);
  
  
  // keys enum  
  Object keys = Ctx::CreateObject();
  for (const auto &[key, val] : GetKeysMap()) {
    auto value = std::dynamic_pointer_cast<Value_T>(Ctx::CreateInt(val));
    keys->SetMember(key, value);
  }
  ScritMod_AddVariable(def, "keys", keys);
  
  // colors enum.  
  Object colors = Ctx::CreateObject();
  for (const auto &[key,val] : GetRaylibColorsMap()) {
    colors->scope->variables[key] = CreateColor(val);
  }
  ScritMod_AddVariable(def, "colors", colors);
  return def;
}
