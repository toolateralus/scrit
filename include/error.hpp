

#pragma once
#include <memory>
#include <stdexcept>

struct SourceInfo;
struct Token;

namespace Values {
struct Type_T;
};

using Type = std::shared_ptr<Values::Type_T>;

struct LexError : std::runtime_error {
  LexError(const SourceInfo &info);
};

struct ParseError : std::runtime_error {
  ParseError(const SourceInfo &info);
};

struct TypeError : std::runtime_error {
  TypeError(const Type &type, const std::string message);
  TypeError(const Type &type_a, const Type &type_b, const std::string message);
};
