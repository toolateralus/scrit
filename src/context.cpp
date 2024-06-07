#include "context.hpp"
#include "value.hpp"



Context::Context() {
  scopes = {
      make_shared<Scope_T>(),
  };
}
Scope Context::PushScope(Scope scope) {
  if (scope == nullptr) {
    scope = std::make_shared<Scope_T>();
  }
  scopes.push_back(scope);
  return scope;
}
Scope Context::PopScope() {
  if (scopes.empty()) {
    throw std::runtime_error("Cannot pop: Scope stack is empty");
  }
  auto scope = scopes.back();
  scopes.pop_back();
  return scope;
}
void Context::Insert(const string &name, Value value) {
  for (const auto &scope : scopes) {
    for (auto &[key, var] : scope->Members()) {
      if (key == name) {
        var = value;
        return;
      }
    }
  }
  auto &variables = scopes.back()->Members();
  variables[name] = value;
}
Value Context::Find(const string &name) {
  for (const auto &scope : scopes) {
    for (const auto &[key, var] : scope->Members()) {
      if (key == name) {
        return var;
      }
    }
  }
  return nullptr;
}
void Context::Reset() {
  scopes.clear();
  PushScope();
}
auto Scope_T::Clone() -> Scope {
  std::map<string, Value> variables = {};
  for (const auto &[k, v] : this->variables) {
    variables[k] = v->Clone();
  }
  auto scope=make_shared<Scope_T>();
  scope->variables = variables;
  return scope;
}
auto Scope_T::Get(const string &name) -> Value {
  if (!variables.contains(name)) {
    return Value_T::UNDEFINED;
  }
  return variables[name];
  }
auto Scope_T::Set(const string &name, Value value) -> void {
   variables[name] = value; 
}
auto Scope_T::Contains(const string &name) -> bool {
  return variables.contains(name);
}
auto Scope_T::Erase(const string &name) -> size_t {
  return variables.erase(name); 
}
auto Scope_T::Members() -> std::map<string, Value>& {
  return variables;
}