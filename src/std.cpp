#include "native.hpp"
#include "value.hpp"
#include "serializer.hpp"
#include <iostream>

REGISTER_FUNCTION(println) {
  for (const auto &arg: args) {
    printf("%s\n", arg->ToString().c_str());;
  }
  return Value_T::Undefined;
}

REGISTER_FUNCTION(readln) {
  string input;
  std::cin >> input;
  return String_T::New(input);
}

REGISTER_FUNCTION(push) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  if (args[0]->GetType() != ValueType::Array) {
    return Value_T::Undefined;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  for (size_t i = 1; i < args.size(); i++) {
    array->Push(args[i]);
  }
  return Value_T::Undefined;
}

REGISTER_FUNCTION(pop) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  if (args[0]->GetType() != ValueType::Array) {
    return Value_T::Undefined;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  return array->Pop();
}

REGISTER_FUNCTION(len) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  if (args[0]->GetType() != ValueType::Array) {
    return Value_T::Undefined;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  return Int_T::New(array->values.size());
}

REGISTER_FUNCTION(typeof) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  string typeName;
  switch(args[0]->GetType()) {
  case ValueType::Invalid:
    typeName = "invalid";
    break;
  case ValueType::Null:
    typeName = "null";
    break;
  case ValueType::Undefined:
    typeName = "undefined";
    break;
  case ValueType::Float:
    typeName = "float";
    break;
  case ValueType::Int:
    typeName = "int";
    break;
  case ValueType::Bool:
    typeName = "bool";
    break;
  case ValueType::String:
    typeName = "string";
    break;
  case ValueType::Object:
    typeName = "object";
    break;
  case ValueType::Array:
    typeName = "array";
    break;
  case ValueType::Callable:
    typeName = "callable";
    break;
  }
  return Ctx::CreateString(typeName);
}

REGISTER_FUNCTION(tostr) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  return Ctx::CreateString(args[0]->ToString());
}

REGISTER_FUNCTION(serialize) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  
  auto val = args[0];
  WriterSettings settings = {};
  Object settingsObj;
  if (args.size() > 1 && Ctx::TryGetObject(args[1], settingsObj)) {
    int indentation = 0;
    int startingIndent;
    string refHandling;
    if (Ctx::TryGetInt(settingsObj->GetMember("indentationSize"), indentation)) {
      settings.IndentSize = indentation;
    }
    if (Ctx::TryGetInt(settingsObj->GetMember("startingIndentLevel"), startingIndent)) {
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
