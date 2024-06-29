#include "native.hpp"
#include "serializer.hpp"
#include "value.hpp"
#include "context.hpp"

void Writer::BuildMap(const Value_T * value) {
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
  if (dynamic_cast<const Object_T*>(val) || dynamic_cast<const Array_T*>(val)) {
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
    case PrimitiveType::Array:{
      auto array = static_cast<const Array_T *>(val);
      for (const auto &value : array->values) {
        Map(value.get());
      }
      break;
    }
    default: break;
  }
}
void Writer::Write(const Value_T *val) {
  
  if (dynamic_cast<const Object_T*>(val) || dynamic_cast<const Array_T*>(val)) {
    foundObjs.insert(val);
  }
  
  string indent = "";
  if (settings.StartingIndentLevel > 0 && indentLevel == 0) {
    indentLevel = settings.StartingIndentLevel;
  }
  if (settings.IndentSize > 0) {
    indentLevel += settings.IndentSize;
    indent = "\n" + string(indentLevel, ' ');
  }
  string element_delimter = ", ";
  string container_delimiter;
  
  if (val == nullptr) {
    return;
  }
  
  switch (val->GetPrimitiveType()) {
    case PrimitiveType::Object: {
      auto obj = static_cast<const Object_T *>(val);
      stream << "{";
      container_delimiter = "}";
      int i = obj->scope->Members().size();
      for (const auto &[key, var] : obj->scope->Members()) {
        i--;
        if (i == 0) {
          element_delimter = "";
        }
        auto value = var.get();
        if ((dynamic_cast<Object_T*>(value) || dynamic_cast<Array_T*>(value)) && foundObjs.contains(value)) {
          switch (settings.ref_handling) {
          case ReferenceHandling::Remove:
            break;
          case ReferenceHandling::Mark:
            stream << indent << '\"' << key.value << "\" : ";
            stream << "ref";
            stream << element_delimter;
            break;
          case ReferenceHandling::Preserve:
            stream << indent << '\"' << key.value << "\" : ";
            stream << "ref:" << references[value];
            stream << element_delimter;
            break;
          }
          continue;
        }
        if (settings.ref_handling == ReferenceHandling::Preserve &&
            references.contains(value)) {
          stream << "<ref:" << references[value] << ">";
        }
        stream << indent << '\"' << key.value << "\" : ";
        Write(value);
        stream << element_delimter;
      }
      break;
    }
    case PrimitiveType::Array: {
      auto array = static_cast<const Array_T *>(val);
      stream << "[";
      container_delimiter = "]";
      int i = array->values.size();
      for (const auto &var : array->values) {
        i--;
        if (i == 0) {
          element_delimter = "";
        }
        auto value = var.get();
        if ((dynamic_cast<Object_T*>(value) || dynamic_cast<Array_T*>(value)) && foundObjs.contains(value)) {
          switch (settings.ref_handling) {
          case ReferenceHandling::Remove:
            break;
          case ReferenceHandling::Mark:
            stream << indent;
            stream << "ref:" << foundObjs.size();
            stream << element_delimter;
            break;
          case ReferenceHandling::Preserve:
            stream << indent;
            stream << "ref:" << references[value];
            stream << element_delimter;
            break;
          }
          continue;
        }
        if (settings.ref_handling == ReferenceHandling::Preserve &&
            references.contains(value)) {
          stream << "<ref:" << references[value] << ">";
        }
        stream << indent;
        Write(value);
        stream << element_delimter;
      }
      break;
    }
    default: {
      stream << val->ToString();
      break;
    }
  }
  if (settings.IndentSize > 0) {
    indentLevel -= settings.IndentSize;
    indent = "\n" + string(indentLevel, ' ');
  }
  stream << indent << container_delimiter;
};
string Writer::ToString(const Value_T * value, WriterSettings settings) {
  Writer writer{.settings = settings};
  if (settings.ref_handling == ReferenceHandling::Preserve) {
    writer.BuildMap(value);
  }
  writer.Write(value);
  return writer.stream.str();
}
