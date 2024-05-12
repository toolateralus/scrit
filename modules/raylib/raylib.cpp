#include "raylib.hpp"
#include "helpers.hpp"
#include "input.hpp"
#include <scrit/native.hpp>

extern "C" ScritModDef *InitScritModule_raylib() {
  ScritModDef *def = CreateModDef();
  *def->description = "raylib bindings for scrit scripting lang";

  // NOTE:
  // this looks atrocious and intimidating, only because we create argument
  // signatures to help the LSP provide more information about native functions
  // to make it easier to use. this is optional

  // use AddFunctionNoInfo("funname", &funcPtr)
  // if you don't need type annotations in the LSP.
  // Note that this may make it significantly harder to use your library.

  // That would be a lot neater than what ive done here.
  // windowing.
  AddFunction(def, "initWindow", &initWindow, ValueType::Undefined,
              {{ValueType::Int, "w"},
               {ValueType::Int, "h"},
               {ValueType::String, "title"}});

  AddFunction(def, "windowShouldClose", &windowShouldClose, ValueType::Bool);

  // Rendering utils.
  AddFunction(def, "beginDrawing", &beginDrawing, ValueType::Undefined);
  AddFunction(def, "endDrawing", &endDrawing, ValueType::Undefined);
  AddFunction(def, "loadModel", &loadModel, ValueType::Undefined,
              {{ValueType::String, "filename"}});
  AddFunction(def, "drawModel", &drawModel, ValueType::Undefined,
              {{ValueType::Object, "model"},
               {ValueType::Object, "position"},
               {ValueType::Float, "scale"},
               {ValueType::Object, "tint"}});
  AddFunction(def, "unloadModel", &unloadModel, ValueType::Undefined,
              {{ValueType::Object, "model"}});
  AddFunction(
      def, "loadShader", &loadShader, ValueType::Undefined,
      {{ValueType::String, "vsFileName"}, {ValueType::String, "fsFileName"}});
  AddFunction(def, "unloadShader", &unloadShader, ValueType::Undefined,
              {{ValueType::Object, "shader"}});
  AddFunction(def, "loadTexture", &loadTexture, ValueType::Undefined,
              {{ValueType::String, "fileName"}});
  AddFunction(def, "drawTexture", &drawTexture, ValueType::Undefined,
              {{ValueType::Object, "texture"},
               {ValueType::Object, "position"},
               {ValueType::Object, "tint"}});

  // Rendering
  AddFunction(def, "clearBackground", &clearBackground, ValueType::Undefined,
              {{ValueType::Object, "color"}});
  AddFunction(def, "drawText", &drawText, ValueType::Undefined,
              {{ValueType::String, "text"},
               {ValueType::Int, "x"},
               {ValueType::Int, "y"},
               {ValueType::Int, "fontSize"},
               {ValueType::Object, "color"}});
  AddFunction(def, "drawCircle", &drawCircle, ValueType::Undefined,
              {{ValueType::Int, "centerX"},
               {ValueType::Int, "centerY"},
               {ValueType::Float, "radius"},
               {ValueType::Object, "color"}});
  AddFunction(def, "drawLine", &drawLine, ValueType::Undefined,
              {{ValueType::Int, "startX"},
               {ValueType::Int, "startY"},
               {ValueType::Int, "endX"},
               {ValueType::Int, "endY"},
               {ValueType::Object, "color"}});
  AddFunction(def, "drawTriangle", &drawTriangle, ValueType::Undefined,
              {{ValueType::Object, "v1"},
               {ValueType::Object, "v2"},
               {ValueType::Object, "v3"},
               {ValueType::Object, "color"}});
  AddFunction(def, "drawRectangle", &drawRectangle, ValueType::Undefined,
              {{ValueType::Int, "x"},
               {ValueType::Int, "y"},
               {ValueType::Int, "width"},
               {ValueType::Int, "height"},
               {ValueType::Object, "color"}});
  AddFunction(def, "drawRectangleLines", &drawRectangleLines,
              ValueType::Undefined,
              {{ValueType::Int, "x"},
               {ValueType::Int, "y"},
               {ValueType::Int, "width"},
               {ValueType::Int, "height"},
               {ValueType::Object, "color"}});

  // Input
  AddFunction(def, "isKeyDown", &isKeyDown, ValueType::Bool,
              {{ValueType::Int, "key"}});
  AddFunction(def, "isMouseButtonDown", &isMouseButtonDown, ValueType::Bool,
              {{ValueType::Int, "button"}});
  AddFunction(def, "getMousePosition", &getMousePosition, ValueType::Object);
  AddFunction(def, "setMousePosition", &setMousePosition, ValueType::Undefined,
              {{ValueType::Int, "x"}, {ValueType::Int, "y"}});
  AddFunction(def, "getMouseWheelMove", &getMouseWheelMove, ValueType::Float);
  AddFunction(def, "drawFPS", &drawFPS, ValueType::Undefined);

  // keys enum
  Object keys = Ctx::CreateObject();
  for (const auto &[key, val] : GetKeysMap()) {
    auto value = std::dynamic_pointer_cast<Value_T>(Ctx::CreateInt(val));
    keys->SetMember(key, value);
  }
  AddVariable(def, "keys", keys);

  // colors enum.
  Object colors = Ctx::CreateObject();
  for (const auto &[key, val] : GetRaylibColorsMap()) {
    colors->scope->variables[key] = CreateColor(val);
  }
  AddVariable(def, "colors", colors);
  return def;
}
