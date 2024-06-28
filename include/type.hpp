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
};


using Type = shared_ptr<Type_T>;

struct NullType : Type_T {
  NullType() : Type_T("null"){}
};
struct UndefinedType : Type_T {
  UndefinedType() : Type_T("undefined"){}
};
struct LambdaType : Type_T {
  LambdaType() : Type_T("property"){}
};
struct StringType: Type_T {
  StringType() : Type_T("string") {}
};
struct IntType: Type_T {
  IntType() : Type_T("int") {}
};
struct ArrayType: Type_T {
  const Type inner;
  ArrayType(const string &name, const Type &inner) : Type_T(name), inner(inner) {}
};
struct ObjectType: Type_T {
  ObjectType() : Type_T("object") {}
};
struct FloatType: Type_T {
  FloatType() : Type_T("float") {}
};
struct BoolType: Type_T {
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

struct AnyType: Type_T {
  AnyType() : Type_T("any") {}
};

struct TypeSystem {
  static std::unordered_map<string, Type> types;
  static Type Any;
  
  static auto DefaultTypes() -> std::unordered_map<string, Type> {
    return {
      {"bool", std::make_shared<BoolType>()},
      {"int", std::make_shared<IntType>()},
      {"float", std::make_shared<FloatType>()},
      {"string", std::make_shared<StringType>()},
      {"null", std::make_shared<NullType>()},
      {"undefined", std::make_shared<UndefinedType>()},
      {"object", std::make_shared<ObjectType>()},
      {"array", std::make_shared<ArrayType>("array", TypeSystem::Any)}
    };
  }
  
  static auto Get(const string &name) -> Type {
    if (types.empty()) {
      types = DefaultTypes();  
    }
    
    return TypeSystem::types[name];
  }
  
  static auto Undefined() -> Type {
    return Get("undefined");
  }
  static auto ArrayTypeFromInner(const Type &inner) -> Type;
  static auto FromPrimitive(const PrimitveType &value) -> Type;
  static auto FromTuple(const vector<Type> &types) -> Type;
  static auto FromCallable(const Type returnType,
                           const vector<Type> paramTypes) -> Type;
};
} // namespace Values
