#include <gtest/gtest.h>
#include "value.hpp"

TEST(EqTest, StrEquals) {
  String EMPTY = String_T::New("");
  String NOT_EMPTY = String_T::New("test string");
  String IDENTICAL = String_T::New("test string");
  
  ASSERT_TRUE(EMPTY->Equals(EMPTY));
  ASSERT_TRUE(NOT_EMPTY->Equals(NOT_EMPTY));
  ASSERT_TRUE(NOT_EMPTY->Equals(IDENTICAL));
  ASSERT_FALSE(EMPTY->Equals(NOT_EMPTY));
}
TEST(EqTest, UndefinedNullEquals) {
  Undefined UNDEFINED = Value_T::Undefined;
  Null NULLT = Value_T::Null;
  ASSERT_EQ(true, UNDEFINED->Equals(UNDEFINED));
  ASSERT_EQ(true, NULLT->Equals(NULLT));
  ASSERT_EQ(false, UNDEFINED->Equals(NULLT));

}
TEST(EqTest, UndefinedNullEqualsUpcasted) {
  Undefined UNDEFINED = Value_T::Undefined;
  Null NULLT = Value_T::Null;
  Value UNDEFINED_V = UNDEFINED;
  Value NULL_V = NULLT;
  ASSERT_EQ(true, UNDEFINED_V->Equals(UNDEFINED_V));
  ASSERT_EQ(true, NULL_V->Equals(NULL_V));
  ASSERT_EQ(false, UNDEFINED_V->Equals(NULL_V));
}
TEST(EqTest, IntEquals) {
  Int ZERO = Int_T::New(0);
  Int ONE = Int_T::New(1);
  ASSERT_EQ(true, ZERO->Equals(ZERO));
  ASSERT_EQ(true, ONE->Equals(ONE));
  ASSERT_EQ(false, ZERO->Equals(ONE));
}
TEST(EqTest, FloatEquals) {
  Float FZERO = Float_T::New(0);
  Float FONE = Float_T::New(1);
  ASSERT_EQ(true, FZERO->Equals(FZERO));
  ASSERT_EQ(true, FONE->Equals(FONE));
  ASSERT_EQ(false, FONE->Equals(FZERO));
}
TEST(EqTest, BoolEquals) {
  Bool TRUE = Value_T::False;
  Bool FALSE = Value_T::True;
  ASSERT_EQ(true, TRUE->Equals(TRUE));
  ASSERT_EQ(false, TRUE->Equals(FALSE));
}