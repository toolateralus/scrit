#include "value.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "type.hpp"

Int_T::Int_T(int value) : Value_T(TypeSystem::Current().Int) { this->value = value; }

Value Int_T::Clone() { return Ctx::CreateInt(value); }

bool Int_T::Equals(Value value) {
  if (value->GetPrimitiveType() == PrimitiveType::Int) {
    return static_cast<Int_T *>(value.get())->value == this->value;
  }
  return false;
};
string Int_T::ToString() const { return std::to_string(value); }

Value Int_T::Add(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    auto i = Int_T::New(this->value + static_cast<Int_T *>(other.get())->value);
    return i;
  }
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    auto i =
        Float_T::New(this->value + static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
Value Int_T::Subtract(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    return Int_T::New(this->value - static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    auto i =
        Float_T::New(this->value - static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
Value Int_T::Multiply(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    return Int_T::New(this->value * static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    auto i =
        Float_T::New(this->value * static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
Value Int_T::Divide(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    return Int_T::New(this->value / static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    auto i =
        Float_T::New(this->value / static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return Value_T::VNULL;
}
void Int_T::Set(Value newValue) {
  if (newValue->GetPrimitiveType() == PrimitiveType::Int) {
    this->value = static_cast<Int_T *>(newValue.get())->value;
  }
  if (newValue->GetPrimitiveType() == PrimitiveType::Float) {
    this->value = static_cast<Float_T *>(newValue.get())->value;
  }
}
Bool Int_T::Or(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    return Bool_T::New(this->value || static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    auto i =
        Bool_T::New(this->value || static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Int_T::And(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    return Bool_T::New(this->value && static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    auto i =
        Bool_T::New(this->value && static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Int_T::Less(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    return Bool_T::New(this->value < static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    auto i =
        Bool_T::New(this->value < static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}
Bool Int_T::Greater(Value other) {
  if (other->GetPrimitiveType() == PrimitiveType::Int) {
    return Bool_T::New(this->value > static_cast<Int_T *>(other.get())->value);
  }
  if (other->GetPrimitiveType() == PrimitiveType::Float) {
    auto i =
        Bool_T::New(this->value > static_cast<Float_T *>(other.get())->value);
    return i;
  }
  return False;
}

Value Int_T::Negate() { return Int_T::New(-value); }
