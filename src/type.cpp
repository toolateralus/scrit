#include "type.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "ctx.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <vector>

using namespace Values;

auto TypeSystem::RegisterType(const Type &type,
                              const bool module_type) -> void {
  const auto exists = global_types.contains(type->GetName());
  // if a type exists && this comes from a module, we supplement members.
  if (exists && module_type) {
    auto t = global_types[type->GetName()];
    for (const auto &[name, member] : type->Scope().Members()) {
      t->Scope().Members()[name] = member;
    }
  } else if (!exists) {
    global_types[type->GetName()] = type;
  }
}

auto TypeSystem::FromPrimitive(const PrimitiveType &t) -> Type {
  auto id = TypeToString(t);
  for (const auto &[name, type] : global_types) {
    if (id == name) {
      return type;
    }
  }
  return nullptr;
}

auto TypeSystem::FromTuple(const vector<Type> &types) -> Type {
  auto type = make_shared<TupleType>(types);
  if (this->global_types.contains(type->GetName())) {
    return this->global_types[type->GetName()];
  }
  this->global_types[type->GetName()] = type;
  return type;
}

auto TypeSystem::FromCallable(const Type returnType,
                              const vector<Type> paramTypes) -> Type {
  // todo: make this more efficient.
  auto type = make_shared<CallableType>(returnType, paramTypes);
  if (global_types.contains(type->GetName())) {
    return global_types[type->GetName()];
  }
  global_types[type->GetName()] = type;
  return type;
}

auto TypeSystem::FindOrCreateTemplate(const string &name, const Type &base,
                                      const vector<Type> &types) -> Type {
  auto &current = Current();
  if (current.global_types.contains(name)) {
    return current.global_types[name];
  }
  auto type = make_shared<TemplateType>(name, base, types);
  current.global_types[name] = type;
  return type;
}

auto Type_T::Get(const string &name) -> Value {
  return this->Scope().Get(name);
}
auto Type_T::Set(const string &name, Value value) -> void {
  this->Scope().Set(name, value, Mutability::Const);
}
auto NullType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto UndefinedType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto CallableType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto TupleType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto BoolType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto FloatType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto ObjectType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto AnyType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto ArrayType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto StringType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
  return scope;
}
auto IntType::Scope() -> Scope_T & {
  static Scope_T scope = Scope_T(nullptr);
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

GenericType_T::GenericType_T(const TypeParam type_param) : type_param(type_param) {}

TemplateType::TemplateType(const string &name, const Type &base_type,
                           const vector<Type> &typenames)
    : name(name), typenames(typenames), base_type(base_type),
      scope(make_shared<Scope_T>(nullptr)) {}

auto TypeSystem::DumpInfo() -> void {
  for (const auto &[name, type] : global_types) {
    std::cout << "type: " << name << "\ncontains '"
              << type->Scope().Members().size() << "' members." << std::endl;
  }
}
auto TypeSystem::GetVector(const vector<string> &names) -> vector<Type> {
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
TypeArgsPtr Parser::ParseTypeArgs() {
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
  return std::make_unique<TypeArguments>(info, std::move(types));
}
Type Parser::ParseTemplateType(const Type &base_type) {

  auto types = ParseTypeArgs()->types;

  auto name = base_type->GetName() + "<";
  for (size_t i = 0; i < types.size(); ++i) {
    name += types[i]->GetName();
    if (i < types.size() - 1) { // Check if it's not the last element
      name += ", ";
    }
  }
  name += ">";

  return TypeSystem::Current().FindOrCreateTemplate(name, base_type, types);
}
Type Parser::ParseFunctionType() {
  Expect(TType::Func);
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
  auto returnType = ParseReturnType();
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
  if (!tokens.empty() && Peek().type == TType::Func) {
    return Parser::ParseFunctionType();
  }

  if (!tokens.empty() && Peek().type == TType::LParen) {
    return ParseTupleType();
  }

  auto tname = Expect(TType::Identifier).value;
  auto type = TypeSystem::Current().Find(tname);
  // template types.
  if (!tokens.empty() && Peek().type == TType::Less) {
    return Parser::ParseTemplateType(type);
  }

  return type;
}

auto TypeSystem::FromTuple(const vector<Value> &values) -> Type {
  vector<Type> types;
  for (const auto &value : values) {
    types.push_back(value->type);
  }
  return FromTuple(types);
}
auto TypeSystem::Exists(const string &name) -> bool {
  auto exists = global_types.contains(name);
  if (exists) {
    return exists;
  }
  return ASTNode::context.TypeExists(name);
}
auto TypeSystem::Find(const string &name) -> Type {
  // Return a type if it exists normally in the hash map.
  if (global_types.contains(name)) {
    return global_types[name];
  }

  if (ASTNode::context.TypeExists(name)) {
    return ASTNode::context.FindType(name);
  }

  throw std::runtime_error("use of undeclared type: " + name);
}
Value NullType::Default() { return Ctx::Null(); }
Value UndefinedType::Default() { return Ctx::Undefined(); }
Value StringType::Default() { return Ctx::CreateString(); }
Value IntType::Default() { return Ctx::CreateInt(); }
Value TemplateType::Default() {
  // todo: figure out how we'll ever default construct various template types.

  if (base_type->GetName() == "array") {
    auto array = Ctx::CreateArray();
    array->type = shared_from_this();
    return array;
  }
  return Ctx::Undefined();
}
Value ObjectType::Default() { return Ctx::CreateObject(); }
Value FloatType::Default() { return Ctx::CreateFloat(); }
Value BoolType::Default() { return Ctx::CreateBool(); }
Value TupleType::Default() {
  vector<Value> values;
  for (const auto &type : subtypes) {
    values.push_back(type->Default());
  }
  return make_shared<Tuple_T>(values);
}
Value CallableType::Default() { return Ctx::Undefined(); }
Value GenericType_T::Default() { return Ctx::Undefined(); }
Value NamedType_T::Default() { return Ctx::Undefined(); }
Value ArrayType::Default() { return Ctx::CreateArray(); }
Value AnyType::Default() { return Ctx::Undefined(); }

StructType::StructType(const string &name,
                       vector<std::unique_ptr<Declaration>> &&fields,
                       vector<string> &template_args)
    : name(name), template_args(template_args), fields(std::move(fields)) {
      
}

Value StructType::Default() {
  auto object = Ctx::CreateObject();
  ASTNode::context.SetCurrentScope(make_shared<Scope_T>(nullptr));
  for (const auto &field : fields) {
    field->Execute();
  }
  object->type = shared_from_this();
  return object;
}

bool StructType::Equals(const Type_T *other) {
  return other != nullptr && other->GetName() == "object" || *other == *this;
}
Scope_T &StructType::Scope() {
  return *this->scope;
}
StructType::~StructType() {}

Value StructType::Construct(ArgumentsPtr &args) {
  auto object = std::dynamic_pointer_cast<Object_T>(Default());
  size_t i = 0;
  for (const auto &arg : args->values) {
    auto value = arg->Evaluate();
    if (i < fields.size()) {
      auto &name = fields[i]->name;
      auto field_type = object->GetMember(name)->type;
      if (!field_type->Equals(arg->type.get())) {
        throw TypeError(field_type, arg->type);
      }
      if (object->scope->Contains(name)) {
        auto it = object->scope->Find(name);
        object->scope->Erase(name);
        object->scope->Set(it->first, value);
      } else {
        object->SetMember(name, value);
      }
    }
    ++i;
  }
  return object;
}
auto NamedType_T::Scope() -> Scope_T & {
  throw std::runtime_error(
      "scope of this type should not be accessed ever");
}
const string NamedType_T::GetName() const { return name; }
auto GenericType_T::Scope() -> Scope_T & {
  return type_param->type->Scope();
}
const string GenericType_T::GetName() const {
  return type_param->type->GetName();
}
const string ArrayType::GetName() const { return "array"; }
const string AnyType::GetName() const { return "any"; }
const string StructType::GetName() const { return name; }
const string CallableType::GetName() const { return name; }
CallableType::CallableType(const Type returnType,
                                   const std::vector<Type> paramTypes)
    : returnType(returnType), paramTypes(paramTypes) {
  name =
      "func" + TupleType(paramTypes).GetName() + " -> " + returnType->GetName();
}
const string TupleType::GetName() const { return name; }
TupleType::TupleType(const std::vector<Type> &in_subtypes)
    : subtypes(in_subtypes) {
  std::stringstream ss;
  ss << "(";
  for (size_t i = 0; i < subtypes.size(); ++i) {
    ss << subtypes[i]->GetName();
    if (i != subtypes.size() - 1) {
      ss << ", ";
    }
  }
  ss << ")";
  name = ss.str();
}
const string BoolType::GetName() const { return "bool"; }
const string FloatType::GetName() const { return "float"; }
const string ObjectType::GetName() const { return "object"; }
const string TemplateType::GetName() const { return name; }
bool TemplateType::Equals(const Type_T *other) {
  return other != nullptr && (*other == *this || *other == *base_type);
}
auto Type_T::Equals(const Type_T *other) -> bool {
  if (!other) {
    return false;
  }
  // TODO: improve the any type.
  return *other == *this || other->GetName() == "any" || GetName() == "any";
}
const string NullType::GetName() const { return "null"; }
const string UndefinedType::GetName() const { return "undefined"; }
const string StringType::GetName() const { return "string"; }
const string IntType::GetName() const { return "int"; }
auto TemplateType::Scope() -> Scope_T & { return *scope; }
NamedType_T::NamedType_T(const string name) : name(name) {}
auto TypeSystem::Initialize() -> void {
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
bool Type_T::operator==(const Type_T &other) const {
  return GetName() == other.GetName();
}
