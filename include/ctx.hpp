#pragma once

#include <cstdint>
#include <memory>
#include <vector>

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

struct Scope_T;
using Scope = shared_ptr<Scope_T>;
namespace Values {
struct Value_T;
struct Bool_T;
struct String_T;
struct Int_T;
struct Float_T;
struct Array_T;
struct Object_T;

using Value = shared_ptr<Value_T>;
using Bool = shared_ptr<Bool_T>;
using String = shared_ptr<String_T>;
using Float = shared_ptr<Float_T>;
using Array = shared_ptr<Array_T>;
using Object = shared_ptr<Object_T>;
using Int = shared_ptr<Int_T>;
} // namespace Values

using namespace Values;
struct Ctx final {
  Ctx() = delete;
  
  Ctx(const Ctx &) = default;
  Ctx(Ctx &&) = default;
  Ctx &operator=(const Ctx &) = default;
  Ctx &operator=(Ctx &&) = default;
  static Value Null();
  static Bool CreateBool(const bool value = false);
  static String CreateString(const string value = std::string(""));
  static Int CreateInt(const int64_t value = 0);
  static Float CreateFloat(const double value = 0.0f);
  static Object CreateObject(shared_ptr<Scope_T> scope = nullptr);
  static Array CreateArray(vector<Value> values);
  static Array CreateArray();
  
  static Array FromFloatVector(vector<double> &values);
  static Array FromStringVector(vector<string> &values);
  static Array FromBoolVector(vector<bool> &values);
  static Array FromIntVector(vector<int64_t> &values);
  
  static bool TryGetString(Value str, string &result);
  static bool TryGetInt(Value value, int64_t &result);
  static bool TryGetFloat(Value value, double &result);
  static bool TryGetBool(Value value, bool &result);
  static bool TryGetObject(Value value, Object &result);
  static bool TryGetArray(Value value, Array &result);
  static bool IsNull(Value value);
};