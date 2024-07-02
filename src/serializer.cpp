#include "serializer.hpp"
#include "context.hpp"
#include "native.hpp"
#include "value.hpp"
#include <stdexcept>

// Builds a map of the references an object has,
// Only used when the writer needs to preserve references
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
// Maps an object inside of the initial value passed to BuildMap
// Should only be caled by BuildMap 
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
// Writes a value to the writer's stream based on it's settings.
// Call BuildMap once before you start calling this if you want to preserve references. 
void Writer::Write(const Value_T *val) {
  if (val == nullptr) {
    throw new std::runtime_error("nullptr contained in value passed to serializer");
  }

  auto type = val->GetPrimitiveType();
  
  if (type == PrimitiveType::Array |
      type == PrimitiveType::Object) {
    
    // hanlde found objects
    if (foundObjs.contains(val)) {
      switch (settings.ref_handling) {
      case ReferenceHandling::Remove:
        break;
      case ReferenceHandling::Mark:
        stream << "ref";
        break;
      case ReferenceHandling::Preserve:
        stream << "ref:" << references[val];
        break;
      }
      return;
    }     
    foundObjs.insert(val);

    // handle preserved references
    if (settings.ref_handling == ReferenceHandling::Preserve &&
        references.contains(val)) {
      // this marks the first reference to an object
      // only if the reference needs to be preserved
      // this is why we have to build the reference map when preserving
      stream << "<ref:" << references[val] << ">";
    }

    // if first iteration, set indent as starting indent
    if (settings.StartingIndentLevel > 0 && indentLevel == 0) {
      indentLevel = settings.StartingIndentLevel;
    }

    // handle indenting and delimiting
    string container_delimiter;
    string indent = "";
    if (settings.IndentSize > 0) {
      indentLevel += settings.IndentSize;
      indent = "\n" + string(indentLevel, ' ');
    }

    // handle iterating over elements
    // depending on if object (key and value) or array (just values)

    // just put identifier in if to check for cast
    if (const Object_T *object;
        type == PrimitiveType::Object &&
        (object = dynamic_cast<const Object_T *>(val))) {
      stream << "{";
      container_delimiter = "}";
      int i = object->scope->Members().size();
      for (const auto &[key, var] : object->scope->Members()) {
        stream << indent << '\"' << key.value << "\" : ";
        Write(var.get());
        i--;
        if (i != 0) {
          // only delimit if not last element
          stream << ", ";
        }
      }
    } else if (const Array_T *array = dynamic_cast<const Array_T *>(val)) {
      // not checking type here because already checked type is obj or array
      // in outer if and else rules out obj so must be array
      stream << "[";
      container_delimiter = "]";
      int i = array->values.size();
      for (const auto &var : array->values) {
        stream << indent;
        Write(var.get());
        i--;
        if (i != 0) {
          // only delimit if not last element
          stream << ", ";
        }
      }
    } else {
      // obviously something really went wrong if GetPrimitiveType() retuns
      // a type mismatched from what dynamic_cast thinks it is 
      throw new std::runtime_error("Value failed to cast as marked type in serializer, this is a language bug.");
    }

    // finish indenting
    if (settings.IndentSize > 0) {
      indentLevel -= settings.IndentSize;
      indent = "\n" + string(indentLevel, ' ');
    }
    
    // finish delimiting
    stream << indent << container_delimiter;

  } else if (type == PrimitiveType::String) {
    stream << '\"' << val->ToString() << '\"';
  } else {
    stream << val->ToString();
  }
}

// Serializes value and builds a string based on the passed in settings.
string Writer::ToString(const Value_T *value, Settings settings) {
  Writer writer(settings);
  
  // only build reference map if we need to, only used to preserve references 
  if (settings.ref_handling == ReferenceHandling::Preserve) {
    writer.BuildMap(value);
  }
  writer.Write(value);
  return writer.stream.str();
}