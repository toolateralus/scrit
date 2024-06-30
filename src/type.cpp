#include "type.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "value.hpp"
#include <iostream>
#include <ostream>

using namespace Values;

Type_T::Type_T(const std::string &name) : name(name) {}

auto TypeSystem::RegisterType(const Type &type) {
  if (types.contains(type->name)) {
    throw std::runtime_error("cannot register type: '" + type->name +
                             "' twice.");
  }
  types[type->name] = type;
}

auto TypeSystem::FromPrimitive(const PrimitiveType &t) -> Type {
  auto id = TypeToString(t);
  for (const auto &[name, type] : types) {
    if (id == name) {
      return type;
    }
  }
  return nullptr;
}

auto TypeSystem::FromTuple(const vector<Type> &types) -> Type {
  return make_shared<TupleType>(types);
}

auto TypeSystem::FromCallable(const Type returnType,
                              const vector<Type> paramTypes) -> Type {
  return make_shared<CallableType>(returnType, paramTypes);
}

auto TypeSystem::GetDefault(const Type &type) -> Value {
  if (type->name == "bool") {
    return Value_T::False;
  } else if (type->name == "int") {
    return Ctx::CreateInt();
  } else if (type->name == "float") {
    return Ctx::CreateFloat();
  } else if (type->name == "string") {
    return Ctx::CreateString();
  } else if (type->name == "object") {
    return Ctx::CreateObject();
  } else if (type->name.starts_with("array") && type->name.contains("<")) {
    auto array = Ctx::CreateArray();
    array->type = type;
    return array;
  }
  return Value_T::UNDEFINED;
}

auto Values::TypeSystem::GetOrCreateTemplate(const string &name,
                                             const Type &base,
                                             const vector<Type> &types)
    -> Type {
  auto &current = Current();
  if (current.types.contains(name)) {
    return current.types[name];
  }
  auto type = make_shared<TemplateType>(name, base, types);
  current.types[name] = type;
  return type;
}

auto Values::Type_T::Get(const string &name) -> Value {
  std::cout << "getting " << name << ". scope has " << Scope().Members().size() << " members." << std::endl;
  return this->Scope().Get(name);
}
auto Values::Type_T::Set(const string &name, Value value) -> void {
  std::cout << "adding " << name << ": " << value->ToString() << std::endl;
  this->Scope().Set(name, value, Mutability::Const);
}
auto NullType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto LambdaType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto UndefinedType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto CallableType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto TupleType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto BoolType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto FloatType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto ObjectType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto AnyType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto ArrayType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto StringType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto IntType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T();
  return scope;
}
auto TemplateType::Get(const string &name) -> Value {
  // specific implementations for this template.
  if (auto result = scope->Get(name)) {
    return result;
  }
  // generic functions from the base type.
  if (auto result = base_type->Get(name)) {
    return result;
  }
  return nullptr;
}

TemplateType::TemplateType(const string &name, const Type &base_type,
                           const vector<Type> &typenames)
    : Type_T(name), typenames(typenames), base_type(base_type),
      scope(Scope_T::Create()) {}