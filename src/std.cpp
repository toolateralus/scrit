#include "native.hpp"
#include "value.hpp"
#include <iostream>
#include "context.hpp"
#include "serializer.hpp"

#pragma clang diagnostic ignored "-Wunused-parameter"


REGISTER_FUNCTION(println, ValueType::Undefined, {}) {
  for (const auto &arg: args) {
    printf("%s\n", arg->ToString().c_str());;
  }
  return Value_T::UNDEFINED;
}

REGISTER_FUNCTION(readln, ValueType::String, {}) {
  string input;
  std::cin >> input;
  return String_T::New(input);
}

REGISTER_FUNCTION(push, ValueType::Undefined, Argument(ValueType::Array, "target"), Argument(ValueType::Any, "elementsToAdd")) {
  if (args.empty()) {
    return Value_T::UNDEFINED;
  }
  
  string value;
  if (Ctx::TryGetString(args[0], value)) {
    for (size_t i = 1; i < args.size(); i++) {
      string arg;
      if (Ctx::TryGetString(args[i], arg)) {
        value += arg;
      }
    }
    return Ctx::CreateString(value);
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
  if (args[0]->GetType() != ValueType::Array) {
    return Value_T::UNDEFINED;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  return Int_T::New(array->values.size());
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
