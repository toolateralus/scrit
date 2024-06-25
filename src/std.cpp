#include "context.hpp"
#include "lexer.hpp"
#include "native.hpp"
#include "serializer.hpp"
#include "value.hpp"
#include <cctype>
#include <cmath>
#include <iostream>
#include <vector>


#ifdef __linux__ 
  #include <termios.h>
  #include <unistd.h>
#elif _WIN32 
  #include <windows.h>
#endif 

#pragma clang diagnostic ignored "-Wunused-parameter"

#define undefined Ctx::Undefined()
#define null Ctx::Null()

REGISTER_FUNCTION(mod) {
  int v;
  int mod;
  
  if (args.empty() || !Ctx::TryGetInt(args[0], v) ||
      !Ctx::TryGetInt(args[1], mod)) {
    return undefined;
  }
  return Ctx::CreateInt(v % mod);
}

REGISTER_FUNCTION(fmod) {
  float v;
  float mod;
  if (args.empty() || !Ctx::TryGetFloat(args[0], v) ||
      !Ctx::TryGetFloat(args[1], mod)) {
    return undefined;
  }
  return Ctx::CreateFloat(std::fmod(v, mod));
}

// Arrays (some of these functions support strings too., push, pop, len, clear)
REGISTER_FUNCTION(expand) {
  if (args.size() < 2) {
    return Ctx::Undefined();
  }

  int value;
  if (args[0]->GetType() == ValueType::Array &&
      Ctx::TryGetInt(args[1], value)) {
    auto array = dynamic_cast<Array_T *>(args[0].get());

    auto default_value = args.size() > 2 ? args[2] : Value_T::UNDEFINED;

    Callable_T *callable = nullptr;
    if (default_value->GetType() == ValueType::Callable) {
      callable = static_cast<Callable_T *>(default_value.get());
    }

    std::vector<Value> empty = {};

    for (int i = 0; i < value; i++) {
      if (callable) {
        array->Push(callable->Call(empty));
      } else {
        array->Push(default_value);
      }
    }
    return args[0];
  }
  return Ctx::Undefined();
}

// Create a deep clone of any value.
REGISTER_FUNCTION(clone) {
  if (args.size() == 0) {
    return Ctx::Undefined();
  }
  return args[0]->Clone();
}

REGISTER_FUNCTION(clear) {

  if (args.size() == 0) {
    return Ctx::Undefined();
  }

  if (args[0]->GetType() == ValueType::String) {
    auto str = dynamic_cast<String_T *>(args[0].get());
    str->value = "";
  } else if (args[0]->GetType() == ValueType::Array) {
    auto array = dynamic_cast<Array_T *>(args[0].get());
    array->values.clear();
  }
  return Ctx::Undefined();
}
REGISTER_FUNCTION(push) {
  if (args.empty()) {
    return Ctx::Undefined();
  }

  if (args[0]->GetType() == ValueType::String) {
    auto arg = static_cast<String_T *>(args[0].get());
    for (size_t i = 1; i < args.size(); i++) {
      arg->value += args[i]->ToString();
    }
  }

  if (args[0]->GetType() != ValueType::Array) {
    return Ctx::Undefined();
  }
  auto array = static_cast<Array_T *>(args[0].get());
  for (size_t i = 1; i < args.size(); i++) {
    array->Push(args[i]);
  }
  return Ctx::Undefined();
}

REGISTER_FUNCTION(index_of) {
  #define undefined Ctx::Undefined()
  if (args.size() < 2 || args[0]->GetType() != Values::ValueType::String) {
    return undefined;
  } 
  auto str = args[0]->TryCast<String_T>();
  auto srch_c = args[1]->TryCast<String_T>();
  size_t found = str->value.find(srch_c->value);
  if (found != std::string::npos) {
    return Ctx::CreateInt(found);
  }
  return Ctx::CreateInt(-1);
}


// have to do this obnoxiously since it just auto-conflicts.
#undef assert
REGISTER_FUNCTION(assert) {
  if (args.empty()) {
    return Bool_T::False;
  }
  if (!args[0]->Equals(Bool_T::True)) {
    if (args.size() > 1) {
      throw std::runtime_error(args[1]->ToString());
    }
    throw std::runtime_error("assertion failed: " + args[0]->ToString());
  }
  return Ctx::Undefined();
}  


REGISTER_FUNCTION(substring) {
  #define undefined Ctx::Undefined()
  if (args.size() < 3 || args[0]->GetType() != Values::ValueType::String) {
    return undefined;
  }
  
  auto str = args[0]->TryCast<String_T>();
  std::pair<int, int> indices;
  
  if (!Ctx::TryGetInt(args[1], indices.first)) {
    return undefined;
  }
  if (!Ctx::TryGetInt(args[2], indices.second)) {
    return undefined;
  }
  return Ctx::CreateString(str->value.substr(indices.first, indices.second));
}
REGISTER_FUNCTION(split) {
  if (args.size() < 2 || args[0]->GetType() != Values::ValueType::String ||
      args[1]->GetType() != Values::ValueType::String) {
    return Ctx::Undefined();
  }
  
  string s;
  if (!Ctx::TryGetString(args[0], s)) {
    return Ctx::Undefined();
  }
  
  string delim;
  if (!Ctx::TryGetString(args[1], delim)) {
    return Ctx::Undefined();
  }
  
  if (!s.contains(delim)) {
    return Ctx::CreateArray();
  }
  
  char delimiter = delim.at(0);
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  
  return Ctx::FromStringVector(tokens);
}

REGISTER_FUNCTION(front) {
  if (args.size() == 0 || (args[0]->GetType() != Values::ValueType::String &&
                           args[0]->GetType() != Values::ValueType::Array)) {
    return Ctx::Undefined();
  }
  Array arr;
  if (Ctx::TryGetArray(args[0], arr) && arr->values.size() != 0) {
    return arr->values.front();
  }
  string str;
  if (Ctx::TryGetString(args[0], str) && str.length() != 0) {
    return Ctx::CreateString(string(1, str.front()));
  }
  return Ctx::Undefined();
}

REGISTER_FUNCTION(back) {
  if (args.size() == 0 || (args[0]->GetType() != Values::ValueType::String &&
                           args[0]->GetType() != Values::ValueType::Array)) {
    return Ctx::Undefined();
  }
  Array arr;
  if (Ctx::TryGetArray(args[0], arr) && arr->values.size() != 0) {
    return arr->values.back();
  }
  string str;
  if (Ctx::TryGetString(args[0], str) && str.length() != 0) {
    return Ctx::CreateString(string(1, str.back()));
  }
  return Ctx::Undefined();
}

REGISTER_FUNCTION(pop) {
  if (args.empty()) {
    return Ctx::Undefined();
  }

  if (args[0]->GetType() == ValueType::String) {
    auto str_value = static_cast<String_T *>(args[0].get());
    string character = std::string(1, str_value->value.back());
    str_value->value.pop_back();
    return Ctx::CreateString(character);
  }

  if (args[0]->GetType() != ValueType::Array) {
    return Ctx::Undefined();
  }
  auto array = static_cast<Array_T *>(args[0].get());
  return array->Pop();
}
REGISTER_FUNCTION(len) {
  if (args.empty()) {
    return Ctx::Undefined();
  }

  string result;
  if (Ctx::TryGetString(args[0], result)) {
    return Int_T::New(result.length());
  }

  Array array;
  if (Ctx::TryGetArray(args[0], array)) {
    return Int_T::New(array->values.size());
  }

  return Ctx::Undefined();
}

// typeof
REGISTER_FUNCTION(typeof) {
  if (args.empty()) {
    return Ctx::Undefined();
  }
  string typeName;
  return Ctx::CreateString(TypeToString(args[0]->GetType()));
}

// Serializer
REGISTER_FUNCTION(serialize) {
  if (args.empty()) {
    return Ctx::Undefined();
  }

  auto val = args[0];
  WriterSettings settings = {};
  Object settingsObj;
  if (args.size() > 1 && Ctx::TryGetObject(args[1], settingsObj)) {
    int indentation = 0;
    int startingIndent;
    string refHandling;
    if (Ctx::TryGetInt(settingsObj->GetMember("indentSize"), indentation)) {
      settings.IndentSize = indentation;
    }
    if (Ctx::TryGetInt(settingsObj->GetMember("startingIndent"),
                       startingIndent)) {
      settings.StartingIndentLevel = startingIndent;
    }
    if (Ctx::TryGetString(settingsObj->GetMember("referenceHandling"),
                          refHandling)) {
      ReferenceHandling handling = ReferenceHandling::Mark;
      if (refHandling == "mark") {
        handling = ReferenceHandling::Mark;
      } else if (refHandling == "remove") {
        handling = ReferenceHandling::Remove;
      } else if (refHandling == "preserve") {
        handling = ReferenceHandling::Preserve;
      }
      settings.ReferenceHandling = handling;
    }
  }
  Writer writer = {.settings = settings};
  writer.BuildMap(val.get());
  writer.Write(val.get());
  return Ctx::CreateString(writer.stream.str());
}

// strings & chars
REGISTER_FUNCTION(tostr) {
  if (args.empty()) {
    return Ctx::Undefined();
  }
  return Ctx::CreateString(args[0]->ToString());
}

REGISTER_FUNCTION(atoi) {
  if (args.size() == 0) {
    return Ctx::Undefined();
  }
  string str;
  if (!Ctx::TryGetString(args[0], str)) {
    return Ctx::Undefined();
  }
  return Ctx::CreateInt(std::atoi(str.c_str()));
}

REGISTER_FUNCTION(isalnum) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::isalnum(value[0]));
  }
  return Ctx::Undefined();
}
REGISTER_FUNCTION(isdigit) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::isdigit(value[0]));
  }
  return Ctx::Undefined();
}
REGISTER_FUNCTION(ispunct) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::ispunct(value[0]));
  }
  return Ctx::Undefined();
}
REGISTER_FUNCTION(isalpha) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::isalpha(value[0]));
  }
  return Ctx::Undefined();
}

// terminal
REGISTER_FUNCTION(println) {
  for (const auto &arg : args) {
    printf("%s\n", arg->ToString().c_str());
    ;
  }
  return Ctx::Undefined();
}
REGISTER_FUNCTION(print) {
  for (const auto &arg : args) {
    printf("%s", arg->ToString().c_str());
  }
  return Undefined_T::UNDEFINED;
}
REGISTER_FUNCTION(cls) {
  printf("\033[23");
  return Ctx::Undefined();
}
REGISTER_FUNCTION(set_cursor) {
  if (args.size() != 2) {
    return Ctx::Undefined();
  }
  
  int x, y;
  if (Ctx::TryGetInt(args[0], x) && Ctx::TryGetInt(args[1], y)) {
    printf("\033[%d;%dH", x, y);
  }
  
  return Ctx::Undefined();
}
REGISTER_FUNCTION(get_cursor) {
  // Query the terminal for the cursor position
  printf("\033[6n");
  
  // Read the response from the terminal
  int x, y;
  if (scanf("\033[%d;%dR", &x, &y) == 2) {
    std::vector<int> pos = {x, y};
    return Ctx::FromIntVector(pos);
  }

  return Ctx::Undefined();
}

REGISTER_FUNCTION(readln) {
  std::string input;
  std::getline(std::cin, input);
  return String_T::New(input);
}
REGISTER_FUNCTION(readch) {
char ch = 0;
#ifdef _WIN32
    DWORD mode, cc;
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    if (h == NULL) {
      return undefined;
    }
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    ReadConsole(h, &ch, 1, &cc, NULL);
    SetConsoleMode(h, mode);
#else
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    read(STDIN_FILENO, &ch, 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
#endif
  return Ctx::CreateString(std::string(1, ch));
}
