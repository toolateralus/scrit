#include "ast.hpp"
#include "context.hpp"
#include "lexer.hpp"
#include "value.hpp"
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
unique_ptr<Assignment> GetStrAssign(string id, string value) {
  return make_unique<Assignment>(info, GetIdentifier(id),
                                 GetString(value));
}
unique_ptr<Block> GetBlock(string id, string stringValueSetInBlock) {
  vector<StatementPtr> statements = {};
  statements.push_back(GetPrintlnCall());
  statements.push_back(GetStrAssign(id, stringValueSetInBlock));
  return make_unique<Block>(info, std::move(statements));
}
ElsePtr GetElse(string id, string stringValueSetInBlock) {
  ElsePtr elseptr = Else::NoIf(info, GetBlock(id, stringValueSetInBlock));
  return elseptr;
}
IfPtr GetIfTrue(string id, string stringValueSetInBlock) {
  return If::NoElse(info, GetTrueExpr(), GetBlock(id, stringValueSetInBlock));
}
IfPtr GetIfFalse(string id, string stringValueSetInBlock) {
  return If::NoElse(info, GetFalseExpr(), GetBlock(id, stringValueSetInBlock));
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
StatementPtr GetFloatAssign(string iden, float value) {
  auto id = GetIdentifier(iden);
  auto zero = Float_T::New(value);
  return make_unique<Assignment>(info, std::move(id), std::make_unique<Operand>(info, zero));
}
StatementPtr GetBoolAssign(string iden, bool value) {
  auto id = GetIdentifier(iden);
  auto zero = Bool_T::New(value);
  return make_unique<Assignment>(info, std::move(id), std::make_unique<Operand>(info, zero));
}
StatementPtr GetArrayAssign(string iden, vector<Value> values = {}) {
  auto id = GetIdentifier(iden);
  auto zero = Array_T::New(values);
  return make_unique<Assignment>(info, std::move(id), std::make_unique<Operand>(info, zero));
}
StatementPtr GetObjectAssign(string iden, BlockPtr &&block) {
  auto id = GetIdentifier(iden);
  auto init = make_unique<ObjectInitializer>(info, std::move(block), make_shared<Scope_T>());
  return make_unique<Assignment>(info, std::move(id), std::move(init));
}
ExpressionPtr GetLessThanExpr(string iden, int rvalue) {
  return make_unique<BinExpr>(info, GetIdentifier(iden), std::make_unique<Operand>(info, Int_T::New(rvalue)), TType::Less);
}
StatementPtr GetIncrement(string i) {
  return make_unique<UnaryStatement>(info, make_unique<UnaryExpr>(info, GetIdentifier(i), TType::Increment));
}

TEST(ASSIGN, ASSIGN_STR) {
  ASTNode::context.Reset();
  GetStrAssign("myvalue", "myDefaultValue")->Execute();
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(
      String_T::New("myDefaultValue")));
}
TEST(ASSIGN, ASSIGN_INT) {
  ASTNode::context.Reset();
  GetIntAssign("myvalue", 100)->Execute();
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(
      Int_T::New(100)));
}
TEST(ASSIGN, ASSIGN_FLOAT) {
  ASTNode::context.Reset();
  GetFloatAssign("myvalue", 100.0f)->Execute();
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(
      Float_T::New(100.0f)));
}
TEST(ASSIGN, ASSIGN_ARRAY) {
  ASTNode::context.Reset();
  auto result = GetArrayAssign("myArray", {Int_T::New(0), Int_T::New(1)})->Execute();
  ASSERT_EQ(result.controlChange, ControlChange::None);
  auto array = ASTNode::context.Find("myArray");
  ASSERT_NE(array, nullptr);
  auto arr = static_cast<Array_T*>(array.get());
  ASSERT_NE(arr, nullptr);
  
  auto element0 = arr->At(Int_T::New(0));
  ASSERT_TRUE(element0->Equals(Int_T::New(0)));
  auto element1 = arr->At(Int_T::New(1));
  ASSERT_TRUE(element1->Equals(Int_T::New(1)));
}
TEST(ASSIGN, ASSIGN_ARRAY_ELEMENT) {
  ASTNode::context.Reset();
  auto result = GetArrayAssign("myArray", {Int_T::New(0), Int_T::New(1)})->Execute();
  ASSERT_EQ(result.controlChange, ControlChange::None);
  auto array = ASTNode::context.Find("myArray");
  ASSERT_NE(array, nullptr);
  auto arr = static_cast<Array_T*>(array.get());
  ASSERT_NE(arr, nullptr);
  
  auto element0 = arr->At(Int_T::New(0));
  ASSERT_TRUE(element0->Equals(Int_T::New(0)));
  auto element1 = arr->At(Int_T::New(1));
  ASSERT_TRUE(element1->Equals(Int_T::New(1)));
  
  arr->Assign(Int_T::New(0), Int_T::New(20));
  element0 = arr->At(Int_T::New(0));
  ASSERT_TRUE(element0->Equals(Int_T::New(20)));
}
TEST(ASSIGN, ASSIGN_OBJECT) {
  ASTNode::context.Reset();
  auto result = GetObjectAssign("myObject", GetBlock("myInnerValue", "SomeValue"))->Execute();
  
  ASSERT_EQ(result.controlChange, ControlChange::None);
  
  auto object = static_cast<Object_T*>(ASTNode::context.Find("myObject").get());
  ASSERT_NE(object, nullptr);
  
  auto member = object->GetMember("myInnerValue");
  ASSERT_NE(member, nullptr);
  auto string = static_cast<String_T*>(member.get());
  ASSERT_NE(string, nullptr);
  ASSERT_TRUE(string->Equals(String_T::New("SomeValue")));
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
  GetStrAssign("myvalue", "myDefaultValue")->Execute();
  string value = "VALUE_SET_IN_BLOCK";
  auto ifStmnt = GetIfTrue("myvalue", value);
  
  ExecutionResult result = ifStmnt->Execute();
  // assert that the assignment that was in the GetBlock overrides the
  // assignment of myDefaultValue took place.
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(
      String_T::New("VALUE_SET_IN_BLOCK")));
  ASSERT_TRUE(result.controlChange == ControlChange::None);
}
TEST(CONTROL_FLOW, TEST_IF_WITH_ELSE) {
  ASTNode::context.Reset();
  GetStrAssign("myvalue", "myDefaultValue")->Execute();
  auto if_with_else =
      If::WithElse(info, GetFalseExpr(), GetBlock("myvalue", "VALUE_SET_IN_IF_BLOCK"),
                   GetElse("myvalue", "VALUE_SET_IN_ELSE_BLOCK"));
  if_with_else->Execute();
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(
      String_T::New("VALUE_SET_IN_ELSE_BLOCK")));
}
TEST(CONTROL_FLOW, TEST_IF_FALSE_ELSE_IF_TRUE_ELSE) {
  ASTNode::context.Reset();
  GetStrAssign("myvalue", "myDefaultValue")->Execute();
  
  
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
      info, GetFalseExpr(), GetBlock("myvalue", "VALUE_SET_IN_IF"),
      Else::New(
          info,
          If::WithElse(info, GetTrueExpr(), GetBlock("myvalue", "VALUE_SET_IN_ELSE_IF"),
                       Else::NoIf(info, GetBlock("myvalue", "VALUE_SET_IN_FINAL_ELSE")))));
                       
  if_with_else_if->Execute();                       
                       
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(String_T::New("VALUE_SET_IN_ELSE_IF")));
}
TEST(CONTROL_FLOW, TEST_IF_FALSE_ELSE_IF_FALSE_ELSE) {
  ASTNode::context.Reset();
  GetStrAssign("myvalue", "myDefaultValue")->Execute();
  
  
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
      info, GetFalseExpr(), GetBlock("myvalue", "VALUE_SET_IN_IF"),
      Else::New(
          info,
          If::WithElse(info, GetFalseExpr(), GetBlock("myvalue", "VALUE_SET_IN_ELSE_IF"),
                       Else::NoIf(info, GetBlock("myvalue", "VALUE_SET_IN_FINAL_ELSE")))));
                       
  if_with_else_if->Execute();                       
                       
  ASSERT_TRUE(ASTNode::context.Find("myvalue")->Equals(String_T::New("VALUE_SET_IN_FINAL_ELSE")));
}
