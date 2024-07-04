#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using std::string;
using std::stringstream;
using std::vector;

struct SourceInfo final {
  SourceInfo() = delete;
  SourceInfo(const SourceInfo &) = default;
  SourceInfo(SourceInfo &&) = default;
  SourceInfo &operator=(const SourceInfo &) = default;
  SourceInfo &operator=(SourceInfo &&) = default;
  ~SourceInfo() {}

  explicit SourceInfo(const int loc, const int col, const std::string &source)
      : loc(loc), col(col), source(source) {}

  std::string ToString() const noexcept {
    return string("{\n   ") + "line: " + std::to_string(loc) +
           "\n   col: " + std::to_string(col) + "\n   source: " + source +
           "\n}\n";
  }
  int loc, col;
  string source;
};

enum class TFamily {
  Operator,
  Literal,
  Keyword,
  Identifier,
};
enum class TType {
  Identifier,
  Int,
  Float,
  Bool,
  String,
  
  LParen,
  RParen,
  
  LCurly,
  RCurly,
  
  LBrace,
  RBrace,
  
  Not,

  Add,
  Sub,
  Mul,
  Div,
  Or,
  And,
  Greater,
  Less,
  GreaterEq,
  LessEq,
  Equals,
  NotEquals,

  //
  AddEq,
  SubEq,
  MulEq,
  DivEq,
  NullCoalescing,
  NullCoalescingEq,

  Increment,
  Decrement,

  Assign,
  Comma,
  Colon,

  // Keywords.
  Func,

  For,
  If,
  Else,
  Dot,
  False,
  True,
  Null,
  Undefined,
  // TODO: add try & catch.

  Match,
  // => used for implicit return and block expressions: returning a value from a
  // block opposed to creating an object.
  Lambda,
  // default keyword. used right now for match statements.
  Default,

  Const,
  Mut,

  Break,
  Continue,
  Return,
  Using,
  From,
  Import,
  Delete,
  Let,
  Arrow,
  Type,
  Struct,
  ScopeResolution,
  Namespace,
};

struct Token {
  Token() = delete;
  Token(const Token &) = default;
  Token(Token &&) = default;
  Token &operator=(const Token &) = default;
  Token &operator=(Token &&) = default;

  explicit Token(const int &loc, const int &col, const string &value,
                 const TType type, const TFamily family);
  SourceInfo info;
  string value;
  int loc = 0, col = 0;
  TType type;
  TFamily family;

  string ToString() const noexcept;
};
struct Lexer final {
  size_t pos = 0;
  size_t loc = 0;
  size_t col = 0;
  std::string_view input;
  const std::unordered_map<string, TType> operators;
  const std::unordered_map<string, TType> keywords;
  explicit Lexer();
  vector<Token> Lex(const string &input);
  Token LexIden() noexcept;
  Token LexNum() noexcept;
  Token LexString();
  Token LexOp();
};
string TokensToString(const vector<Token> &tokens);
string TTypeToString(const TType &type);
string TFamilyToString(const TFamily &family);
bool IsCompoundAssignmentOperator(const TType &type);
