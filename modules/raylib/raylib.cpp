#include "raylib.hpp"


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
