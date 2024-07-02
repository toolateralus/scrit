#include "serializer.hpp"
#include "context.hpp"
#include "native.hpp"
#include "value.hpp"
#include <iostream>

void Writer::BuildMap(const Value_T *value) {
  foundObjs.clear();
  references.clear();
  switch (value->GetPrimitiveType()) {
  case PrimitiveType::Object:
    Map(static_cast<const Object_T *>(value));
    break;
  case PrimitiveType::Array:
    Map(static_cast<const Array_T *>(value));
    break;
  default:
    break;
  }
  foundObjs.clear();
}
void Writer::Map(const Value_T *val) {
  if (dynamic_cast<const Object_T *>(val) ||
      dynamic_cast<const Array_T *>(val)) {
    if (foundObjs.contains(val)) {
      if (!references.contains(val)) {
        references[val] = references.size();
      }
      return;
    }
    foundObjs.insert(val);
  }
  switch (val->GetPrimitiveType()) {
  case PrimitiveType::Object: {
    auto obj = static_cast<const Object_T *>(val);
    for (const auto &[key, var] : obj->scope->Members()) {
      Map(var.get());
    }
    break;
  }
  case PrimitiveType::Array: {
    auto array = static_cast<const Array_T *>(val);
    for (const auto &value : array->values) {
      Map(value.get());
    }
    break;
  }
  default:
    break;
  }
}
void Writer::HandleRefs(const string &element_delimter, Value_T *&value, const string &key) {
  
  // for getting a space away from iden when it exists, but
  // don't want that one space indentation unless it does exist.
  string ref_key = key.empty() ? "$ref<" : indent + "$ref<";
  
  switch (settings.ref_handling) {
    case ReferenceHandling::Remove:
      break;
    case ReferenceHandling::Mark:
      stream << indent << key << ref_key << foundObjs.size() << ">\n";
      break;
    case ReferenceHandling::Preserve: {
      if (references.contains(value)) 
        stream << indent << key << ref_key << references[value] << ">\n";
    }
  }
}

void Writer::WriteObject(const Object_T *obj) {
  
  const static string container_delimiter_front = "{";
  const static string container_delimiter  = "}";
  const static string element_delimter = ", ";
  
  stream << newline << indent << container_delimiter_front << newline;
  
  int i = 0;
  const size_t size = obj->scope->Members().size();
  
  // do the indented writing
  {
    const auto _ = Indenter(this);
    for (const auto &[key, var] : obj->scope->Members()) {
      auto value = var.get();
      
      if (foundObjs.contains(value)) {
        HandleRefs(element_delimter, value, '\"' + key.value + '\"');
        continue;
      }
      
      stream << indent << '\"' << key.value << "\" : ";
      
      // try write.
      Write(value);
      
      i++;
      if (i != size - 1) {
        stream << element_delimter << newline;
      }
    }
  }
  
  stream << newline << indent  << container_delimiter;
}

void Writer::WriteArray(const Array_T *array) {
  const static string container_delimiter_front = "[";
  const static string element_delimter = ", ";
  const static string container_delimiter = "]";
  
  stream << newline << indent << container_delimiter_front << newline;
  
  // do the indented writing 
  {
    const auto _ = Indenter(this);
    const size_t size = array->values.size();
    
    int i = 0;
    for (const auto &var : array->values) {
      
      auto value = var.get();
      
      if (foundObjs.contains(value)) {
        HandleRefs(element_delimter, value);
        continue;
      }
      
      // try write.
      Write(value);
      
      // only append the element_delimiter if we're not on
      // the last element of the list.
      ++i;
      if (i != size - 1) {
        stream << element_delimter << newline;
      }
    }
  }
  
  stream << newline << container_delimiter;
}

void Writer::Write(const Value_T *val) {
  auto object = dynamic_cast<const Object_T *>(val);
  auto array = dynamic_cast<const Array_T *>(val);
  
  if (object || array) {
    foundObjs.insert(val);
  }
  
  if (val == nullptr) {
    std::cout << "writer failed.. value was nullptr." << std::endl;
    return;
  }
  
  auto _ = Writer::Indenter(this);
  switch (val->GetPrimitiveType()) {
    case PrimitiveType::Object: {
      WriteObject(object);
      break;
    }
    case PrimitiveType::Array: {
      WriteArray(array);
      break;
    }
    default: {
      stream << val->ToString();
      break;
    }
  }
};

string Writer::ToString(const Value_T *value, Settings settings) {
  Writer writer(settings);
  
  if (settings.ref_handling == ReferenceHandling::Preserve) {
    writer.BuildMap(value);
  }
  writer.Write(value);
  return writer.stream.str();
}
Writer::Indenter::~Indenter() {
  auto &settings = writer->settings;
  if (settings.IndentSize > 0) {
    writer->indentLevel -= settings.IndentSize;
    writer->indent = string(writer->indentLevel, ' ');
  }
}
Writer::Indenter::Indenter(Writer *writer) : writer(writer) {
  auto &settings = writer->settings;
  if (settings.StartingIndentLevel > 0 && writer->indentLevel == 0) {
    writer->indentLevel = settings.StartingIndentLevel;
  }
  if (settings.IndentSize > 0) {
    writer->indentLevel += settings.IndentSize;
    writer->indent = string(writer->indentLevel, ' ');
  }
}
