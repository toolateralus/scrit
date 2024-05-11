#include "native.hpp"
#include "value.hpp"
#include "serializer.hpp"
#include <iostream>
#include "context.hpp"

#pragma clang diagnostic ignored "-Wunused-parameter"


REGISTER_FUNCTION(println, "undefined", CreateArgumentSignature({Argument("varargs", "any")})) {
  for (const auto &arg: args) {
    printf("%s\n", arg->ToString().c_str());;
  }
  return Value_T::Undefined;
}

REGISTER_FUNCTION(readln, "string", CreateArgumentSignature({Argument("none", "none")})) {
  string input;
  std::cin >> input;
  return String_T::New(input);
}

REGISTER_FUNCTION(push, "undefined", CreateArgumentSignature({Argument("array", "target"), Argument("varargs", "elementsToAdd")})) {
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
REGISTER_FUNCTION(pop, "elementType", CreateArgumentSignature({Argument("array", "target")})) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  if (args[0]->GetType() != ValueType::Array) {
    return Value_T::Undefined;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  return array->Pop();
}

REGISTER_FUNCTION(len, "int", CreateArgumentSignature({Argument("array|string", "target")})) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  if (args[0]->GetType() != ValueType::Array) {
    return Value_T::Undefined;
  }
  auto array = static_cast<Array_T*>(args[0].get());
  return Int_T::New(array->values.size());
}

REGISTER_FUNCTION(typeof, "string", CreateArgumentSignature({Argument("value", "target")})) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  string typeName;
  return Ctx::CreateString(TypeToString(args[0]->GetType()));
}

REGISTER_FUNCTION(tostr, "string", CreateArgumentSignature({Argument("value", "target")})) {
  if (args.empty()) {
    return Value_T::Undefined;
  }
  return Ctx::CreateString(args[0]->ToString());
}

REGISTER_FUNCTION(serialize, "string", CreateArgumentSignature({Argument("value", "target"), Argument("settings", "object")})) {
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
