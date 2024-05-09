#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::stringstream;
using std::vector;

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
  
  Increment,
  Decrement,
  
  Assign,
  Comma,
  
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
  
  Break,
  Continue,
  Return,
  Import,
  From
};
struct Token {
  Token(const int &loc, const int &col, const string &value, const TType type,
        const TFamily family);
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