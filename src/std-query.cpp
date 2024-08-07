#include "context.hpp"
#include "ctx.hpp"
#include "native.hpp"
#include "type.hpp"
#include "value.hpp"
#include <vector>

#pragma clang diagnostic ignored "-Wunused-parameter"

#define undefined Ctx::Null()
#define null Ctx::Null()

REGISTER_FUNCTION(where, "array", {"any", "any"}) {
  if (args.size() < 2 ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::Array ||
      args[1]->GetPrimitiveType() != Values::PrimitiveType::Callable) {
    return undefined;
  }
  Array_T *a = static_cast<Array_T *>(args[0].get());
  Callable_T *c = static_cast<Callable_T *>(args[1].get());
  std::vector<Value> values;
  std::vector<Value> c_args;
  for (const auto &element : a->values) {
    c_args = {element};
    if (c->Call(c_args)->Equals(Value_T::True)) {
      values.push_back(element);
    }
  }
  return Ctx::CreateArray(values);
}
