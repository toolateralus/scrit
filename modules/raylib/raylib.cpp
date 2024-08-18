#include "raylib.h"
#include <memory>
#include <scrit/ctx.hpp>
#include <scrit/native.hpp>
#include <scrit/scritmod.hpp>

// it's nice to define some helper macros like this to make the module def more concise.

#define function(name) static auto name(std::vector<Value> args) -> Value
#define object(name) static auto name() -> Value

static auto color_to_val(const Color &color) -> Value {
  auto arr = Ctx::CreateArray();
  arr->Push(Ctx::CreateInt(color.r));
  arr->Push(Ctx::CreateInt(color.g));
  arr->Push(Ctx::CreateInt(color.b));
  arr->Push(Ctx::CreateInt(color.a));
  return arr;
};

static auto color_from_val(const Value &color) -> Color {
  if (auto col = std::dynamic_pointer_cast<Array_T>(color)) {
    return Color(col->values[0]->Cast<Int_T>()->value,
                 col->values[1]->Cast<Int_T>()->value,
                 col->values[2]->Cast<Int_T>()->value,
                 col->values[3]->Cast<Int_T>()->value);
  }
  return WHITE;
}

static auto string_to_key(const std::string &str) -> int{
  int key = -1;
  switch (str[0]) {
    case 'a': key = KEY_A; break;
    case 'b': key = KEY_B; break;
    case 'c': key = KEY_C; break;
    case 'd': key = KEY_D; break;
    case 'e': key = KEY_E; break;
    case 'f': key = KEY_F; break;
    case 'g': key = KEY_G; break;
    case 'h': key = KEY_H; break;
    case 'i': key = KEY_I; break;
    case 'j': key = KEY_J; break;
    case 'k': key = KEY_K; break;
    case 'l': key = KEY_L; break;
    case 'm': key = KEY_M; break;
    case 'n': key = KEY_N; break;
    case 'o': key = KEY_O; break;
    case 'p': key = KEY_P; break;
    case 'q': key = KEY_Q; break;
    case 'r': key = KEY_R; break;
    case 's': key = KEY_S; break;
    case 't': key = KEY_T; break;
    case 'u': key = KEY_U; break;
    case 'v': key = KEY_V; break;
    case 'w': key = KEY_W; break;
    case 'x': key = KEY_X; break;
    case 'y': key = KEY_Y; break;
    case 'z': key = KEY_Z; break;
    default: break;
  }
  return key;
}

function(init_window) {
  auto w = args[0]->Cast<Int_T>()->value;
  auto h = args[1]->Cast<Int_T>()->value;
  auto title = args[2]->Cast<String_T>()->value;
  InitWindow(w, h, title.c_str());
  return Ctx::Null();
}

function(window_should_close) { return Ctx::CreateBool(WindowShouldClose()); }

function(begin_drawing) {
  BeginDrawing();
  return Ctx::Null();
}

function(get_mouse_position) {
  auto x = GetMouseX();
  auto y = GetMouseY();
  auto arr = Ctx::CreateArray();
  arr->Push(Ctx::CreateInt(x));
  arr->Push(Ctx::CreateInt(y));
  return arr;
}

function(draw_pixel) {
  int64_t x = args[0]->Cast<Int_T>()->value;
  int64_t y = args[1]->Cast<Int_T>()->value;
  Color color = color_from_val(args[2]);
  DrawPixel(x, y, color);
  return Ctx::Null();
}

function(is_key_down) {
  auto key = string_to_key(args[0]->Cast<String_T>()->value);
  return Ctx::CreateBool(IsKeyDown(key));
}
function(is_key_up) {
  auto key = string_to_key(args[0]->Cast<String_T>()->value);
  return Ctx::CreateBool(IsKeyUp(key));
}
function(is_key_released) {
  auto key = string_to_key(args[0]->Cast<String_T>()->value);
  return Ctx::CreateBool(IsKeyReleased(key));
}
function(is_key_pressed) {
  auto key = string_to_key(args[0]->Cast<String_T>()->value);
  return Ctx::CreateBool(IsKeyPressed(key));
}

function(end_drawing) {
  EndDrawing();
  return Ctx::Null();
}

function(get_scroll) {
  return Ctx::CreateFloat(GetMouseWheelMove());
}

function(clear_background) {
  auto color = color_from_val(args[0]);
  ClearBackground(color);
  return Ctx::Null();
}

function(draw_line) {
  int64_t start_x = args[0]->Cast<Int_T>()->value,
          start_y = args[1]->Cast<Int_T>()->value,
          end_x = args[2]->Cast<Int_T>()->value,
          end_y = args[3]->Cast<Int_T>()->value;
  Color color = color_from_val(args[4]);
  DrawLine(start_x, start_y, end_x, end_y, color);
  return Ctx::Null();
}

function(draw_circle) {
  int64_t start_x = args[0]->Cast<Int_T>()->value,
          start_y = args[1]->Cast<Int_T>()->value;
  Color color = color_from_val(args[3]);
          
  double radius = args[2]->Cast<Float_T>()->value;
  DrawCircle(start_x, start_y, radius, color);
  return Ctx::Null();
}

function(draw_rectangle) {
  int64_t start_x = args[0]->Cast<Int_T>()->value,
          start_y = args[1]->Cast<Int_T>()->value,
          end_x = args[2]->Cast<Int_T>()->value,
          end_y = args[3]->Cast<Int_T>()->value;
  Color color = color_from_val(args[4]);
  DrawRectangle(start_x, start_y, end_x, end_y, color);
  return Ctx::Null();
}

object(colors) {
  auto object = Ctx::CreateObject();
  object->SetMember("LIGHTGRAY", color_to_val(LIGHTGRAY));
  object->SetMember("GRAY", color_to_val(GRAY));
  object->SetMember("DARKGRAY", color_to_val(DARKGRAY));
  object->SetMember("YELLOW", color_to_val(YELLOW));
  object->SetMember("GOLD", color_to_val(GOLD));
  object->SetMember("ORANGE", color_to_val(ORANGE));
  object->SetMember("PINK", color_to_val(PINK));
  object->SetMember("RED", color_to_val(RED));
  object->SetMember("MAROON", color_to_val(MAROON));
  object->SetMember("GREEN", color_to_val(GREEN));
  object->SetMember("LIME", color_to_val(LIME));
  object->SetMember("DARKGREEN", color_to_val(DARKGREEN));
  object->SetMember("SKYBLUE", color_to_val(SKYBLUE));
  object->SetMember("BLUE", color_to_val(BLUE));
  object->SetMember("DARKBLUE", color_to_val(DARKBLUE));
  object->SetMember("PURPLE", color_to_val(PURPLE));
  object->SetMember("VIOLET", color_to_val(VIOLET));
  object->SetMember("DARKPURPLE", color_to_val(DARKPURPLE));
  object->SetMember("BEIGE", color_to_val(BEIGE));
  object->SetMember("BROWN", color_to_val(BROWN));
  object->SetMember("DARKBROWN", color_to_val(DARKBROWN));
  object->SetMember("WHITE", color_to_val(WHITE));
  object->SetMember("BLACK", color_to_val(BLACK));
  object->SetMember("BLANK", color_to_val(BLANK));
  object->SetMember("MAGENTA", color_to_val(MAGENTA));
  object->SetMember("RAYWHITE", color_to_val(RAYWHITE));
  return object;
}

function(set_target_fps) {
  SetTargetFPS(args[0]->Cast<Int_T>()->value);
  return Ctx::Null();
}



extern "C" ScritModDef *InitScritModule_raylib() {
  ScritModDef *def = CreateModDef();
  *def->description = "raylib bindings for 'scrit' language.";
  def->AddFunction("init_window", CREATE_FUNCTION(init_window, "null",
                                                  {"int", "int", "string"}));
  def->AddFunction("window_should_close",
                   CREATE_FUNCTION(window_should_close, "bool", {}));
  def->AddFunction("begin_drawing", CREATE_FUNCTION(begin_drawing, "null", {}));
  def->AddFunction("end_drawing", CREATE_FUNCTION(end_drawing, "null", {}));
  
  def->AddFunction("draw_line", CREATE_FUNCTION(draw_line, "null", {"int", "int", "int", "int", "any"}));
  def->AddFunction("draw_circle", CREATE_FUNCTION(draw_circle, "null", {"int", "int", "float", "any"}));
  def->AddFunction("draw_rectangle", CREATE_FUNCTION(draw_rectangle, "null", {"int", "int", "int", "int", "any"}));
  
  def->AddFunction("clear_background", CREATE_FUNCTION(clear_background, "null", {"any"}));
  
  def->AddFunction("is_key_down", CREATE_FUNCTION(is_key_down, "bool", {"string"}));
  def->AddFunction("is_key_up", CREATE_FUNCTION(is_key_up, "bool", {"string"}));
  def->AddFunction("is_key_pressed", CREATE_FUNCTION(is_key_pressed, "bool", {"string"}));
  def->AddFunction("is_key_released", CREATE_FUNCTION(is_key_released, "bool", {"string"}));
  def->AddFunction("set_target_fps", CREATE_FUNCTION(set_target_fps, "null", {"int"}));
  
  def->AddFunction("get_scroll", CREATE_FUNCTION(get_scroll, "float", {}));
  def->AddFunction("get_mouse_position", CREATE_FUNCTION(get_mouse_position, "array", {}));
  //def->AddVariable("Colors", colors(), Mutability::Const);
  return def;
}