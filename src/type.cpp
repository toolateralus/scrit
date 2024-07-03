#include "type.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "value.hpp"
#include <iostream>
#include <ostream>
#include <stdexcept>

using namespace Values;

Type_T::Type_T(const std::string &name) : name(name) {}

auto TypeSystem::RegisterType(const Type &type,
                              const bool module_type) -> void {
  const auto exists = global_types.contains(type->name);
  // if a type exists && this comes from a module, we supplement members.
  if (exists && module_type) {
    auto t = global_types[type->name];
    for (const auto &[name, member] : type->Scope().Members()) {
      t->Scope().Members()[name] = member;
    }
  } else if (!exists) {
    global_types[type->name] = type;
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
  if (this->global_types.contains(type->name)) {
    return this->global_types[type->name];
  }
  this->global_types[type->name] = type;
  return type;
}

auto TypeSystem::FromCallable(const Type returnType,
                              const vector<Type> paramTypes) -> Type {
  // todo: make this more efficient.
  auto type = make_shared<CallableType>(returnType, paramTypes);
  if (global_types.contains(type->name)) {
    return global_types[type->name];
  }
  global_types[type->name] = type;
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

  if (base_type->name == "array") {
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
Value ArrayType::Default() { return Ctx::CreateArray(); }
Value AnyType::Default() { return Ctx::Undefined(); }

StructType::StructType(const string &name,
                       std::unique_ptr<ObjectInitializer> &&ctor_obj, vector<string> &template_args)
    : Type_T(name), ctor_obj(std::move(ctor_obj)), template_args(template_args) {
  // this happens during forward declaration during parsing of struct so they
  // can reference their own type.
  if (this->ctor_obj == nullptr) {
    return;
  }
  
  for (const auto &statement : this->ctor_obj->block->statements) {
    if (auto decl = dynamic_cast<Declaration *>(statement.get())) {
      field_names.push_back(decl->name);
    }
  }
}

Value StructType::Default() {
  auto object = ctor_obj->Evaluate()->Clone();
  object->type = shared_from_this();
  return object;
}

bool StructType::Equals(const Type_T *other) {
  return other != nullptr && other->name == "object" || *other == *this;
}
Scope_T &StructType::Scope() {
  static Scope_T scope;
  return scope;
}
StructType::~StructType() {}

Value StructType::Construct(ArgumentsPtr &args) {
  auto object = std::dynamic_pointer_cast<Object_T>(Default());
  std::cout << object->ToString() << std::endl;

  size_t i = 0;
  for (const auto &arg : args->values) {
    auto value = arg->Evaluate();
    if (i < field_names.size()) {
      auto name = field_names[i];
      auto field_type = object->GetMember(name)->type;
      
      if (!field_type->Equals(arg->type.get())) {
        throw TypeError(field_type, arg->type);
      }
      object->SetMember(name, value);
    }
    ++i;
  }
  return object;
}
