#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::stringstream;
using std::vector;

struct SourceInfo {
  static vector<SourceInfo *> &getInfos() {
    static vector<SourceInfo *> all_info;
    return all_info;
  }
  SourceInfo(const int loc, const int col, const std::string &source) : loc(loc), col(col), source(source) {
    getInfos().push_back(this);
  }
  std::string ToString() const {
    return string("{\n   ") + "line: " + std::to_string(loc) +
           "\n   col: " + std::to_string(col) + "\n   source: "+ source + "\n}\n";
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
  SubscriptLeft,
  SubscriptRight,
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
  Arrow
};
struct Token {
  Token(const int &loc, const int &col, const string &value, const TType type,
        const TFamily family);
  SourceInfo info;
  string value;
  int loc = 0, col = 0;
  TType type;
  TFamily family;

  string ToString() const;
};
struct Lexer {
  size_t pos = 0;
  size_t loc = 0;
  size_t col = 0;
  string input;
  std::unordered_map<string, TType> operators;
  std::unordered_map<string, TType> keywords;
  Lexer();
  vector<Token> Lex(const string &input);
  Token LexIden();
  Token LexString();
  Token LexNum();
  Token LexOp();
};
string TokensToString(const vector<Token> &tokens);
string TTypeToString(const TType &type);
string TFamilyToString(const TFamily &family);
bool IsCompoundAssignmentOperator(const TType &type);

