#include "raylib.hpp"
#include "helpers.hpp"
#include "input.hpp"
#include <scrit/native.hpp>

extern "C" ScritModDef *InitScritModule_raylib() {
  ScritModDef *def = CreateModDef();
  *def->description = "raylib bindings for scrit scripting lang";
  
  
  // NOTE: 
  // this looks atrocious and intimidating, only because we create argument signatures
  // to help the LSP provide more information about native functions to make it easier to use.
  // this is optional
  
  // Another alternative to :
  // NativeFunction::Create(std::string name, NativeFunctionPtr &ptr) is easier to use.
  
  // Is to create factory functions and write your wrappers separately.
  // for example
  
  /*
    NativeFunction getInitWindow() {
      return NativeFunction::Create(
                                "initWindow", "undefined|err", &initWindow,
                                CreateArgumentSignature(
                                    {Argument("int", "w"), Argument("int", "h"),
                                     Argument("string", "title")}))
    } 
    Value initWindow(std::vector<Value> args) {
      ... your implementation
    }
  
  */
  
  // That would be a lot neater than what ive done here.
  
  // windowing.
  AddFunction(def, NativeFunction::Create(
                                "initWindow", "undefined|err", &initWindow,
                                CreateArgumentSignature(
                                    {Argument("int", "w"), Argument("int", "h"),
                                     Argument("string", "title")})));
                                     
  AddFunction(def, NativeFunction::Create("windowShouldClose", "bool",
                                                   &windowShouldClose));
  
  // Rendering utils.
  AddFunction(
      def, NativeFunction::Create("beginDrawing", "undefined", &beginDrawing));
  AddFunction(
      def, NativeFunction::Create("endDrawing", "undefined", &endDrawing));
  AddFunction(
      def, NativeFunction::Create(
               "loadModel", "undefined|err", &loadModel,
               CreateArgumentSignature({Argument("string", "filename")})));
  AddFunction(
      def, NativeFunction::Create(
               "drawModel", "undefined|err", &drawModel,
               CreateArgumentSignature(
                   {Argument("Model", "model"), Argument("Vector3", "position"),
                    Argument("float", "scale"), Argument("Color", "tint")})));
  AddFunction(
      def, NativeFunction::Create(
               "unloadModel", "undefined", &unloadModel,
               CreateArgumentSignature({Argument("Model", "model")})));
  AddFunction(
      def, NativeFunction::Create(
               "loadShader", "undefined|err", &loadShader,
               CreateArgumentSignature({Argument("string", "vsFileName"),
                                        Argument("string", "fsFileName")})));
  AddFunction(
      def, NativeFunction::Create(
               "unloadShader", "undefined", &unloadShader,
               CreateArgumentSignature({Argument("Shader", "shader")})));
  AddFunction(
      def, NativeFunction::Create(
               "loadTexture", "undefined|err", &loadTexture,
               CreateArgumentSignature({Argument("string", "fileName")})));
  AddFunction(
      def, NativeFunction::Create(
               "drawTexture", "undefined|err", &drawTexture,
               CreateArgumentSignature({Argument("Texture2D", "texture"),
                                        Argument("Vector2", "position"),
                                        Argument("Color", "tint")})));

  // Rendering
  AddFunction(
      def, NativeFunction::Create(
               "clearBackground", "undefined", &clearBackground,
               CreateArgumentSignature({Argument("Color", "color")})));
  AddFunction(
      def, NativeFunction::Create(
               "drawText", "undefined|err", &drawText,
               CreateArgumentSignature(
                   {Argument("string", "text"), Argument("int", "x"),
                    Argument("int", "y"), Argument("int", "fontSize"),
                    Argument("Color", "color")})));
  AddFunction(
      def, NativeFunction::Create(
               "drawCircle", "undefined|err", &drawCircle,
               CreateArgumentSignature(
                   {Argument("int", "centerX"), Argument("int", "centerY"),
                    Argument("float", "radius"), Argument("Color", "color")})));
  AddFunction(
      def, NativeFunction::Create(
               "drawLine", "undefined|err", &drawLine,
               CreateArgumentSignature(
                   {Argument("int", "startX"), Argument("int", "startY"),
                    Argument("int", "endX"), Argument("int", "endY"),
                    Argument("Color", "color")})));
  AddFunction(
      def, NativeFunction::Create(
               "drawTriangle", "undefined|err", &drawTriangle,
               CreateArgumentSignature(
                   {Argument("Vector2", "v1"), Argument("Vector2", "v2"),
                    Argument("Vector2", "v3"), Argument("Color", "color")})));
  AddFunction(
      def, NativeFunction::Create(
               "drawRectangle", "undefined|err", &drawRectangle,
               CreateArgumentSignature(
                   {Argument("int", "x"), Argument("int", "y"),
                    Argument("int", "width"), Argument("int", "height"),
                    Argument("Color", "color")})));
  AddFunction(
      def, NativeFunction::Create(
               "drawRectangleLines", "undefined|err", &drawRectangleLines,
               CreateArgumentSignature(
                   {Argument("int", "x"), Argument("int", "y"),
                    Argument("int", "width"), Argument("int", "height"),
                    Argument("Color", "color")})));

  // Input
  AddFunction(def,
                       NativeFunction::Create(
                           "isKeyDown", "bool", &isKeyDown,
                           CreateArgumentSignature({Argument("int", "key")})));
  AddFunction(
      def, NativeFunction::Create(
               "isMouseButtonDown", "bool", &isMouseButtonDown,
               CreateArgumentSignature({Argument("int", "button")})));
  AddFunction(
      def,
      NativeFunction::Create("getMousePosition", "Vector2", &getMousePosition));
      
  AddFunction(
      def,
      NativeFunction::Create("setMousePosition", "undefined", &setMousePosition,
                             CreateArgumentSignature({Argument("int", "x"),
                                                      Argument("int", "y")})));
                                                      
  AddFunction(def, NativeFunction::Create("getMouseWheelMove", "float",
                                                   &getMouseWheelMove));
  AddFunction(
      def, NativeFunction::Create("drawFPS", "undefined", &drawFPS));
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
