#include "native.hpp"
#include "value.hpp"
#include <cctype>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include "context.hpp"
#include "serializer.hpp"

#pragma clang diagnostic ignored "-Wunused-parameter"


REGISTER_FUNCTION(println, ValueType::Undefined, {}) {
  for (const auto &arg: args) {
    printf("%s\n", arg->ToString().c_str());;
  }
  return Value_T::UNDEFINED;
}

REGISTER_FUNCTION(isalnum, ValueType::Bool, {Argument(ValueType::String, "char")}) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::isalnum(value[0]));
  }
  return Value_T::UNDEFINED;
}
REGISTER_FUNCTION(isdigit, ValueType::Bool, {Argument(ValueType::String, "char")}) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::isdigit(value[0]));
  }
  return Value_T::UNDEFINED;
}
REGISTER_FUNCTION(ispunct, ValueType::Bool, {Argument(ValueType::String, "char")}) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::ispunct(value[0]));
  }
  return Value_T::UNDEFINED;
}
REGISTER_FUNCTION(isalpha, ValueType::Bool, {Argument(ValueType::String, "char")}) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::isalpha(value[0]));
  }
  return Value_T::UNDEFINED;
}


REGISTER_FUNCTION(readln, ValueType::String, {}) {
  string input;
  std::cin >> input;
  return String_T::New(input);
}





REGISTER_FUNCTION(readch, ValueType::String, {}) {
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


REGISTER_FUNCTION(push, ValueType::Undefined, Argument(ValueType::Array, "target"), Argument(ValueType::Any, "elementsToAdd")) {
  if (args.empty()) {
    return Value_T::UNDEFINED;
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
    return Value_T::UNDEFINED;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  for (size_t i = 1; i < args.size(); i++) {
    array->Push(args[i]);
  }
  return Value_T::UNDEFINED;
}
REGISTER_FUNCTION(pop, ValueType::Any, {Argument(ValueType::Array, "target")}) {
  if (args.empty()) {
    return Value_T::UNDEFINED;
  }
  
  if (args[0]->GetType() == ValueType::String) {
    auto str_value = static_cast<String_T*>(args[0].get());
    string character = std::string(1, str_value->value.back());
    str_value->value.pop_back();    
    return Ctx::CreateString(character);
  }
  
  if (args[0]->GetType() != ValueType::Array) {
    return Value_T::UNDEFINED;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  return array->Pop();
}

REGISTER_FUNCTION(len, ValueType::Int, {Argument(ValueType::Array, "target")}) {
  if (args.empty()) {
    return Value_T::UNDEFINED;
  }
  
  string result;
  if (Ctx::TryGetString(args[0], result)) {
    return Int_T::New(result.length());
  }
  
  Array array;
  if (Ctx::TryGetArray(args[0], array)) {
    return Int_T::New(array->values.size());
  }
  
  return Value_T::UNDEFINED;
}

REGISTER_FUNCTION(typeof, ValueType::String, {Argument(ValueType::Any, "target")}) {
  if (args.empty()) {
    return Value_T::UNDEFINED;
  }
  string typeName;
  return Ctx::CreateString(TypeToString(args[0]->GetType()));
}

REGISTER_FUNCTION(tostr, ValueType::String, {Argument(ValueType::Any, "target")}) {
  if (args.empty()) {
    return Value_T::UNDEFINED;
  }
  return Ctx::CreateString(args[0]->ToString());
}

REGISTER_FUNCTION(serialize, ValueType::String, Argument(ValueType::Any, "target"), Argument(ValueType::Object, "settings")) {
  if (args.empty()) {
    return Value_T::UNDEFINED;
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
