#include "ast.hpp"
#include "context.hpp"
#include "lexer.hpp"
#include "native.hpp"
#include "serializer.hpp"
#include "type.hpp"
#include "value.hpp"
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

#define undefined Ctx::Null()
#define null Ctx::Null()

REGISTER_FUNCTION(mod, "int", {"int", "int"}) {
  int64_t v;
  int64_t mod;

  if (args.empty() || !Ctx::TryGetInt(args[0], v) ||
      !Ctx::TryGetInt(args[1], mod)) {
    return undefined;
  }
  return Ctx::CreateInt(v % mod);
}

REGISTER_FUNCTION(fmod, "float", {"float", "float"}) {
  double v;
  double mod;
  if (args.empty() || !Ctx::TryGetFloat(args[0], v) ||
      !Ctx::TryGetFloat(args[1], mod)) {
    return undefined;
  }
  return Ctx::CreateFloat(std::fmod(v, mod));
}

// Create a deep clone of any value.
REGISTER_FUNCTION(clone, "any", {"any"}) {
  if (args.size() == 0) {
    return Ctx::Null();
  }
  return args[0]->Clone();
}

// have to do this obnoxiously since it just auto-conflicts.
#undef assert
REGISTER_FUNCTION(assert, "null", {"bool", "any"}) {
  if (args.empty()) {
    return Bool_T::False;
  }
  if (!args[0]->Equals(Bool_T::True)) {
    if (args.size() > 1) {
      throw std::runtime_error("assertion failed: " + args[1]->ToString());
    }
    throw std::runtime_error("assertion failed: " + args[0]->ToString());
  }
  return Ctx::Null();
}

// get_type
REGISTER_FUNCTION(get_type, "string", {"any"}) {
  if (args.empty()) {
    return Ctx::Null();
  }

  if (args[0]->type) {
    auto v = args[0]->type->Name();
    return Ctx::CreateString(v);
  }
  return Ctx::CreateString("undefined -- this is a language bug.");
}
// Serializer
REGISTER_FUNCTION(serialize, "string", {"any", "object"}) {
  auto val = args[0];
  Writer::Settings settings = {};
  Object settingsObj;
  if (Ctx::TryGetObject(args[1], settingsObj)) {
    int64_t indentation = 0;
    int64_t startingIndent;
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
      settings.ref_handling = handling;
    }
  }
  Writer writer(settings);
  writer.BuildMap(val.get());
  writer.Write(val.get());
  return Ctx::CreateString(writer.stream.str());
}
// strings & chars
REGISTER_FUNCTION(tostr, "string", {"any"}) {
  if (args.empty()) {
    return Ctx::Null();
  }
  return Ctx::CreateString(args[0]->ToString());
}
REGISTER_FUNCTION(atoi, "int", {"string"}) {
  if (args.size() == 0) {
    return Ctx::Null();
  }
  string str;
  if (!Ctx::TryGetString(args[0], str)) {
    return Ctx::Null();
  }
  return Ctx::CreateInt(std::atoi(str.c_str()));
}

REGISTER_FUNCTION(atan2, "float", {"float", "float"}) {
  std::cout << "atan2" << "\n";
double 
  a = args[0]->Cast<Float_T>()->value,
  b = args[1]->Cast<Float_T>()->value;
 return Ctx::CreateFloat(atan2(a, b));
}

REGISTER_FUNCTION(get, "any", {"any", "int"}) {
  if (args[0]->GetPrimitiveType() == Values::PrimitiveType::Tuple) {
    auto tuple = args[0]->Cast<Tuple_T>();
    auto index = args[1]->Cast<Int_T>();
    return tuple->values[index->value];
  }
  return Ctx::Null();
}

REGISTER_FUNCTION(cbrt, "float", {"any"}) {
  double f;
  int64_t i;
  if (Ctx::TryGetFloat(args[0], f)) {
    return Ctx::CreateFloat(std::cbrt(f));
  } else if (Ctx::TryGetInt(args[0], i)) {
    return Ctx::CreateFloat(std::cbrt(i));
  }
  return Ctx::Null();
}

// terminal
REGISTER_FUNCTION(println, "null", {"any"}) {
  for (const auto &arg : args) {
    printf("%s\n", arg->ToString().c_str());
    ;
  }
  return Ctx::Null();
}
REGISTER_FUNCTION(print, "null", {"any"}) {
  for (const auto &arg : args) {
    printf("%s", arg->ToString().c_str());
  }
  return Ctx::Null();
}

REGISTER_FUNCTION(readln, "string", {}) {
  string s;
  std::cin >> s;
  return Ctx::CreateString(s);
}

static struct termios orig_termios;
REGISTER_FUNCTION(enter_raw_mode, "null", {}) {
  struct termios raw;
  tcgetattr(STDIN_FILENO, &orig_termios);
  raw = orig_termios;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  return Ctx::Null();
}
REGISTER_FUNCTION(exit_raw_mode, "null", {}) {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
  return Ctx::Null();
}

REGISTER_FUNCTION(readch, "string", {}) {
  char ch = getchar();
  return Ctx::CreateString(string(1, ch));
}