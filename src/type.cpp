#include "type.hpp"
#include "parser.hpp"
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
  auto type = make_shared<TupleType>(types);
  if (this->types.contains(type->name)) {
    return this->types[type->name];
  }
  this->types[type->name] = type;
  return type;
}

auto TypeSystem::FromCallable(const Type returnType,
                              const vector<Type> paramTypes) -> Type {
  // todo: make this more efficient.
  auto type= make_shared<CallableType>(returnType, paramTypes);
  if (types.contains(type->name)) {
    return types[type->name];
  }
  types[type->name] = type;
  return type;
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

auto Values::TypeSystem::FindOrCreateTemplate(const string &name,
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
auto Values::TypeSystem::GetVector(const vector<string> &names)
    -> vector<Type> {
  auto types = vector<Type>();
  for (const auto &name : names) {
    auto type = Find(name);
    if (!type) {
      throw std::runtime_error("invalid type in type group: " + name);
    }
    types.push_back(type);
  }
  return types;
}

// Type parsing
Type Parser::ParseReturnType() {
  Type returnType = nullptr;
  if (Peek().type == TType::LCurly) {
    returnType = TypeSystem::Current().Undefined;
  } else {
    Expect(TType::Arrow);
    returnType = ParseType();
  }
  return returnType;
}
Type Parser::ParseTemplateType(const Type &base_type) {
  Expect(TType::Less);
  vector<Type> types;
  while (!tokens.empty()) {
    auto next = Peek();
    
    // if the next token is > we are done.
    if (next.type == TType::Greater) {
      break;
    }
    
    types.push_back(ParseType());
    
    // eat commas.
    if (Peek().type == TType::Comma) {
      Eat();
    }
  }
  
  Expect(TType::Greater);
  
  auto name = base_type->name + "<";
  for (size_t i = 0; i < types.size(); ++i) {
    name += types[i]->name;
    if (i < types.size() - 1) { // Check if it's not the last element
      name += ", ";
    }
  }
  name += ">";
  
  return TypeSystem::Current().FindOrCreateTemplate(name, base_type, types);
}
Type Parser::ParseFunctionType(const Type &returnType) {
  Expect(TType::LParen);
  std::vector<Type> types;
  while (!tokens.empty()) {
    if (Peek().type == TType::RParen) {
      break;
    } 
    auto type = ParseType();
    types.push_back(type);
    if (Peek().type == TType::Comma) {
      Eat();
    }
  }
  Expect(TType::RParen);
  return TypeSystem::Current().FromCallable(returnType, types);
}
Type Parser::ParseTupleType() {
  Expect(TType::LParen);
  std::vector<Type> types;
  while (!tokens.empty()) {
    types.push_back(ParseType());
    
    if (Peek().type == TType::Comma) {
      Eat();
    }
    if (Peek().type == TType::RParen) {
      break;
    }
  }
  Expect(TType::RParen);
  return TypeSystem::Current().FromTuple(types);
}
Type Parser::ParseType() {
  if (Peek().type == TType::LParen) {
    return ParseTupleType();
  }
  
  
  auto tname = Expect(TType::Identifier).value;
  auto type = TypeSystem::Current().Find(tname);
  // template types.
  if (Peek().type == TType::Less) {
    return Parser::ParseTemplateType(type);
  }
  if (Peek().type == TType::LParen) {
    return Parser::ParseFunctionType(type);
  }
  
  return type;
}

auto Values::TypeSystem::FromTuple(const vector<Value> &values) -> Type {
  vector<Type> types;
  for (const auto &value : values) {
    types.push_back(value->type);
  }
  return FromTuple(types);
}
