#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::make_shared;
using std::vector;
using std::shared_ptr;
using std::string;
using std::unordered_map;

struct Scope_T;

namespace Values {
struct Value_T;
enum struct PrimitiveType;
using Value = shared_ptr<Value_T>;

struct Type_T {
  Type_T(const std::string &name);
  virtual ~Type_T(){};
  const string name;
  
  Type_T(const Type_T&) = delete;
  Type_T(Type_T&&) = delete;
  Type_T& operator=(const Type_T&) = delete;
  Type_T& operator=(Type_T&&) = delete;
  
  virtual auto Scope() -> Scope_T& = 0;
  
  virtual auto Get(const string &name) -> Value;
  virtual auto Set(const string &name, Value value) -> void;
  
  bool operator==(const Type_T &other) const { return name == other.name; }
  
  static bool Equals(const Type_T *t0, const Type_T *t1) {
    if (t0 == nullptr || t1 == nullptr) {
      return false;
    }
    return *t0 == *t1;
  }
};

using Type = shared_ptr<Type_T>;

struct NullType : Type_T {
  NullType() : Type_T("null") {}
  auto Scope() -> Scope_T& override;
};
struct UndefinedType : Type_T {
  UndefinedType() : Type_T("undefined") {}
  auto Scope() -> Scope_T & override;
};
struct LambdaType : Type_T {
  LambdaType() : Type_T("property") {}
  auto Scope() -> Scope_T & override;
};
struct StringType : Type_T {
  StringType() : Type_T("string") {}
  auto Scope() -> Scope_T & override;
};
struct IntType : Type_T {
  IntType() : Type_T("int") {}
  auto Scope() -> Scope_T & override;
};

struct TemplateType : Type_T {
  const vector<Type> typenames;
  const Type base_type;
  const shared_ptr<Scope_T> scope;
  TemplateType(const string &name, const Type &base_type,
               const vector<Type> &typenames);
  
  auto Scope() -> Scope_T& override {
    return *scope;
  }
  auto Get(const string &name) -> Value override;
};

struct ObjectType : Type_T {
  ObjectType() : Type_T("object") {}
  auto Scope() -> Scope_T & override;
};
struct FloatType : Type_T {
  FloatType() : Type_T("float") {}
  auto Scope() -> Scope_T & override;
};
struct BoolType : Type_T {
  BoolType() : Type_T("bool") {}
  auto Scope() -> Scope_T & override;
};
struct TupleType : Type_T {
  const std::vector<Type> subtypes;
  TupleType(const std::vector<Type> &in_subtypes)
      : Type_T(GetName(in_subtypes)), subtypes(in_subtypes) {}
  auto Scope() -> Scope_T & override;
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
  auto Scope() -> Scope_T & override;
  CallableType(const Type returnType, const std::vector<Type> paramTypes)
      : Type_T(CallableType::GetName(returnType, paramTypes)),
        returnType(returnType), paramTypes(paramTypes) {}

private:
  static auto GetName(const Type returnType, const std::vector<Type> paramTypes)
      -> string {
    return returnType->name + TupleType::GetName(paramTypes);
  }
};

struct ArrayType : Type_T {
  ArrayType() : Type_T("array") {}
  auto Scope() -> Scope_T & override;
};

struct AnyType : Type_T {
  AnyType() : Type_T("any") {}
  auto Scope() -> Scope_T & override;
};

struct TypeSystem {
  std::unordered_map<string, Type> types = {
      {"null", std::make_shared<NullType>()},
      {"undefined", std::make_shared<UndefinedType>()},
      {"int", std::make_shared<IntType>()},
      {"float", std::make_shared<FloatType>()},
      {"bool", std::make_shared<BoolType>()},
      {"native_callable", std::make_shared<CallableType>(make_shared<AnyType>(), std::vector<Type>({}))},
      {"array", make_shared<ArrayType>()},
      {"string", std::make_shared<StringType>()},
      {"object", std::make_shared<ObjectType>()},
      {"any", Any}
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

  auto RegisterType(const Type &type);

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
