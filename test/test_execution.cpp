#include "ast.hpp"
#include "context.hpp"
#include "lexer.hpp"
#include "value.hpp"
#include <cassert>
#include <gtest/gtest.h>

#include <memory>

using std::make_unique;
using std::unique_ptr;

SourceInfo info = SourceInfo{0, 0};

ExpressionPtr GetFalseExpr() {
  return make_unique<Operand>(info, Bool_T::New(false));
}
ExpressionPtr GetTrueExpr() {
  return make_unique<Operand>(info, Bool_T::New(true));
}
IdentifierPtr GetIdentifier(string name) {
  return make_unique<Identifier>(info, name);
}
OperandPtr GetString(string value) {
  return make_unique<Operand>(info, String_T::New(value));
}
unique_ptr<Call> GetPrintlnCall() {
  auto printlnId = GetIdentifier("println");
  std::vector<ExpressionPtr> argsVec = {};
  argsVec.push_back(GetString("from println"));
  ArgumentsPtr args = make_unique<Arguments>(info, std::move(argsVec));
  unique_ptr<Call> call =
      make_unique<Call>(info, std::move(printlnId), std::move(args));
  return call;
}
unique_ptr<Assignment> GetStrAssign(string value) {
  return make_unique<Assignment>(info, GetIdentifier("myvalue"),
                                 GetString(value));
}
unique_ptr<Block> GetBlock(string stringValueSetInBlock) {
  vector<StatementPtr> statements = {};
  statements.push_back(GetPrintlnCall());
  statements.push_back(GetStrAssign(stringValueSetInBlock));
  return make_unique<Block>(info, std::move(statements));
}
ElsePtr GetElse(string stringValueSetInBlock) {
  ElsePtr elseptr = Else::NoIf(info, GetBlock(stringValueSetInBlock));
  return elseptr;
}
IfPtr GetIfTrue(string stringValueSetInBlock) {
  return If::NoElse(info, GetTrueExpr(), GetBlock(stringValueSetInBlock));
}
IfPtr GetIfFalse(string stringValueSetInBlock) {
  return If::NoElse(info, GetFalseExpr(), GetBlock(stringValueSetInBlock));
}
unique_ptr<For> GetFor(StatementPtr &&decl, ExpressionPtr &&condition, StatementPtr &&increment, BlockPtr &&block) {
  return make_unique<For>(info, std::move(decl), std::move(condition), std::move(increment), std::move(block), make_shared<Scope_T>());
}
BlockPtr GetBlockBreak() {
  vector<StatementPtr> statements = {};
  statements.push_back(make_unique<Break>(info));
  return make_unique<Block>(info, std::move(statements));
}
StatementPtr GetIntAssign(string iden, int value) {
  auto id = GetIdentifier(iden);
  auto zero = Int_T::New(value);
  return make_unique<Assignment>(info, std::move(id), std::make_unique<Operand>(info, zero));
}
ExpressionPtr GetLessThanExpr(string iden, int rvalue) {
  return make_unique<BinExpr>(info, GetIdentifier(iden), std::make_unique<Operand>(info, Int_T::New(rvalue)), TType::Less);
}
StatementPtr GetIncrement(string i) {
  return make_unique<UnaryStatement>(info, make_unique<UnaryExpr>(info, GetIdentifier(i), TType::Increment));
}

TEST(ASSIGN, ASSIGN_STR) {
  ASTNode::context.Reset();
  GetStrAssign("myDefaultValue")->Execute();
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(
      String_T::New("myDefaultValue")));
}
TEST(CONTROL_FLOW, TEST_FOR_NO_CONDITION_INSTANT_BREAK) {
  auto result = GetFor(nullptr, nullptr, nullptr, GetBlockBreak())->Execute();
  ASSERT_TRUE(result.controlChange == ControlChange::None);
}
TEST(CONTROL_FLOW, TEST_FOR_TRUE) {
  auto result = GetFor(nullptr, GetTrueExpr(), nullptr, GetBlockBreak())->Execute();
  ASSERT_TRUE(result.controlChange == ControlChange::None);
}
TEST(CONTROL_FLOW, TEST_FOR_I_LESS_THAN_10) {
  auto result = GetFor(GetIntAssign("i", 0), GetLessThanExpr("i", 10), GetIncrement("i"), GetBlockBreak())->Execute();
  ASSERT_TRUE(result.controlChange == ControlChange::None);
  ASSERT_TRUE(ASTNode::context.Find("i") == nullptr);
}


TEST(CONTROL_FLOW, TEST_IF_NO_ELSE) {
  ASTNode::context.Reset();
  GetStrAssign("myDefaultValue")->Execute();

  string value = "VALUE_SET_IN_BLOCK";
  auto ifStmnt = GetIfTrue(value);

  ExecutionResult result = ifStmnt->Execute();
  // assert that the assignment that was in the GetBlock overrides the
  // assignment of myDefaultValue took place.
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(
      String_T::New("VALUE_SET_IN_BLOCK")));
  ASSERT_TRUE(result.controlChange == ControlChange::None);
}
TEST(CONTROL_FLOW, TEST_IF_WITH_ELSE) {
  ASTNode::context.Reset();
  GetStrAssign("myDefaultValue")->Execute();
  auto if_with_else =
      If::WithElse(info, GetFalseExpr(), GetBlock("VALUE_SET_IN_IF_BLOCK"),
                   GetElse("VALUE_SET_IN_ELSE_BLOCK"));
  if_with_else->Execute();
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(
      String_T::New("VALUE_SET_IN_ELSE_BLOCK")));
}
TEST(CONTROL_FLOW, TEST_IF_FALSE_ELSE_IF_TRUE_ELSE) {
  ASTNode::context.Reset();
  GetStrAssign("myDefaultValue")->Execute();
  
  
  /*
    this represents
    
    if false {
      myvalue = "VALUE_SET_IN_IF"
    } else if true {
      myvalue = "VALUE_SET_IN_ELSE_IF"
    } else {
      myvalue = "VALUE_SET_IN_FINAL_ELSE"
    }
    
  */
  
  auto if_with_else_if = If::WithElse(
      info, GetFalseExpr(), GetBlock("VALUE_SET_IN_IF"),
      Else::New(
          info,
          If::WithElse(info, GetTrueExpr(), GetBlock("VALUE_SET_IN_ELSE_IF"),
                       Else::NoIf(info, GetBlock("VALUE_SET_IN_FINAL_ELSE")))));
                       
  if_with_else_if->Execute();                       
                       
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(String_T::New("VALUE_SET_IN_ELSE_IF")));
}
TEST(CONTROL_FLOW, TEST_IF_FALSE_ELSE_IF_FALSE_ELSE) {
  ASTNode::context.Reset();
  GetStrAssign("myDefaultValue")->Execute();
  
  
  /*
    this represents
    
    if false {
      myvalue = "VALUE_SET_IN_IF"
    } else if false {
      myvalue = "VALUE_SET_IN_ELSE_IF"
    } else {
      myvalue = "VALUE_SET_IN_FINAL_ELSE"
    }
    
  */
  
  auto if_with_else_if = If::WithElse(
      info, GetFalseExpr(), GetBlock("VALUE_SET_IN_IF"),
      Else::New(
          info,
          If::WithElse(info, GetFalseExpr(), GetBlock("VALUE_SET_IN_ELSE_IF"),
                       Else::NoIf(info, GetBlock("VALUE_SET_IN_FINAL_ELSE")))));
                       
  if_with_else_if->Execute();                       
                       
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(String_T::New("VALUE_SET_IN_FINAL_ELSE")));
}
