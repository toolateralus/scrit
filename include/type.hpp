#pragma once
#include <memory>
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
struct Declaration;

namespace Values {
struct Value_T;
enum struct PrimitiveType;
using Value = shared_ptr<Value_T>;

struct Type_T {
  virtual ~Type_T() {};
  virtual const string Name() const = 0;
  Type_T() {};
  Type_T(const Type_T &) = delete;
  Type_T(Type_T &&) = delete;
  Type_T &operator=(const Type_T &) = delete;
  Type_T &operator=(Type_T &&) = delete;
  virtual auto Scope() -> Scope_T & = 0;
  virtual auto Default() -> Value = 0;
  virtual auto Get(const string &name) -> Value;
  virtual auto Set(const string &name, Value value) -> void;
  bool operator==(const Type_T &other) const;
  virtual auto Equals(const Type_T *other) -> bool;
};

using Type = shared_ptr<Type_T>;

struct NullType : Type_T {
  const string Name() const override;
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct UndefinedType : Type_T {
  const string Name() const override;
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct StringType : Type_T {
  const string Name() const override;
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct IntType : Type_T {
  const string Name() const override;
  auto Scope() -> Scope_T & override;
  Value Default() override;
};

struct TemplateType : Type_T, std::enable_shared_from_this<TemplateType> {
  const vector<Type> typenames;
  const Type base_type;
  const shared_ptr<Scope_T> scope;
  TemplateType(const string &name, const Type &base_type, const vector<Type> &typenames);
  string name;
  auto Scope() -> Scope_T & override;
  auto Get(const string &name) -> Value override;
  bool Equals(const Type_T *other) override;
  const string Name() const override;
  Value Default() override;
};

struct ObjectType : Type_T {
  const string Name() const override;
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct FloatType : Type_T {
  const string Name() const override;
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct BoolType : Type_T {
  const string Name() const override;
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct TupleType : Type_T {
  const std::vector<Type> subtypes;
  TupleType(const std::vector<Type> &in_subtypes);
  auto Scope() -> Scope_T & override;
  Value Default() override;
  string name;
  const string Name() const override;
};
struct CallableType : Type_T, std::enable_shared_from_this<CallableType> {
  const Type returnType;
  const std::vector<Type> paramTypes;
  auto Scope() -> Scope_T & override;
  CallableType(const Type returnType, const std::vector<Type> paramTypes);
  Value Default() override;
  string name;
  const string Name() const override;
};
struct TypeParam_T {
  string name;
  Type type;
};
using TypeParam = std::shared_ptr<TypeParam_T>;
struct GenericType_T : Type_T {
  const TypeParam type_param;
  const string Name() const override;
  GenericType_T(const TypeParam type_param);
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct NamedType_T : Type_T {
  const string name;
  const string Name() const override;
  NamedType_T(const string name);
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct ArrayType : Type_T {
  const string Name() const override;
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct AnyType : Type_T {
  const string Name() const override;
  auto Scope() -> Scope_T & override;
  Value Default() override;
};
struct StructType : Type_T, std::enable_shared_from_this<StructType> {
  ~StructType();
  shared_ptr<Scope_T> scope;
  vector<std::unique_ptr<Declaration>> fields;
  vector<string> template_args;
  
  StructType(const StructType &) = delete;
  StructType(StructType &&) = delete;
  StructType &operator=(const StructType &) = delete;
  StructType &operator=(StructType &&) = delete;
  
  StructType(const string &name, vector<std::unique_ptr<Declaration>> &&fields,
             vector<string> &template_args, shared_ptr<Scope_T> scope);
  bool Equals(const Type_T *other) override;
  Value Default() override;
  Value Construct(ArgumentsPtr &args);
  Scope_T &Scope() override;
  string name;
  const string Name() const override;
};

struct TypeSystem {
  std::unordered_map<string, Type> global_types;

  Type Null;
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
  auto Initialize() -> void;
  TypeSystem(const TypeSystem &) = delete;
  TypeSystem(TypeSystem &&) = delete;
  TypeSystem &operator=(const TypeSystem &) = delete;
  TypeSystem &operator=(TypeSystem &&) = delete;
  ~TypeSystem() { global_types.clear(); }
  TypeSystem() { Initialize(); }
};
} // namespace Values
