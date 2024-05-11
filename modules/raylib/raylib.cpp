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
              CreateArgumentSignature({Argument(ValueType::Int, "w"),
                                       Argument(ValueType::Int, "h"),
                                       Argument(ValueType::String, "title")}));

  AddFunction(def, "windowShouldClose", &windowShouldClose, ValueType::Bool);

  // Rendering utils.
  AddFunction(def, "beginDrawing", &beginDrawing, ValueType::Undefined);
  AddFunction(def, "endDrawing", &endDrawing, ValueType::Undefined);
  AddFunction(
      def, "loadModel", &loadModel, ValueType::Undefined,
      CreateArgumentSignature({Argument(ValueType::String, "filename")}));
  AddFunction(def, "drawModel", &drawModel, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Object, "model"),
                                       Argument(ValueType::Object, "position"),
                                       Argument(ValueType::Float, "scale"),
                                       Argument(ValueType::Object, "tint")}));
  AddFunction(def, "unloadModel", &unloadModel, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Object, "model")}));
  AddFunction(
      def, "loadShader", &loadShader, ValueType::Undefined,
      CreateArgumentSignature({Argument(ValueType::String, "vsFileName"),
                               Argument(ValueType::String, "fsFileName")}));
  AddFunction(def, "unloadShader", &unloadShader, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Object, "shader")}));
  AddFunction(
      def, "loadTexture", &loadTexture, ValueType::Undefined,
      CreateArgumentSignature({Argument(ValueType::String, "fileName")}));
  AddFunction(def, "drawTexture", &drawTexture, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Object, "texture"),
                                       Argument(ValueType::Object, "position"),
                                       Argument(ValueType::Object, "tint")}));

  // Rendering
  AddFunction(def, "clearBackground", &clearBackground, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Object, "color")}));
  AddFunction(def, "drawText", &drawText, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::String, "text"),
                                       Argument(ValueType::Int, "x"),
                                       Argument(ValueType::Int, "y"),
                                       Argument(ValueType::Int, "fontSize"),
                                       Argument(ValueType::Object, "color")}));
  AddFunction(def, "drawCircle", &drawCircle, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Int, "centerX"),
                                       Argument(ValueType::Int, "centerY"),
                                       Argument(ValueType::Float, "radius"),
                                       Argument(ValueType::Object, "color")}));
  AddFunction(def, "drawLine", &drawLine, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Int, "startX"),
                                       Argument(ValueType::Int, "startY"),
                                       Argument(ValueType::Int, "endX"),
                                       Argument(ValueType::Int, "endY"),
                                       Argument(ValueType::Object, "color")}));
  AddFunction(def, "drawTriangle", &drawTriangle, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Object, "v1"),
                                       Argument(ValueType::Object, "v2"),
                                       Argument(ValueType::Object, "v3"),
                                       Argument(ValueType::Object, "color")}));
  AddFunction(def, "drawRectangle", &drawRectangle, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Int, "x"),
                                       Argument(ValueType::Int, "y"),
                                       Argument(ValueType::Int, "width"),
                                       Argument(ValueType::Int, "height"),
                                       Argument(ValueType::Object, "color")}));
  AddFunction(def, "drawRectangleLines", &drawRectangleLines,
              ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Int, "x"),
                                       Argument(ValueType::Int, "y"),
                                       Argument(ValueType::Int, "width"),
                                       Argument(ValueType::Int, "height"),
                                       Argument(ValueType::Object, "color")}));

  // Input
  AddFunction(def, "isKeyDown", &isKeyDown, ValueType::Bool,
              CreateArgumentSignature({Argument(ValueType::Int, "key")}));
  AddFunction(def, "isMouseButtonDown", &isMouseButtonDown, ValueType::Bool,
              CreateArgumentSignature({Argument(ValueType::Int, "button")}));
  AddFunction(def, "getMousePosition", &getMousePosition, ValueType::Object);
  AddFunction(def, "setMousePosition", &setMousePosition, ValueType::Undefined,
              CreateArgumentSignature({Argument(ValueType::Int, "x"),
                                       Argument(ValueType::Int, "y")}));
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
