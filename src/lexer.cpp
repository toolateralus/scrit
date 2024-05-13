#include "lexer.hpp"

#include <algorithm>
#include <cctype>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

Token::Token(const int &loc, const int &col, const string &value,
             const TType type, const TFamily family) {
  this->loc = loc;
  this->col = col;
  this->value = std::move(value);
  this->type = type;
  this->family = family;
  this->info = {loc, col};
}
string Token::ToString() const {
  stringstream stream = {};
  stream << "Token(" << value << ") type::" << TTypeToString(type)
         << " family::" << TFamilyToString(family) << "\n";
  return stream.str();
}
vector<Token> Lexer::Lex(const string &input) {
  this->input = input;
  vector<Token> tokens = {};
  char cur = input[pos];

  while (pos < input.length()) {
    cur = input[pos];
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
    } else if (isalpha(cur) || cur == '_') {
      tokens.push_back(LexIden());
    } else if (ispunct(cur)) {
      tokens.push_back(LexOp());
    } else {
      throw std::runtime_error(string("failed to parse character ") + cur);
    }
  }

  return tokens;
}
Token Lexer::LexIden() {
  stringstream stream = std::stringstream{};
  int startLoc = loc;
  int startCol = col;

  while (isalnum(input[pos]) || input[pos] == '_') {
    stream << input[pos];
    pos++;
    col++;
  }

  auto value = stream.str();

  if (keywords.count(value) > 0) {
    return Token(startLoc, startCol, value, keywords[value], TFamily::Keyword);
  } else {
    return Token(startLoc, startCol, value, TType::Identifier,
                 TFamily::Identifier);
  }
}
Token Lexer::LexString() {
  stringstream stream = {};
  int startLoc = loc;
  int startCol = col;

  pos++; // move past opening "
  col++;
  while (true) {
    if (pos >= input.length()) {
      throw std::runtime_error("Unescaped string literal,"
                               " ln:" +
                               std::to_string(startLoc) +
                               " col:" + std::to_string(startCol));
    }
    if (!(input[pos] != '\"' ||
        (input[pos] == '\"' && input[pos - 1] == '\\'))) {
      break;
    }

    if (input[pos] == '\\' && pos + 1 < input.size()) {
      switch (input[pos + 1]) {
      case '\"':
      stream << '\"';
      pos++;
      break;
      case 'n':
      stream << '\n';
      pos++;
      break;
      case 't': 
      stream << "\t";
      pos++;
      break;
      case 'b': {
      stream << "\b";
      pos++;
      break;
      }
      case 'r': {
      stream << "\r";
      pos++;
      break;
      }
      case 'f': {
      stream << "\f";
      pos++;
      break;
      }
      case '\'': {
      stream << "\'";
      pos++;
      break;
      }
      case '\\': {
      stream << "\\";
      pos++;
      break;
      }
      default:
      stream << '\\';
      break;
      }
    } else {
      stream << input[pos];
    }
    pos++;
    col++;
  }

  pos++; // move past closing "
  col++;

  auto value = stream.str();
  return Token(startLoc, startCol, value, TType::String, TFamily::Literal);
}
Token Lexer::LexNum() {
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

    return Token(startLoc, startCol, stream.str(), TType::Float,
                 TFamily::Literal);
  } else {
    return Token(startLoc, startCol, stream.str(), TType::Int,
                 TFamily::Literal);
  }
}
Token Lexer::LexOp() {
  stringstream stream = {};
  int startLoc = loc;
  int startCol = col;
  std::vector<std::pair<string, TType>> sorted_operators(operators.begin(),
                                                         operators.end());
  std::sort(sorted_operators.begin(), sorted_operators.end(),
            [](const auto &a, const auto &b) {
              return a.first.size() > b.first.size();
            });
  for (const auto &op : sorted_operators) {
    if (input.substr(pos, op.first.size()) == op.first) {
      stream << op.first;
      pos += op.first.length();
      col += op.first.length();
      return Token(startLoc, startCol, op.first, op.second, TFamily::Operator);
    }
  }
  auto ch = std::string() + input[startLoc];
  throw std::runtime_error("failed to parse operator " + ch);
}
Lexer::Lexer() {
  operators = {
      {"+", TType::Add},
      {"-", TType::Sub},
      {"*", TType::Mul},
      {"/", TType::Div},
      {"||", TType::Or},
      {"&&", TType::And},
      {">", TType::Greater},
      {"<", TType::Less},
      {">=", TType::GreaterEq},
      {"<=", TType::LessEq},
      {"+=", TType::AddEq},
      {"-=", TType::SubEq},
      {"*=", TType::MulEq},
      {"/=", TType::DivEq},
      {"++", TType::Increment},
      {"--", TType::Decrement},
      {"==", TType::Equals},
      {"!=", TType::NotEquals},
      {".", TType::Dot},
      {"!", TType::Not},
      // punctuation
      {"(", TType::LParen},
      {")", TType::RParen},

      {"{", TType::LCurly},
      {"}", TType::RCurly},
      
      {"[", TType::SubscriptLeft},
      {"]", TType::SubscriptRight},
      {",", TType::Comma},
      {":", TType::Colon},
      {"=", TType::Assign},
      {"??", TType::NullCoalescing},
      // these are escpaed because theyre trigraphs
      {"\?\?=", TType::NullCoalescingEq}
  };
  keywords = {
      {"func", TType::Func},           {"for", TType::For},
      {"continue", TType::Continue},   {"break", TType::Break},
      {"return", TType::Return},       {"if", TType::If},
      {"else", TType::Else},           {"false", TType::False},
      {"true", TType::True},           {"null", TType::Null},
      {"undefined", TType::Undefined}, {"import", TType::Import},
      {"from", TType::From}, {"use", TType::Use},
  };
  loc = 1;
}
string TTypeToString(const TType &type) {
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
  case TType::GreaterEq:
    return "GreaterEq";
  case TType::LessEq:
    return "LessEq";
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
  case TType::Use:
    return "Use";
  case TType::AddEq:
    return "AddEq";
  case TType::SubEq:
    return "SubEq";
  case TType::MulEq:
    return "MulEq";
  case TType::DivEq:
    return "DivEq";
  case TType::Increment:
    return "Increment";
  case TType::Decrement:
    return "Decrement";
  case TType::NotEquals:
    return "NotEquals";
  case TType::Dot:
    return "Dot";
  case TType::SubscriptLeft:
    return "SubscriptLeft";
  case TType::SubscriptRight:
    return "SubscriptRight";
  case TType::Comma:
    return "Comma";
  case TType::Colon:
    return "Colon";
  case TType::NullCoalescing:
    return "NullCoalescing";
  case TType::NullCoalescingEq:
    return "NullCoalescingEq";
  case TType::False:
    return "False";
  case TType::True:
    return "True";
  case TType::Null:
    return "Null";
  case TType::Undefined:
    return "Undefined";
  case TType::Import:
    return "Import";
  case TType::From:
    return "From";
  default:
    return "Unknown";
  }
}
string TFamilyToString(const TFamily &family) {
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
string TokensToString(const vector<Token> &tokens) {
  stringstream stream = {};
  for (const auto &token : tokens) {
    stream << token.ToString();
  }
  return stream.str();
}
bool IsCompoundAssignmentOperator(const TType &type) {
  return type == TType::AddEq || type == TType::SubEq || type == TType::MulEq ||
         type == TType::DivEq || type == TType::NullCoalescingEq;
}

