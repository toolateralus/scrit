#include "context.hpp"
#include "ast.hpp"
#include "type.hpp"
#include "value.hpp"
#include <stdexcept>

Context::Context() {}



void Namespace::Erase(const string &name) {
  root_scope->Erase(name);
}

void Namespace::Insert(const Scope_T::Key &key, Value value) {
  root_scope->Set(key, value);
}

void Namespace::Insert(const string &name, Value value,
                       const Mutability &mutability) {
  root_scope->Set(name, value, mutability);
}

auto Namespace::FindIter(const string &name) const -> VarIter {
  return root_scope->Find(name);
}

auto Namespace::Find(const string &name) const -> Value {
  auto value = root_scope->Find(name);
  
  if (value != root_scope->End()) {
    return value->second;
  }
  
  for (const auto &[_, importedNs] : imported_namespaces) {
    auto found = importedNs->Find(name);
    if (found)
      return found;
  }
  
  // TODO: search parent ns's

  return nullptr;
}

auto Scope_T::Clone() -> Scope {
  std::map<Key, Value> variables = {};

  for (const auto &[k, v] : this->variables) {
    variables[k] = v->Clone();
  }
  
  auto scope = make_shared<Scope_T>(this->parent.lock());
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
    Erase(key.value);
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
  root_scope->PushModule(ScritModHandle(handle));
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
  return root_scope->FindType(name);
}

auto Namespace::TypeExists(const string &name) -> bool {
  return root_scope->TypeExists(name);
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
    return {.value = ns->Find(res->identifiers.back())};
  } else if (ns) {
    return {._namespace = ns};
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
auto Scope_T::InsertType(const string &name, const Type &type) -> void {
  if (TypeExists(name) && FindType(name) != nullptr) {
    throw std::runtime_error("re-definition of type: " + name);
  }
  types[name] = type;
}
auto Scope_T::OverwriteType(const string &name, const Type &type) -> void {
  types[name] = type;
}
auto Scope_T::FindType(const string &name) -> Type {
  if (types.contains(name)) {
    return types[name];
  }
  throw std::runtime_error("couldn't find type: " + name + " in this scope");
}
auto Scope_T::PushModule(ScritModHandle &&handle) -> void {
  module_handles.push_back(std::move(handle));
}
auto Scope_T::End() -> VarIter{ return variables.end(); }

