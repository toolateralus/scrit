#pragma once
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

struct Arguments;
using ArgumentsPtr = std::unique_ptr<Arguments>;
struct ObjectInitializer;
struct Scope_T;

namespace Values {
struct Value_T;
enum struct PrimitiveType;
using Value = shared_ptr<Value_T>;

struct Type_T {
  Type_T(const std::string &name);
  virtual ~Type_T() {};
  const string name;

  Type_T(const Type_T &) = delete;
  Type_T(Type_T &&) = delete;
  Type_T &operator=(const Type_T &) = delete;
  Type_T &operator=(Type_T &&) = delete;

  virtual auto Scope() -> Scope_T & = 0;

  virtual auto Default() -> Value = 0;
  
  virtual auto Get(const string &name) -> Value;
  virtual auto Set(const string &name, Value value) -> void;
  
  bool operator==(const Type_T &other) const { return name == other.name; }
  
  virtual auto Equals(const Type_T *other) -> bool {
    if (!other) {
      return false;
    }
    // TODO: improve the any type.
    return *other == *this || other->name == "any" || name == "any";
  }
};

using Type = shared_ptr<Type_T>;

struct NullType : Type_T {
  NullType() : Type_T("null") {}
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct UndefinedType : Type_T {
  UndefinedType() : Type_T("undefined") {}
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct StringType : Type_T {
  StringType() : Type_T("string") {}
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct IntType : Type_T {
  IntType() : Type_T("int") {}
  auto Scope() -> Scope_T & override;
  Value Default() override;
};

struct TemplateType : Type_T, std::enable_shared_from_this<TemplateType> {
  const vector<Type> typenames;
  const Type base_type;
  const shared_ptr<Scope_T> scope;
  TemplateType(const string &name, const Type &base_type,
               const vector<Type> &typenames);

  auto Scope() -> Scope_T & override { return *scope; }
  auto Get(const string &name) -> Value override;

  bool Equals(const Type_T *other) override {
    return other != nullptr && (*other == *this || *other == *base_type);
  }

  Value Default() override;
};

struct ObjectType : Type_T {
  ObjectType() : Type_T("object") {}
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct FloatType : Type_T {
  FloatType() : Type_T("float") {}
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct BoolType : Type_T {
  BoolType() : Type_T("bool") {}
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct TupleType : Type_T {
  const std::vector<Type> subtypes;
  TupleType(const std::vector<Type> &in_subtypes)
      : Type_T(GetName(in_subtypes)), subtypes(in_subtypes) {}
  auto Scope() -> Scope_T & override;

  Value Default() override;

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

  Value Default() override;

private:
  static auto GetName(const Type returnType,
                      const std::vector<Type> paramTypes) -> string {
    return returnType->name + TupleType::GetName(paramTypes);
  }
};

struct ArrayType : Type_T {
  ArrayType() : Type_T("array") {}
  auto Scope() -> Scope_T & override;

  Value Default() override;
};

struct AnyType : Type_T {
  AnyType() : Type_T("any") {}
  auto Scope() -> Scope_T & override;
  Value Default() override;
};

struct StructType : Type_T, std::enable_shared_from_this<StructType> {
  ~StructType();
  std::unique_ptr<ObjectInitializer> ctor_obj;
  vector<string> names;
  StructType(const string &name, std::unique_ptr<ObjectInitializer> &&init);
  bool Equals(const Type_T *other) override;
  Value Default() override;
  Value Construct(ArgumentsPtr &args);
  Scope_T &Scope() override;
};

struct TypeSystem {

  std::unordered_map<string, Type> global_types;

  Type Null;
  Type Undefined;
  Type Int;
  Type Float;
  Type Bool;
  Type NativeCallable;
  Type String;
  Type Any;

  auto GetVector(const vector<string> &names) -> vector<Type>;

  auto Find(const string &name) -> Type;

  auto Exists(const string &name) -> bool;

  auto FindOrCreateTemplate(const string &name, const Type &base,
                            const vector<Type> &types) -> Type;

  auto FromPrimitive(const PrimitiveType &value) -> Type;

  auto FromTuple(const vector<Type> &types) -> Type;

  auto FromTuple(const vector<Value> &values) -> Type;

  auto FromCallable(const Type returnType,
                    const vector<Type> paramTypes) -> Type;

  auto RegisterType(const Type &type, const bool module_type = false) -> void;

  auto DumpInfo() -> void;

  static TypeSystem &Current() {
    static TypeSystem instance;
    return instance;
  }

private:
  auto Initialize() -> void {
    Null = std::make_shared<NullType>();
    Undefined = std::make_shared<UndefinedType>();
    Int = std::make_shared<IntType>();
    Float = std::make_shared<FloatType>();
    Bool = std::make_shared<BoolType>();
    NativeCallable = std::make_shared<CallableType>(make_shared<AnyType>(),
                                                    std::vector<Type>({}));
    String = std::make_shared<StringType>();
    Any = make_shared<AnyType>();
    global_types = {{"null", Null},
                    {"undefined", Undefined},
                    {"int", Int},
                    {"float", Float},
                    {"bool", Bool},
                    {"native_callable", NativeCallable},
                    {"string", String},
                    {"array", make_shared<ArrayType>()},
                    {"object", make_shared<ObjectType>()},
                    {"any", Any}};
  }
  TypeSystem(const TypeSystem &) = delete;
  TypeSystem(TypeSystem &&) = delete;
  TypeSystem &operator=(const TypeSystem &) = delete;
  TypeSystem &operator=(TypeSystem &&) = delete;
  ~TypeSystem() { global_types.clear(); }
  TypeSystem() { Initialize(); }
};
} // namespace Values
