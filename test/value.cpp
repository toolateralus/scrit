#include <gtest/gtest.h>
#include "value.hpp"

TEST(ValueTest, StrEquals) {
  String EMPTY = make_shared<String_T>("");
  String NOT_EMPTY = make_shared<String_T>("test string");
  
  ASSERT_EQ(true, EMPTY->Equals(EMPTY));
  ASSERT_EQ(true, NOT_EMPTY->Equals(NOT_EMPTY));
  ASSERT_EQ(false, EMPTY->Equals(NOT_EMPTY));
}

TEST(ValueTest, UndefinedNullEquals) {
  Undefined UNDEFINED = Value_T::Undefined;
  Null NULLT = Value_T::Null;
  ASSERT_EQ(true, UNDEFINED->Equals(UNDEFINED));
  ASSERT_EQ(true, NULLT->Equals(NULLT));
  ASSERT_EQ(false, UNDEFINED->Equals(NULLT));

}
TEST(ValueTest, UndefinedNullEqualsUpcasted) {
  Undefined UNDEFINED = Value_T::Undefined;
  Null NULLT = Value_T::Null;
  Value UNDEFINED_V = UNDEFINED;
  Value NULL_V = NULLT;
  ASSERT_EQ(true, UNDEFINED_V->Equals(UNDEFINED_V));
  ASSERT_EQ(true, NULL_V->Equals(NULL_V));
  ASSERT_EQ(false, UNDEFINED_V->Equals(NULL_V));
}


TEST(ValueTest, IntEquals) {
  Int ZERO = make_shared<Int_T>(0);
  Int ONE = make_shared<Int_T>(1);
  ASSERT_EQ(true, ZERO->Equals(ZERO));
  ASSERT_EQ(true, ONE->Equals(ONE));
  ASSERT_EQ(false, ZERO->Equals(ONE));
}

TEST(ValueTest, FloatEquals) {
  Float FZERO = make_shared<Float_T>(0);
  Float FONE = make_shared<Float_T>(1);
  ASSERT_EQ(true, FZERO->Equals(FZERO));
  ASSERT_EQ(true, FONE->Equals(FONE));
  ASSERT_EQ(false, FONE->Equals(FZERO));
}

TEST(ValueTest, BoolEquals) {
  Bool TRUE = Value_T::False;
  Bool FALSE = Value_T::True;
  ASSERT_EQ(true, TRUE->Equals(TRUE));
  ASSERT_EQ(false, TRUE->Equals(FALSE));
}
