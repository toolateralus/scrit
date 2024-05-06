#include "value.hpp"
#include "ast.hpp"
#include <memory>

shared_ptr<Bool_T> Value_T::True = make_shared<::Bool_T>(true);
shared_ptr<Bool_T> Value_T::False = make_shared<::Bool_T>(false);
Value Value_T::Null = make_shared<::Null>();
Value Value_T::Undefined = make_shared<::Undefined>();

Value Object_T::GetMember(const string &name) {
  return scope->variables[name];
}
Value Object_T::GetMember(unique_ptr<Identifier> &name) {
  return scope->variables[name->name];
}

void Object_T::SetMember(const string &name, Value &value) {
  scope->variables[name] = value;
}
void Object_T::SetMember(unique_ptr<Identifier> &name, Value &value) {
  scope->variables[name->name] = value;
}

Value Callable_T::Call(unique_ptr<Arguments>) {
  // TODO:
  return Null;
}