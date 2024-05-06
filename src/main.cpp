#include "lexer.hpp"
#include <cassert>
#include <iostream>

void test_token();

int main() { test_token(); }

void test_token() {
  Lexer lexer = {};
  string code = "i = 0 + 2 - 3 && 4 / v {} () \"string\" 0.0 func for { return 0 }";
  auto tokens = lexer.Lex(code);
  std::cout << "lexed " << tokens.size() << " tokens\n";
  
  assert(tokens.size() == 23);
  assert(tokens[0].type == TType::Identifier && tokens[0].value == "i");
  assert(tokens[1].type == TType::Assign && tokens[1].value == "=");
  assert(tokens[2].type == TType::Int && tokens[2].value == "0");
  assert(tokens[3].type == TType::Add && tokens[3].value == "+");
  assert(tokens[4].type == TType::Int && tokens[4].value == "2");
  assert(tokens[5].type == TType::Sub && tokens[5].value == "-");
  assert(tokens[6].type == TType::Int && tokens[6].value == "3");
  assert(tokens[7].type == TType::And && tokens[7].value == "&&");
  assert(tokens[8].type == TType::Int && tokens[8].value == "4");
  assert(tokens[9].type == TType::Div && tokens[9].value == "/");
  assert(tokens[10].type == TType::Identifier && tokens[10].value == "v");
  assert(tokens[11].type == TType::LCurly && tokens[11].value == "{");
  assert(tokens[12].type == TType::RCurly && tokens[12].value == "}");
  assert(tokens[13].type == TType::LParen && tokens[13].value == "(");
  assert(tokens[14].type == TType::RParen && tokens[14].value == ")");
  assert(tokens[15].type == TType::String && tokens[15].value == "string");
  assert(tokens[16].type == TType::Float && tokens[16].value == "0.0");
  assert(tokens[17].type == TType::Func && tokens[17].value == "func");
  assert(tokens[18].type == TType::For && tokens[18].value == "for");
  assert(tokens[19].type == TType::LCurly && tokens[19].value == "{");
  assert(tokens[20].type == TType::Return && tokens[20].value == "return");
  assert(tokens[21].type == TType::Int && tokens[21].value == "0");
  assert(tokens[22].type == TType::RCurly && tokens[22].value == "}");
}