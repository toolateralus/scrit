#include "ast.hpp"
#include "context.hpp"
#include "lexer.hpp"
#include "type.hpp"
#include "native.hpp"
#include "serializer.hpp"
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

// Create a deep clone of any value.
REGISTER_FUNCTION(clone) {
  if (args.size() == 0) {
    return Ctx::Undefined();
  }
  return args[0]->Clone();
}

REGISTER_FUNCTION(nameof) {
  if (args.empty()) {
    return undefined;
  }
  const auto &ctx = ASTNode::context;
  for (const auto &scope : ctx.scopes) {
    for (const auto &member: scope->Members()) {
      if (args[0]->Equals(member.second)) {
        return Ctx::CreateString(member.first.value);
      }
    }    
  }
  return undefined;
}

// have to do this obnoxiously since it just auto-conflicts.
#undef assert
REGISTER_FUNCTION(assert) {
  if (args.empty()) {
    return Bool_T::False;
  }
  if (!args[0]->Equals(Bool_T::True)) {
    if (args.size() > 1) {
      throw std::runtime_error("assertion failed: " + args[1]->ToString());
    }
    throw std::runtime_error("assertion failed: " + args[0]->ToString());
  }
  return Ctx::Undefined();
}  


// typeof
REGISTER_FUNCTION(type) {
  if (args.empty()) {
    return Ctx::Undefined();
  }
  
  if (args[0]->type){
    auto v = args[0]->type->name;
    return Ctx::CreateString(v);
  }
  return Ctx::CreateString("undefined -- this is a language bug.");
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
      settings.ref_handling = handling;
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
