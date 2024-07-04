#include "ctx.hpp"
#include "value.hpp"

Values::Array Ctx::FromFloatVector(vector<float> &values) {
  Array array = CreateArray();
  for (const auto &value : values) {
    array->Push(Ctx::CreateFloat(value));
  }
  return array;
}
Values::Array Ctx::FromStringVector(vector<string> &values) {
  Array array = CreateArray();
  for (const auto &value : values) {
    array->Push(Ctx::CreateString(value));
  }
  return array;
}
Values::Array Ctx::FromBoolVector(vector<bool> &values) {
  Array array = CreateArray();
  for (const auto &value : values) {
    array->Push(Ctx::CreateBool(value));
  }
  return array;
}
Values::Array Ctx::FromIntVector(vector<int> &values) {
  Array array = CreateArray();
  for (const auto &value : values) {
    array->Push(Ctx::CreateInt(value));
  }
  return array;
}
Value Ctx::Undefined() { return Value_T::UNDEFINED; }
Value Ctx::Null() { return Value_T::VNULL; }

Bool Ctx::CreateBool(const bool value) { return Bool_T::New(value); }
String Ctx::CreateString(const string value) { return String_T::New(value); }
Int Ctx::CreateInt(const int value) { return Int_T::New(value); }
Float Ctx::CreateFloat(const float value) { return Float_T::New(value); }
Object Ctx::CreateObject(Scope scope) { return Object_T::New(scope); }
Array Ctx::CreateArray(vector<Value> values) { return Array_T::New(values); }

bool Ctx::TryGetArray(Value value, Array &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Array) {
    result = std::dynamic_pointer_cast<Array_T>(value);
    return true;
  }
  return false;
}
bool Ctx::TryGetObject(Value value, Object &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Object) {
    result = std::dynamic_pointer_cast<Object_T>(value);
    return true;
  }
  return false;
}
bool Ctx::TryGetBool(Value value, bool &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Bool) {
    result = static_cast<Bool_T *>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::TryGetFloat(Value value, float &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Float) {
    result = static_cast<Float_T *>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::TryGetInt(Value value, int &result) {
  if (value->GetPrimitiveType() == PrimitiveType::Int) {
    result = static_cast<Int_T *>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::TryGetString(Value value, string &result) {
  if (value->GetPrimitiveType() == PrimitiveType::String) {
    result = static_cast<String_T *>(value.get())->value;
    return true;
  }
  return false;
}
bool Ctx::IsUndefined(Value value) { return value->Equals(Value_T::UNDEFINED); }
bool Ctx::IsNull(Value value) { return value->Equals(Value_T::VNULL); }

Values::Array Ctx::CreateArray() { return Array_T::New(); }
