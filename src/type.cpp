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
  const auto exists = global_types.contains(type->Name());
  // if a type exists && this comes from a module, we supplement members.
  if (exists && module_type) {
    auto t = global_types[type->Name()];
    for (const auto &[name, member] : type->Scope().Members()) {
      t->Scope().Members()[name] = member;
    }
  } else if (!exists) {
    global_types[type->Name()] = type;
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
  if (this->global_types.contains(type->Name())) {
    return this->global_types[type->Name()];
  }
  this->global_types[type->Name()] = type;
  return type;
}

auto TypeSystem::FromCallable(const Type returnType,
                              const vector<Type> paramTypes) -> Type {
  // todo: make this more efficient.
  auto type = make_shared<CallableType>(returnType, paramTypes);
  if (global_types.contains(type->Name())) {
    return global_types[type->Name()];
  }
  global_types[type->Name()] = type;
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
  return this->Scope().GetValue(name);
}
auto Type_T::Set(const string &name, Value value) -> void {
  this->Scope().Declare(name, value, Mutability::Const);
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
    return scope->GetValue(name);
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

  auto name = base_type->Name() + "<";
  for (size_t i = 0; i < types.size(); ++i) {
    name += types[i]->Name();
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

  if (base_type->Name() == "array") {
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
                       vector<string> &template_args, shared_ptr<Scope_T> scope)
    : name(name), template_args(template_args), fields(std::move(fields)), scope(scope) {
      
}

Value StructType::Default() {
  ASTNode::context.SetCurrentScope(scope);
  for (const auto &field : fields) {
    field->Execute();
  }
  auto object = Ctx::CreateObject(scope->Clone());
  object->type = shared_from_this();
  scope->Clear();
  return object;
}

bool StructType::Equals(const Type_T *other) {
  return other != nullptr && other->Name() == "object" || *other == *this;
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
        auto [it, _] = object->scope->Find(name);
        object->scope->Declare(it->first.value, value, it->first.mutability);
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
const string NamedType_T::Name() const { return name; }
auto GenericType_T::Scope() -> Scope_T & {
  return type_param->type->Scope();
}
const string GenericType_T::Name() const {
  return type_param->type->Name();
}
const string ArrayType::Name() const { return "array"; }
const string AnyType::Name() const { return "any"; }
const string StructType::Name() const { return name; }
const string CallableType::Name() const { return name; }
CallableType::CallableType(const Type returnType,
                                   const std::vector<Type> paramTypes)
    : returnType(returnType), paramTypes(paramTypes) {
  name =
      "func" + TupleType(paramTypes).Name() + " -> " + returnType->Name();
}
const string TupleType::Name() const { return name; }
TupleType::TupleType(const std::vector<Type> &in_subtypes)
    : subtypes(in_subtypes) {
  std::stringstream ss;
  ss << "(";
  for (size_t i = 0; i < subtypes.size(); ++i) {
    ss << subtypes[i]->Name();
    if (i != subtypes.size() - 1) {
      ss << ", ";
    }
  }
  ss << ")";
  name = ss.str();
}
const string BoolType::Name() const { return "bool"; }
const string FloatType::Name() const { return "float"; }
const string ObjectType::Name() const { return "object"; }
const string TemplateType::Name() const { return name; }
bool TemplateType::Equals(const Type_T *other) {
  return other != nullptr && (*other == *this || *other == *base_type);
}
auto Type_T::Equals(const Type_T *other) -> bool {
  if (!other) {
    return false;
  }
  // TODO: improve the any type.
  return *other == *this || other->Name() == "any" || Name() == "any";
}
const string NullType::Name() const { return "null"; }
const string UndefinedType::Name() const { return "undefined"; }
const string StringType::Name() const { return "string"; }
const string IntType::Name() const { return "int"; }
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
  return Name() == other.Name();
}
