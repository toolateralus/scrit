#include "type.hpp"

auto TypeSystem::FromPrimitive(const PrimitveType &t) -> Type {
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
                              const vector<Type> paramTypes) -> Type{
  return make_shared<CallableType>(returnType, paramTypes);
}

auto TypeSystem::ArrayTypeFromInner(const Type &inner) -> Type {
  for (const auto &[n, t] : types) {
    if (auto array = std::dynamic_pointer_cast<ArrayType>(t)) {
      if (inner == array->inner) {
        return t;
      }
    }
  }
  auto name ="array<" + inner->name + ">";
  auto new_type = make_shared<ArrayType>(name, inner);
  types[name] = new_type;
  return new_type;
}
auto TypeSystem::GetDefault(const Type &type) -> Value {
  if (type->name == "bool") {
    return Value_T::False;
  } else if (type->name == "int") {
    return Ctx::CreateInt();
  } else if (type->name == "float") {
    return Ctx::CreateFloat();
  } else if (type->name == "array") {
    return Ctx::CreateArray();
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
