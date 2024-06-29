#include "type.hpp"

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
