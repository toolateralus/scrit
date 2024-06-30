#include <iostream>
#include <scrit/native.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/type.hpp>

#define function(name) Value name(std::vector<Value> args)
#define undefined Ctx::Undefined()

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
    return Ctx::Undefined();
  }

  if (args[0]->GetPrimitiveType() == PrimitiveType::Array) {
    auto array = dynamic_cast<Array_T *>(args[0].get());
    array->values.clear();
  }
  return Ctx::Undefined();
}

function(expand) {
  if (args.size() < 2) {
    return Ctx::Undefined();
  }

  int value;
  if (args[0]->GetPrimitiveType() == PrimitiveType::Array &&
      Ctx::TryGetInt(args[1], value)) {
    auto array = dynamic_cast<Array_T *>(args[0].get());

    auto default_value = args.size() > 2 ? args[2] : Value_T::UNDEFINED;

    Callable_T *callable = nullptr;
    if (default_value->GetPrimitiveType() == PrimitiveType::Callable) {
      callable = static_cast<Callable_T *>(default_value.get());
    }

    std::vector<Value> empty = {};

    for (int i = 0; i < value; i++) {
      if (callable) {
        array->Push(callable->Call(empty));
      } else {
        array->Push(default_value);
      }
    }
    return args[0];
  }
  return Ctx::Undefined();
}

function(push) {
  if (args.empty()) {
    return Ctx::Undefined();
  }

  if (args[0]->GetPrimitiveType() != PrimitiveType::Array) {
    return Ctx::Undefined();
  }
  auto array = static_cast<Array_T *>(args[0].get());

  for (size_t i = 1; i < args.size(); i++) {
    array->Push(args[i]);
  }

  return Ctx::Undefined();
}

function(front) {
  if (args.size() == 0 || args[0]->GetPrimitiveType() != PrimitiveType::Array) {
    return Ctx::Undefined();
  }
  auto array = static_cast<Array_T *>(args[0].get());
  if (array->values.size() != 0) {
    return array->values.front();
  }
  return Ctx::Undefined();
}

function(back) {
  if (args.size() == 0 || args[0]->GetPrimitiveType() != PrimitiveType::Array) {
    return Ctx::Undefined();
  }
  auto array = static_cast<Array_T *>(args[0].get());
  if (array->values.size() != 0) {
    return array->values.back();
  }
  return Ctx::Undefined();
}

function(pop) {
  if (args.empty()) {
    return Ctx::Undefined();
  }

  if (args[0]->GetPrimitiveType() != PrimitiveType::Array) {
    return Ctx::Undefined();
  }
  auto array = static_cast<Array_T *>(args[0].get());
  return array->Pop();
}

function(len) {
  if (args.empty()) {
    return Ctx::Undefined();
  }

  Array array;
  if (Ctx::TryGetArray(args[0], array)) {
    return Int_T::New(array->values.size());
  }

  return Ctx::Undefined();
}


extern "C" ScritModDef *InitScritModule_array() {
  ScritModDef *def = CreateModDef();
  *def->description = "provide functionality for the array type.";
  
  auto array = TypeSystem::Current().Get("array");
  
  std::cout << "type: " << array->name << std::endl;
  
  array->Set("remove",   NativeFunctions::MakeCallable(remove));
  array->Set("contains", NativeFunctions::MakeCallable(contains));
  array->Set("clear",    NativeFunctions::MakeCallable(clear));
  array->Set("expand",   NativeFunctions::MakeCallable(expand));
  array->Set("push",     NativeFunctions::MakeCallable(push));
  array->Set("front",    NativeFunctions::MakeCallable(front));
  array->Set("back",     NativeFunctions::MakeCallable(back));
  array->Set("pop",      NativeFunctions::MakeCallable(pop));
  array->Set("len",      NativeFunctions::MakeCallable(len));
  
  std::cout << "new size: " <<  TypeSystem::Current().Get("array")->Scope().Members().size() << std::endl;
  
  return def;
}