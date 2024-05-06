#pragma once

#include <cctype>
#include <ostream>
#include <sstream>
#include <stdexcept>
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

  Not,

  Add,
  Sub,
  Mul,
  Div,
  Or,
  And,
  Greater,
  Less,
  GreaterEQ,
  LessEQ,
  Equals,

  Assign,
  
  Func,
  
  For,
  If,
  Else,
  
  Break,
  Continue,
  Return,
};

static string TTypeToString(const TType &type) {
  switch (type) {
    case TType::Identifier:
      return "Identifier";
    case TType::Int:
      return "Int";
    case TType::Float:
      return "Float";
    case TType::Bool:
      return "Bool";
    case TType::String:
      return "String";
    case TType::LParen:
      return "LParen";
    case TType::RParen:
      return "RParen";
    case TType::LCurly:
      return "LCurly";
    case TType::RCurly:
      return "RCurly";
    case TType::Not:
      return "Not";
    case TType::Add:
      return "Add";
    case TType::Sub:
      return "Sub";
    case TType::Mul:
      return "Mul";
    case TType::Div:
      return "Div";
    case TType::Or:
      return "Or";
    case TType::And:
      return "And";
    case TType::Greater:
      return "Greater";
    case TType::Less:
      return "Less";
    case TType::GreaterEQ:
      return "GreaterEQ";
    case TType::LessEQ:
      return "LessEQ";
    case TType::Equals:
      return "Equals";
    case TType::Assign:
      return "Assign";
    case TType::Func:
      return "Func";
    case TType::For:
      return "For";
    case TType::If:
      return "If";
    case TType::Else:
      return "Else";
    case TType::Break:
      return "Break";
    case TType::Continue:
      return "Continue";
    case TType::Return:
      return "Return";
    default:
      return "Unknown";
  }
}

static string TFamilyToString(const TFamily &family) {
  switch (family) {
    case TFamily::Operator:
      return "Operator";
    case TFamily::Literal:
      return "Literal";
    case TFamily::Keyword:
      return "Keyword";
    case TFamily::Identifier:
      return "Identifier";
    default:
      return "Unknown";
  }
}




struct Token {
  Token(const int &loc, const int &col, const string &value, const TType type,
        const TFamily family)
      : loc(loc), col(col), value(std::move(value)), type(type),
        family(family) {}
  string value;
  int loc = 0, col = 0;
  TType type;
  TFamily family;
  
  string ToString() const {
    stringstream stream = {};
    stream << "Token(" << value << ") type::" << TTypeToString(type) << " family::" << TFamilyToString(family) << "\n";
    return stream.str();
  }
};

static string TokensToString(const vector<Token> &tokens) {
  stringstream stream = {};
  for (const auto &token : tokens) {
    stream << token.ToString();
  }
  return stream.str();
}

struct Lexer {
  int pos;
  int loc;
  int col;
  string input;
  
  std::unordered_map<string, TType> operators = {
      {"+", TType::Add},
      {"-", TType::Sub},
      {"*", TType::Mul},
      {"/", TType::Div},
      {"||", TType::Or},
      {"&&", TType::And},
      {">", TType::Greater},
      {"<", TType::Less},
      {">=", TType::GreaterEQ},
      {"<=", TType::LessEQ},
      {"==", TType::Equals},

      {"!", TType::Not},
      // punctuation
      {"(", TType::LParen},
      {")", TType::RParen},

      {"{", TType::LCurly},
      {"}", TType::RCurly},

      {"=", TType::Assign},
  };
  std::unordered_map<string, TType> keywords{
    {"func", TType::Func},
  };
  vector<Token> Lex(const string &input) {
    this->input = input;  
    vector<Token> tokens = {};
    char cur = input[pos];
    
    while (pos < input.length()) {
      if (cur == '\n') {
        pos++;
        loc++;
        col = 0;
        continue;
      }

      if (cur == '\t') {
        pos++;
        col++;
        continue;
      }

      if (cur == ' ') {
        pos++;
        col++;
        continue;
      }

      if (cur == '\"') {
        tokens.push_back(LexString());
      } else if (isdigit(cur)) {
        tokens.push_back(LexNum());
      } else if (isalpha(cur)) {
        tokens.push_back(LexIden());
      } else if (ispunct(cur)) {
        tokens.push_back(LexOp());
      }

      pos++;
      col++;
      cur = input[pos];
    }
    
    return tokens;
  }

  Token LexIden() {
    stringstream stream = std::stringstream{};
    int startLoc = loc;
    int startCol = col;
    
    while (isalnum(input[pos])) {
      stream << input[pos];
      pos++;
      col++;
    }
    
    auto value = stream.str();
    
    if (keywords.count(value) > 0) {
      return Token(startLoc, startCol, value, keywords[value], TFamily::Keyword);
    } else {
      return Token(startLoc, startCol, value, TType::Identifier, TFamily::Identifier);
    }
  }

  Token LexString() {
    stringstream stream = {};
    int startLoc = loc;
    int startCol = col;
    
    pos++;
    col++;
    
    while (input[pos] != '\"') {
      stream << input[pos];
      pos++;
      col++;
    }
    
    pos++;
    col++;
    
    auto value = stream.str();
    return Token(startLoc, startCol, value, TType::String, TFamily::Literal);
  }

  Token LexNum() {
    stringstream stream = {};
    int startLoc = loc;
    int startCol = col;
    
    while (isdigit(input[pos])) {
      stream << input[pos];
      pos++;
      col++;
    }
    
    if (input[pos] == '.') {
      stream << input[pos];
      pos++;
      col++;

      while (isdigit(input[pos])) {
        stream << input[pos];
        pos++;
        col++;
      }
      
      return Token(startLoc, startCol, stream.str(), TType::Float, TFamily::Literal);
    } else {
      return Token(startLoc, startCol, stream.str(), TType::Int, TFamily::Literal);
    }
  }

  Token LexOp() {
    stringstream stream = {};
    int startLoc = loc;
    int startCol = col;
    string value;

    for (const auto& op : operators) {
      if (input.substr(pos).find(op.first) == 0) {
        value = op.first;
        stream << value;
        pos += value.length();
        col += value.length();
        return Token(startLoc, startCol, value, op.second, TFamily::Operator);
      }
    }
    
    throw std::runtime_error("failed to parse operator " + value);
  }
};
