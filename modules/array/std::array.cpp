#include <scrit/native.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/type.hpp>
#include <scrit/ctx.hpp>

#define function(name) Value name(std::vector<Value> args)

#define undefined Ctx::Null()

function(contains) {
  if (args.size() < 2) {
    return undefined;
  }

  Array result;
  if (!Ctx::TryGetArray(args[0], result)) {
    throw std::runtime_error("contains may only be used on arrays.");
  }
  for (const auto &value : result->values) {
    if (value->Equals(args[1])) {
      return Value_T::True;
    }
  }
  return Value_T::False;
}
function(remove) {
  if (args.empty()) {
    return undefined;
  }

  // Erase array element.
  if (args[0]->GetPrimitiveType() == Values::PrimitiveType::Array) {
    int i;
    Array_T *a = static_cast<Array_T *>(args[0].get());
    if (args[1]->GetPrimitiveType() == Values::PrimitiveType::Callable) {
      // Predicate.
      auto callable = static_cast<Callable_T *>(args[1].get());
      const auto lambda = [callable](Value value) -> bool {
        std::vector<Value> args = {value};
        return callable->Call(args)->Equals(Value_T::True);
      };
      auto new_end = std::remove_if(a->values.begin(), a->values.end(), lambda);
      a->values.erase(new_end, a->values.end());
    } else if (Ctx::TryGetInt(args[1], i)) {
      // At index.
      a->values.erase(a->values.begin(), a->values.begin() + i);
    } else {
      std::runtime_error("Invalid argument passed to {remove} :: expected "
                         "predicate function, or index of element to remove");
    }
  }

  return undefined;
}

function(clear) {
  if (args.size() == 0) {
    return Ctx::Null();
  }

  if (args[0]->GetPrimitiveType() == PrimitiveType::Array) {
    auto array = dynamic_cast<Array_T *>(args[0].get());
    array->values.clear();
  }
  return Ctx::Null();
}

function(expand) {
  static auto __undefined = Ctx::Null();
  auto array = args[0]->Cast<Array_T>();
  auto count = args[1]->Cast<Int_T>()->value;
  Callable_T *callable = nullptr;
  for (int i = array->values.size(); i < count; i++) {
    array->Push(__undefined);
  }
  return args[0];
}

function(push) {
  if (args.empty()) {
    return Ctx::Null();
  }

  if (args[0]->GetPrimitiveType() != PrimitiveType::Array) {
    return Ctx::Null();
  }
  auto array = static_cast<Array_T *>(args[0].get());

  for (size_t i = 1; i < args.size(); i++) {
    array->Push(args[i]);
  }

  return Ctx::Null();
}

function(front) {
  if (args.size() == 0 || args[0]->GetPrimitiveType() != PrimitiveType::Array) {
    return Ctx::Null();
  }
  auto array = static_cast<Array_T *>(args[0].get());
  if (array->values.size() != 0) {
    return array->values.front();
  }
  return Ctx::Null();
}

function(back) {
  if (args.size() == 0 || args[0]->GetPrimitiveType() != PrimitiveType::Array) {
    return Ctx::Null();
  }
  auto array = static_cast<Array_T *>(args[0].get());
  if (array->values.size() != 0) {
    return array->values.back();
  }
  return Ctx::Null();
}

function(pop) {
  if (args.empty()) {
    return Ctx::Null();
  }

  if (args[0]->GetPrimitiveType() != PrimitiveType::Array) {
    return Ctx::Null();
  }
  auto array = static_cast<Array_T *>(args[0].get());
  return array->Pop();
}

function(len) {
  if (args.empty()) {
    return Ctx::Null();
  }

  Array array;
  if (Ctx::TryGetArray(args[0], array)) {
    return Int_T::New(array->values.size());
  }

  return Ctx::Null();
}

extern "C" ScritModDef *InitScritModule_std_SR_array() {
  ScritModDef *def = CreateModDef();
  *def->description = "provide functionality for the array type.";
  auto array = make_shared<ArrayType>();
  
  array->Set("remove", CREATE_CALLABLE(remove, "null", {"any", "any"}));
  array->Set("contains", CREATE_CALLABLE(contains, "bool", {"any", "any"}));
  array->Set("clear", CREATE_CALLABLE(clear, "null", {"any"}));
  array->Set("expand", CREATE_CALLABLE(expand, "array", {"any", "int"}));
  array->Set("push", CREATE_CALLABLE(push, "null", {"any", "any"}));
  array->Set("front", CREATE_CALLABLE(front, "any", {"any"}));
  array->Set("back", CREATE_CALLABLE(back, "any", {"any"}));
  array->Set("pop", CREATE_CALLABLE(pop, "any", {"any"}));
  array->Set("len", CREATE_CALLABLE(len, "int", {"any"}));
  
  def->AddType("array", array);
  def->SetNamespace("std::array");

  return def;
}