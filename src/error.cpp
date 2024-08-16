
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
                         "\noffending type: " + type->Name()) {}



TypeError::TypeError(const Type &type_a, const Type &type_b,
                     const std::string message)
    : std::runtime_error([message, type_a, type_b]() -> std::string {
        if (!type_a || !type_b) {
          std::string type_str = "";
          
          if (type_a) {
            type_str += type_a->Name();
          }
          if (type_b) {
            type_str += type_a ? " ," : ""  + type_b->Name();
          }
          return "Type error for types: " + type_str + '\n' + message;
        }
        return "Type Error: incompatible types.\noffending types:\n" +
               type_a->Name() + "\n" + type_b->Name() + "\n" + message;
      }()) {}
