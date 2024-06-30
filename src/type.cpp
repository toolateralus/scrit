#include "type.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "value.hpp"
#include <iostream>
#include <ostream>

using namespace Values;

Type_T::Type_T(const std::string &name) : name(name) {}

auto TypeSystem::RegisterType(const Type &type, const bool module_type) -> void {
  const auto exists = types.contains(type->name);
  // if a type exists && this comes from a module, we supplement members.
  if (exists && module_type) {
    auto t = types[type->name];
    for (const auto &[name, member] : type->Scope().Members()) {
      t->Scope().Members()[name] = member;      
    }
  } else if (!exists) {
    types[type->name] = type;
  }
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
  return this->Scope().Get(name);
}
auto Values::Type_T::Set(const string &name, Value value) -> void {
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
  if (scope->Contains(name)) {
    return scope->Get(name);
  }
  // generic functions from the base type.
  if (base_type->Scope().Contains(name)) {
    return base_type->Get(name);
  }
  return nullptr;
}

TemplateType::TemplateType(const string &name, const Type &base_type,
                           const vector<Type> &typenames)
    : Type_T(name), typenames(typenames), base_type(base_type),
      scope(Scope_T::Create()) {}
      
auto Values::TypeSystem::DumpInfo() -> void {
  for (const auto &[name, type] : types) {
    std::cout << "type: " << name << "\ncontains '"
              << type->Scope().Members().size() << "' members." << std::endl;
  }
}
