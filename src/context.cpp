#include "context.hpp"
#include "ast.hpp"
#include "type.hpp"
#include "value.hpp"
#include <stdexcept>
#include "ast_serializer.hpp"

Context::Context() {}



void Namespace::Erase(const string &name) {
  current_scope->Erase(name);
}

auto Namespace::FindIter(const string &name) const -> std::pair<VarIter, bool> {
  return current_scope->Find(name);
}

auto Namespace::GetValue(const string &name) const -> Value {
  auto [value, found] = current_scope->Find(name);
  
  if (found) {
    return value->second;
  }
  
  for (const auto &[_, importedNs] : imported_namespaces) {
    auto found = importedNs->GetValue(name);
    if (found)
      return found;
  }
  
  if (parent.lock())
    return parent.lock()->GetValue(name);
  
  return nullptr;
}


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
  current_scope->PushModule(ScritModHandle(handle));
}
ScritModHandle::ScritModHandle(ScritModHandle &&move) noexcept {
  this->handle = move.handle;
  move.handle = nullptr;
}


auto Namespace::FindType(const string &name) -> Type {
  return current_scope->FindType(name);
}

auto Namespace::TypeExists(const string &name) -> bool {
  return current_scope->TypeExists(name);
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



Context::ResolvedPath Context::Resolve(ScopeResolution *res) {
  auto ns = FindNamespace(res->identifiers);
  if (ns && ns->GetValue(res->identifiers.back())) {
    return {.value = ns->GetValue(res->identifiers.back())};
  } else if (ns) {
    return {._namespace = ns};
  }
  throw std::runtime_error("failed to resolive symbol :" + res->full_path);
}

auto Context::Find(const string &name) const -> Value {
  if (FunctionRegistry::Exists(name)) {
    return FunctionRegistry::GetCallable(name);
  }  
  
  auto result = current_namespace->GetValue(name);
  if (result) {
    return result;
  }
  return nullptr;
}


auto Scope_T::Find(const std::string &name) -> std::pair<VarIter, bool> {
  auto it =
      std::find_if(variables.begin(), variables.end(),
                   [&](const auto &pair) { return name == pair.first.value; });
  if (it == variables.end() && parent.lock()) {
    return parent.lock()->Find(name);
  }
  if (it == variables.end()) {
    return std::pair<VarIter, bool>(nullptr, false);
  }
  return std::pair<VarIter, bool>(it, true);
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
  
  if (parent.lock()) {
    return parent.lock()->FindType(name);
  }
  
  throw std::runtime_error("couldn't find type: " + name + " in this scope");
}
auto Scope_T::PushModule(ScritModHandle &&handle) -> void {
  module_handles.push_back(std::move(handle));
}
auto Scope_T::End() -> VarIter{ return variables.end(); }

auto Scope_T::Clone() -> Scope {
  std::map<Key, Value> variables = {};

  for (const auto &[k, v] : this->variables) {
    variables[k] = v->Clone();
  }
  
  auto scope = make_shared<Scope_T>(this->parent.lock());
  scope->variables = variables;
  return scope;
}
auto Scope_T::GetValue(const string &name) -> Value {
  auto [it, found] = Find(name);
  if (!found) {
    return Value_T::UNDEFINED;
  }
  return variables[it->first];
}

auto Scope_T::Assign(const string &name, Value value) -> void {
  auto it = std::find_if(variables.begin(), variables.end(), [name](const auto &pair) {
    return pair.first.value == name;
  });
  if (it == variables.end()) {
    if (parent.lock()) {
      return parent.lock()->Assign(name, value);
    }
    throw std::runtime_error("Cannot assign non-existent variable: " + name);
  }
  auto &[key, var] = *it;
  if (key.mutability == Mutability::Mut) {
    variables[key] = value;
    return;
  }
  throw std::runtime_error("Cannot set a const value.. identifier: " + name);
}

auto Scope_T::Contains(const string &name) -> bool {
  return Find(name).second;
}

auto Scope_T::Erase(const string &name) -> size_t {
  auto it = std::find_if(variables.begin(), variables.end(), [name](const auto &pair) {
    return pair.first.value == name;
  });
  if (it != variables.end()) {
    variables.erase(it);
    return 1;
  }
  return 0;
}

auto Scope_T::Members() -> std::map<Key, Value> & { return variables; }

string Context::StackFrame::ToString() {
  ASTSerializer visitor = {}; 
  if (!call || call->type == nullptr) {
    return "cannot print stack information.";
  }
  visitor.visit(call);
  return visitor.stream.str();
}

auto Scope_T::Declare(const string &name, Value value,
                      const Mutability &mutability) -> void {
  Erase(name); // erase any pre-existing variable with this signature.
  variables[Key(name, mutability)] = value;
}
                      
std::shared_ptr<Namespace>
Context::FindNamespace(const vector<std::string> &identifiers) {
  if (identifiers.empty()) {
    return nullptr;
  }

  std::shared_ptr<Namespace> ns = root_namespace;

  for (size_t i = 0; i < identifiers.size() && ns; ++i) {
    const std::string &currentPart = identifiers[i];
    if (ns->nested_namespaces.contains(currentPart)) {
      ns = ns->nested_namespaces[currentPart];
    } else {
      break;
    }
  }
  return ns;
}
bool Context::NamespaceExists(const vector<std::string> &identifiers) {
  return FindNamespace(identifiers) != nullptr;
}
