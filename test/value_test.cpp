#include <gtest/gtest.h>
#include "native.hpp"
#include "value.hpp"
#include "context.hpp"

using namespace Values;

TEST(EqTest, StrEquals) {
  String EMPTY = String_T::New("");
  String NOT_EMPTY = String_T::New("test string");
  String OTHER_NOT_EMPTY = String_T::New("test string");
  
  ASSERT_TRUE(EMPTY->Equals(EMPTY));
  ASSERT_TRUE(NOT_EMPTY->Equals(NOT_EMPTY));
  ASSERT_TRUE(NOT_EMPTY->Equals(OTHER_NOT_EMPTY));
  ASSERT_FALSE(EMPTY->Equals(NOT_EMPTY));
}
TEST(EqTest, UndefinedNullEquals) {
  Undefined UNDEFINED = Value_T::UNDEFINED;
  Null NULLT = Value_T::VNULL;
  ASSERT_TRUE(UNDEFINED->Equals(UNDEFINED));
  ASSERT_TRUE(NULLT->Equals(NULLT));
  ASSERT_FALSE(UNDEFINED->Equals(NULLT));
  ASSERT_FALSE(NULLT->Equals(UNDEFINED));
}
TEST(EqTest, UndefinedNullEqualsUpcasted) {
  Undefined UNDEFINED = Value_T::UNDEFINED;
  Null NULLT = Value_T::VNULL;
  Value UNDEFINED_V = UNDEFINED;
  Value NULL_V = NULLT;
  ASSERT_TRUE(UNDEFINED_V->Equals(UNDEFINED_V));
  ASSERT_TRUE(NULL_V->Equals(NULL_V));
  ASSERT_FALSE(UNDEFINED_V->Equals(NULL_V));
  ASSERT_FALSE(NULL_V->Equals(UNDEFINED_V));
}
TEST(EqTest, IntEquals) {
  Int ZERO = Int_T::New(0);
  Int ONE = Int_T::New(1);
  ASSERT_TRUE(ZERO->Equals(ZERO));
  ASSERT_TRUE(ONE->Equals(ONE));
  ASSERT_FALSE(ZERO->Equals(ONE));
  ASSERT_FALSE(ONE->Equals(ZERO));
}
TEST(EqTest, FloatEquals) {
  Float FZERO = Float_T::New(0);
  Float FONE = Float_T::New(1);
  ASSERT_TRUE(FZERO->Equals(FZERO));
  ASSERT_TRUE(FONE->Equals(FONE));
  ASSERT_FALSE(FONE->Equals(FZERO));
  ASSERT_FALSE(FZERO->Equals(FONE));
}
TEST(EqTest, BoolEquals) {
  Bool TRUE = Value_T::False;
  Bool FALSE = Value_T::True;
  ASSERT_TRUE(TRUE->Equals(TRUE));
  ASSERT_TRUE(FALSE->Equals(FALSE));
  ASSERT_FALSE(TRUE->Equals(FALSE));
  ASSERT_FALSE(FALSE->Equals(TRUE));
}
TEST(EqTest, ObjectEquals) {
  Object OBJ1 = Object_T::New();
  Object OBJ2 = Object_T::New();
  OBJ2->scope->variables["FALSE"] = Value_T::False;
  Object OTHER_OBJ2 = Object_T::New();
  OTHER_OBJ2->scope->variables["FALSE"] = Value_T::False;
  ASSERT_FALSE(OBJ1->Equals(OBJ2));
  ASSERT_FALSE(OBJ2->Equals(OTHER_OBJ2));
  ASSERT_TRUE(OBJ1->Equals(OBJ1));
}
TEST(EqTest, ArrayEquals) {
  Array ARR1 = Array_T::New();
  Array ARR2 = Array_T::New();
  ASSERT_FALSE(ARR1->Equals(ARR2));
  ASSERT_TRUE(ARR1->Equals(ARR1));
}
TEST(AddTest, StrAdd) {
  String str = Ctx::CreateString("2");
  String str1 = Ctx::CreateString("1");
  auto result = static_cast<String_T*>(str->Add(str1).get())->value;
  ASSERT_EQ("21", result);
}
TEST(AddTest, IntAdd) {
  Int i0 = Ctx::CreateInt(10); 
  Int i1 = Ctx::CreateInt(10);
  auto result = static_cast<Int_T*>(i0->Add(i1).get())->value;
  ASSERT_EQ(20, result);
}
TEST(AddTest, FloatAdd) {
  Float i0 = Ctx::CreateFloat(10.5f); 
  Float i1 = Ctx::CreateFloat(10.0f);
  auto result = static_cast<Float_T*>(i0->Add(i1).get())->value;
  ASSERT_EQ(20.5f, result);
}
TEST(SubTest, IntSub) {
  Int i0 = Ctx::CreateInt(10); 
  Int i1 = Ctx::CreateInt(10);
  auto result = static_cast<Int_T*>(i0->Subtract(i1).get())->value;
  ASSERT_EQ(0, result);
}
TEST(SubTest, FloatSub) {
  Float i0 = Ctx::CreateFloat(10.5f); 
  Float i1 = Ctx::CreateFloat(10.0f);
  auto result = static_cast<Float_T*>(i0->Subtract(i1).get())->value;
  ASSERT_EQ(0.5f, result);
}
TEST(MulTest, IntMul) {
  Int i0 = Ctx::CreateInt(10); 
  Int i1 = Ctx::CreateInt(10);
  auto result = static_cast<Int_T*>(i0->Multiply(i1).get())->value;
  ASSERT_EQ(100, result);
}
TEST(MulTest, FloatMul) {
  Float i0 = Ctx::CreateFloat(10.5f); 
  Float i1 = Ctx::CreateFloat(10.0f);
  auto result = static_cast<Float_T*>(i0->Multiply(i1).get())->value;
  ASSERT_EQ(105.0f, result);
}
TEST(DivTest, IntDiv) {
  Int i0 = Ctx::CreateInt(10); 
  Int i1 = Ctx::CreateInt(2);
  auto result = static_cast<Int_T*>(i0->Divide(i1).get())->value;
  ASSERT_EQ(5, result);
}
TEST(DivTest, FloatDiv) {
  Float i0 = Ctx::CreateFloat(10.5f); 
  Float i1 = Ctx::CreateFloat(2.0f);
  auto result = static_cast<Float_T*>(i0->Divide(i1).get())->value;
  ASSERT_EQ(5.25f, result);
}
TEST(AndTest, BoolAnd) {
  Bool b0 = Value_T::True;
  Bool b1 = Value_T::True;
  auto result = static_cast<Bool_T*>(b0->And(b1).get())->value;
  ASSERT_TRUE(b0->And(b1));
}
TEST(AndTest, IntAnd) {
  Int i0 = Ctx::CreateInt(10); 
  Int i1 = Ctx::CreateInt(5);
  auto result = static_cast<Bool_T*>(i0->And(i1).get())->value;
  ASSERT_TRUE(result);
}
TEST(AndTest, FloatAnd) {
  Float f0 = Ctx::CreateFloat(10.5f); 
  Float f1 = Ctx::CreateFloat(5.0f);
  auto result = static_cast<Bool_T*>(f0->And(f1).get())->value;
  ASSERT_TRUE(result);
}
TEST(OrTest, IntOr) {
  Int i0 = Ctx::CreateInt(10); 
  Int i1 = Ctx::CreateInt(0);
  ASSERT_TRUE(static_cast<Bool_T*>(i0->Or(i1).get())->value);
}
TEST(OrTest, FloatOr) {
  Float f0 = Ctx::CreateFloat(10.5f); 
  Float f1 = Ctx::CreateFloat(0.0f);
  ASSERT_TRUE(static_cast<Bool_T*>(f0->Or(f1).get())->value);
}
TEST(OrTest, BoolOr) {
  Bool b0 = Value_T::True;
  Bool b1 = Value_T::False;
  ASSERT_TRUE(static_cast<Bool_T*>(b0->Or(b1).get())->value);
}
