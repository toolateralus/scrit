#include "native.hpp"
#include "value.hpp"
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

using std::shared_ptr;
using std::string;
using std::unordered_map;

namespace Values {
struct Value_T;
using Value = shared_ptr<Value_T>;

struct Type_T {
  Type_T(const std::string &name) : name(name) {}
  virtual ~Type_T(){};
  const string name;
  bool operator==(const Type_T &other) const { return name == other.name; }
  static bool Equals(const Type_T *t0, const Type_T *t1) {
    if (t0 == nullptr || t1 == nullptr) {
      return false;
    }
    return *t0 == *t1;
  }
};
;

using Type = shared_ptr<Type_T>;

struct NullType : Type_T {
  NullType() : Type_T("null") {}
};
struct UndefinedType : Type_T {
  UndefinedType() : Type_T("undefined") {}
};
struct LambdaType : Type_T {
  LambdaType() : Type_T("property") {}
};
struct StringType : Type_T {
  StringType() : Type_T("string") {}
};
struct IntType : Type_T {
  IntType() : Type_T("int") {}
};

struct TemplateType : Type_T {
  const vector<Type> typenames;
  const Type base_type;
  TemplateType(const string &name, const Type &base_type,
               const vector<Type> &typenames)
      : Type_T(name), typenames(typenames), base_type(base_type) {
  }
};

struct ObjectType : Type_T {
  ObjectType() : Type_T("object") {}
};
struct FloatType : Type_T {
  FloatType() : Type_T("float") {}
};
struct BoolType : Type_T {
  BoolType() : Type_T("bool") {}
};
struct TupleType : Type_T {
  const std::vector<Type> subtypes;
  TupleType(const std::vector<Type> &in_subtypes)
      : Type_T(GetName(in_subtypes)), subtypes(in_subtypes) {}
  static auto GetName(const std::vector<Type> &subtypes) -> string {
    std::stringstream ss;
    ss << "(";
    for (size_t i = 0; i < subtypes.size(); ++i) {
      ss << subtypes[i]->name;
      if (i != subtypes.size() - 1) {
        ss << ", ";
      }
    }
    ss << ")";
    return ss.str();
  }
};
struct CallableType : Type_T {
  const Type returnType;
  const std::vector<Type> paramTypes;
  CallableType(const Type returnType, const std::vector<Type> paramTypes)
      : Type_T(CallableType::GetName(returnType, paramTypes)),
        returnType(returnType), paramTypes(paramTypes) {}

private:
  static auto GetName(const Type returnType, const std::vector<Type> paramTypes)
      -> string {
    return "func" + returnType->name + TupleType::GetName(paramTypes);
  }
};

struct ArrayType : Type_T {
  ArrayType() : Type_T("array") {}
};

struct AnyType : Type_T {
  AnyType() : Type_T("any") {}
};

struct TypeSystem {
  std::unordered_map<string, Type> types = {
      {"null", std::make_shared<NullType>()},
      {"undefined", std::make_shared<UndefinedType>()},
      {"int", std::make_shared<IntType>()},
      {"float", std::make_shared<FloatType>()},
      {"bool", std::make_shared<BoolType>()},
      {"array", make_shared<TemplateType>("array", make_shared<ArrayType>(), vector<Type>())},
      {"string", std::make_shared<StringType>()},
      {"object", std::make_shared<ObjectType>()},
  };
  
  const Type Any = make_shared<AnyType>();
  
  auto Get(const string &name) -> Type { return types[name]; }
  
  auto GetOrCreateTemplate(const string &name, const Type &base,
                           const vector<Type> &types) -> Type;
  auto Undefined() -> Type { return Get("undefined"); }
  
  auto GetDefault(const Type &type) -> Value;
  
  auto FromPrimitive(const PrimitiveType &value) -> Type;
  
  auto FromTuple(const vector<Type> &types) -> Type;
  
  auto FromCallable(const Type returnType, const vector<Type> paramTypes)
      -> Type;
  
  
  static TypeSystem &Current() {
    static TypeSystem instance;
    return instance;
  }
private:
  TypeSystem(const TypeSystem&) = delete;
  TypeSystem(TypeSystem&&) = delete;
  TypeSystem& operator=(const TypeSystem&) = delete;
  TypeSystem& operator=(TypeSystem&&) = delete;
  ~TypeSystem() { types.clear(); }
  TypeSystem() {}
};
} // namespace Values
