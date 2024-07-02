
#include "error.hpp"
#include "lexer.hpp"
#include <stdexcept>
#include "type.hpp"

using namespace Values;

LexError::LexError(const SourceInfo &info)
    : std::runtime_error("LexError: " + info.ToString()) {}
    
ParseError::ParseError(const SourceInfo &info)
    : std::runtime_error("LexError: " + info.ToString()) {}
    
    
TypeError::TypeError(const Type &type, const std::string message)
    : std::runtime_error("Type Error: " + message +
                         "\noffending type: " + type->name) {}
                         
// use this immediately-invoked lambda to do some checking on the init.
TypeError::TypeError(const Type &type_a, const Type &type_b)
  : std::runtime_error([type_a, type_b]() -> std::string {
    if (!type_a || !type_b) {
      return "Type Error: One or more type arguments are null.";
    }
    return "Type Error: incompatible types.\noffending types:\n" +
         type_a->name + "\n" + type_b->name;
  }()) {}

TypeError::TypeError(const Type &type_a, const Type &type_b, const std::string message)
  : std::runtime_error([message, type_a, type_b]() -> std::string {
    if (!type_a || !type_b) {
      return "Type Error: One or more type arguments are null.." + message;
    }
    return "Type Error: incompatible types.\noffending types:\n" +
         type_a->name + "\n" + type_b->name + "\n" + message;
  }()) {}
