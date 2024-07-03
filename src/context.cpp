#include "context.hpp"
#include "ast.hpp"
#include "type.hpp"
#include "value.hpp"
#include <stdexcept>

Context::Context() {}

Scope Namespace::PushScope(Scope scope) {
  if (scope == nullptr) {
    scope = std::make_shared<Scope_T>();
  }
  scopes.push_back(scope);
  return scope;
}
Scope Namespace::PopScope() {
  if (scopes.empty()) {
    throw std::runtime_error("Cannot pop: Scope stack is empty");
  }
  auto scope = scopes.back();
  scopes.pop_back();
  return scope;
}

void Namespace::Erase(const string &name) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    if ((*it)->Contains(name)) {
      (*it)->Erase(name);
      return;
    }
  }
}

void Namespace::Insert(const Scope_T::Key &key, Value value) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    if ((*it)->Contains(key.value)) {
      (*it)->Set(key, value);
      return;
    }
  }
  scopes.back()->Set(key, value);
}

void Namespace::Insert(const string &name, Value value,
                     const Mutability &mutability) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    if ((*it)->Contains(name)) {
      (*it)->Set(name, value, mutability);
      return;
    }
  }
  scopes.back()->Set(name, value, mutability);
}

auto Namespace::FindIter(const string &name) const -> VarIter {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    if ((*it)->Contains(name)) {
      return (*it)->Find(name);
    }
  }
  return scopes.back()->end();
}

auto Namespace::Find(const string &name) const -> Value {
  // First, search in the current namespace's scopes
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    for (const auto &[key, var] : (*it)->Members()) {
      if (key.value == name) {
        if (var->GetPrimitiveType() == PrimitiveType::Lambda) {
          if (auto lambda = std::dynamic_pointer_cast<Lambda_T>(var)) {
            return lambda->Evaluate();
          }
        }
        return var;
      }
    }
  }

  for (const auto& [_, importedNs] : imported_namespaces) {
    auto found = importedNs->Find(name);
    if (found) 
      return found;
  }
  

  
  return nullptr;
}

auto Scope_T::Clone() -> Scope {
  std::map<Key, Value> variables = {};

  for (const auto &[k, v] : this->variables) {
    variables[k] = v->Clone();
  }

  auto scope = make_shared<Scope_T>();
  scope->variables = variables;
  return scope;
}
auto Scope_T::Get(const string &name) -> Value {
  auto it = Find(name);
  if (it == variables.end()) {
    return Value_T::UNDEFINED;
  }
  return variables[it->first];
}

auto Scope_T::Set(const Scope_T::Key &key, Value value) -> void {
  variables[key] = value;
}

auto Scope_T::Set(const string &name, Value value,
                  const Mutability &mutability) -> void {
  if (TypeSystem::Current().Exists(name)) {
    throw std::runtime_error("cannot declare a variable of an existing type: " +
                             name);
  }

  auto it = Find(name);
  auto &[key, var] = *it;

  // variable didn't exist, we freely declare it.
  if (it == variables.end()) {
    variables[Key(name, mutability, false)] = value;
    return;
  }

  // a forward declaration is being fulfilled.
  if (key.forward_declared) {
    auto new_key = Key(key.value, key.mutability, false);
    variables[new_key] = value;
    return;
  }

  // the variable is being assigned,
  if (key.mutability == Mutability::Mut) {
    variables[key] = value;
    return;
  }

  throw std::runtime_error("Cannot set a const value.. identifier: " + name);
}
auto Scope_T::Contains(const string &name) -> bool {
  return Find(name) != variables.end();
}
auto Scope_T::Erase(const string &name) -> size_t {
  auto it = Find(name);
  if (it != variables.end()) {
    variables.erase(it);
    return 1;
  }
  return 0;
}
auto Scope_T::Members() -> std::map<Key, Value> & { return variables; }

ScritModHandle::~ScritModHandle() noexcept {
  // was moved or already disposed.
  if (handle == nullptr) {
    return;
  }
  dlclose(handle);
  handle = nullptr;
}
ScritModHandle::ScritModHandle(void *handle) noexcept : handle(handle) {}

void Namespace::RegisterModuleHandle(void *handle) {
  scopes.back()->PushModule(ScritModHandle(handle));
}
ScritModHandle::ScritModHandle(ScritModHandle &&move) noexcept {
  this->handle = move.handle;
  move.handle = nullptr;
}

auto Scope_T::Find(const std::string &name) -> VarIter {
  auto it =
      std::find_if(variables.begin(), variables.end(),
                   [&](const auto &pair) { return name == pair.first.value; });

  return it;
}

auto Namespace::FindType(const string &name) -> Type {
  for (auto it = scopes.rbegin(); it != scopes.rend(); it++) {
    auto scope = *it;
    if (scope->TypeExists(name)) {
      return scope->FindType(name);
    }
  }
  throw std::runtime_error("couldn't find type " + name);
}

auto Namespace::TypeExists(const string &name) -> bool {
  for (auto it = scopes.rbegin(); it != scopes.rend(); it++) {
    auto scope = *it;
    if (scope->TypeExists(name)) {
      return true;
    }
  }
  return false;
}
auto Scope_T::ClearVariables() -> void {
  auto it = variables.begin();
  while (it != variables.end()) {
    if (it->second->GetPrimitiveType() != Values::PrimitiveType::Callable) {
      it = variables.erase(it);
    } else {
      ++it;
    }
  }
}

auto Scope_T::ForwardDeclare(const string &name, const Type &type,
                             const Mutability &mut) -> void {
  auto val = make_shared<Undefined_T>();
  val->type = type;
  variables[Key(name, mut, true)] = val;
}

Context::ResolvedPath Context::Resolve(ScopeResolution *res) {
  auto ns = FindNamespace(res->identifiers);
  if (ns && ns->Find(res->identifiers.back())) {
    return { .value = ns->Find(res->identifiers.back()) };
  } else if (ns) {
    return { ._namespace = ns };
  }
  throw std::runtime_error("failed to resolive symbol :" + res->full_path);
}

auto Context::Find(const string &name) const -> Value {
  auto result = current_namespace->Find(name);
  if (!result && FunctionRegistry::Exists(name)) {
    return FunctionRegistry::GetCallable(name);
  }
  return result;
}
