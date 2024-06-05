#include "native.hpp"
#include "value.hpp"
#include <cctype>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include "context.hpp"
#include "serializer.hpp"

#pragma clang diagnostic ignored "-Wunused-parameter"




// Arrays (some of these functions support strings too., push, pop, len, clear)
REGISTER_FUNCTION(expand) {
  if (args.size() < 2) {
    return Ctx::Undefined();
  }
  
  int value;
  if (args[0]->GetType() == ValueType::Array && Ctx::TryGetInt(args[1], value)) {
    auto array = dynamic_cast<Array_T*>(args[0].get());
    
    auto default_value = args.size() > 2 ? args[2] : Value_T::UNDEFINED;
    
    Callable_T *callable = nullptr;
    if (default_value->GetType() == ValueType::Callable) {
      callable = static_cast<Callable_T*>(default_value.get());
    }
    
    std::vector<Value> empty = {};
    
    for (int i = 0; i < value ; i++) {
      if (callable) {
        array->Push(callable->Call(empty));
      } else {
        array->Push(default_value);
      }
      
    }
  }
  return Ctx::Undefined();
}
REGISTER_FUNCTION(clear) {
  
  if (args.size() == 0) {
    return Ctx::Undefined();
  }
  
  if (args[0]->GetType() == ValueType::String) {
    auto str = dynamic_cast<String_T*>(args[0].get());
    str->value = "";
  } else if (args[0]->GetType() == ValueType::Array) {
    auto array = dynamic_cast<Array_T*>(args[0].get());
    array->values.clear();
  }
  return Ctx::Undefined();
}
REGISTER_FUNCTION(push) {
  if (args.empty()) {
    return Ctx::Undefined();
  }
  
  if (args[0]->GetType() == ValueType::String) {
    auto arg = static_cast<String_T*>(args[0].get());
    for (size_t i = 1; i < args.size(); i++) {
      if (args[i]->GetType() == ValueType::String) {
        auto str_arg = static_cast<String_T*>(args[i].get());
        arg->value += str_arg->value;
      }
    }
  }
  
  if (args[0]->GetType() != ValueType::Array) {
    return Ctx::Undefined();
  }
  auto array = static_cast<Array_T*>(args[0].get());
  for (size_t i = 1; i < args.size(); i++) {
    array->Push(args[i]);
  }
  return Ctx::Undefined();
}
REGISTER_FUNCTION(pop) {
  if (args.empty()) {
    return Ctx::Undefined();
  }
  
  if (args[0]->GetType() == ValueType::String) {
    auto str_value = static_cast<String_T*>(args[0].get());
    string character = std::string(1, str_value->value.back());
    str_value->value.pop_back();    
    return Ctx::CreateString(character);
  }
  
  if (args[0]->GetType() != ValueType::Array) {
    return Ctx::Undefined();
  }
  auto array = static_cast<Array_T*>(args[0].get());
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
    if (Ctx::TryGetInt(settingsObj->GetMember("startingIndent"), startingIndent)) {
      settings.StartingIndentLevel = startingIndent;
      
    }
    if (Ctx::TryGetString(settingsObj->GetMember("referenceHandling"), refHandling)) {
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
  Writer writer = {
    .settings = settings
  };
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
  for (const auto &arg: args) {
    printf("%s\n", arg->ToString().c_str());;
  }
  return Ctx::Undefined();
}
REGISTER_FUNCTION(print) {
  for (const auto &arg: args) {
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
REGISTER_FUNCTION(readln) {
  string input;
  std::cin >> input;
  return String_T::New(input);
}
REGISTER_FUNCTION(readch) {
  // Save the old terminal settings
  struct termios old_tio, new_tio;
  tcgetattr(STDIN_FILENO, &old_tio);

  // Set the new settings
  new_tio = old_tio;
  new_tio.c_lflag &=(~ICANON & ~ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

  // Read a character
  char ch;
  read(STDIN_FILENO, &ch, 1);
  
  // Restore the old terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

  // Convert the character to a string
  std::string str(1, ch);
  return String_T::New(str);
}
