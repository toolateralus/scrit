#include <scrit/scritmod.hpp>

#define function(name) Value name (std::vector<Value> args) 
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
  if (args[0]->GetType() == Values::ValueType::Array) {
    int i;
    Array_T *a = static_cast<Array_T *>(args[0].get());
    if (args[1]->GetType() == Values::ValueType::Callable) {
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
extern "C" ScritModDef* InitScritModule_array() {
  ScritModDef *def = CreateModDef();
  *def->description = "your description here";
  
  def->AddFunction("remove", remove);
  def->AddFunction("contains", contains);
  return def;
}