#include <gtest/gtest.h>
#include "native.hpp"
#include "value.hpp"

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
  Undefined UNDEFINED = Value_T::Undefined;
  Null NULLT = Value_T::Null;
  ASSERT_TRUE(UNDEFINED->Equals(UNDEFINED));
  ASSERT_TRUE(NULLT->Equals(NULLT));
  ASSERT_FALSE(UNDEFINED->Equals(NULLT));
  ASSERT_FALSE(NULLT->Equals(UNDEFINED));
}
TEST(EqTest, UndefinedNullEqualsUpcasted) {
  Undefined UNDEFINED = Value_T::Undefined;
  Null NULLT = Value_T::Null;
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