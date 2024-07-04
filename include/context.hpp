#pragma once

#include "ast.hpp"
#include <stdexcept>
#include <unordered_map>
#ifdef __linux__
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#include <map>
#include <memory>
#include <string>
#include <vector>

using std::shared_ptr;
using std::string;
using std::vector;


// ast node for iden::other::iden
struct ScopeResolution;
  
namespace Values {
  struct Value_T;
  typedef shared_ptr<Values::Value_T> Value; 
} // namespace Values
using namespace Values;

struct Scope_T;
using Scope = shared_ptr<Scope_T>;

struct ScritModHandle {
  void *handle;
  ScritModHandle() noexcept = delete;
  ScritModHandle(void *handle) noexcept;

  ScritModHandle(const ScritModHandle &copy) noexcept = delete;
  ScritModHandle(ScritModHandle &&move) noexcept;

  ScritModHandle &operator=(const ScritModHandle &) noexcept = delete;
  ScritModHandle &operator=(ScritModHandle &&other) noexcept;
  ~ScritModHandle() noexcept;
};

struct Scope_T {
  struct Key {
    const std::string value;
    const Mutability mutability;
    bool forward_declared;
    Key() = delete;
    Key(const std::string &value, const Mutability &mutability,
        const bool forward_declared = false)
        : value(value), mutability(mutability),
          forward_declared(forward_declared) {}

    bool operator<(const Key &other) const { return value < other.value; }
    bool operator==(const Key &other) const {
      return value == other.value && mutability == other.mutability;
    }
  };
  
  Scope_T() {}
  
  Scope_T(const Scope_T &) = delete;
  Scope_T(Scope_T &&) = delete;
  Scope_T &operator=(const Scope_T &) = delete;
  Scope_T &operator=(Scope_T &&) = delete;
  Scope_T(Scope_T *scope);
  
  ~Scope_T() {
    variables.clear();
    types.clear();
  }
  
  using VarIter = std::_Rb_tree_iterator<
          std::pair<const Scope_T::Key, std::shared_ptr<Values::Value_T>>>;
  
  auto Find(const std::string &name)
      -> VarIter;
      
  auto Contains(const string &name) -> bool;
  auto Erase(const string &name) -> size_t;
  auto Members() -> std::map<Key, Value> &;

  auto ForwardDeclare(const string &name, const Type &type,
                      const Mutability &mut) -> void;

  auto ClearVariables() -> void;
  
  // Used to patch fwd declared types during struct decls.
  auto OverwriteType(const string &name, const Type &type) -> void;
  auto InsertType(const string &name, const Type &type) -> void;
  auto FindType(const string &name) -> Type;
  auto TypeExists(const string &name) -> bool { return types.contains(name); }
  
  auto Get(const string &name) -> Value;
  auto Set(const Key &name, Value value) -> void;
  auto Set(const string &name, Value value,
           const Mutability &mutability = Mutability::Const) -> void;
  
  auto Clear() -> void { variables.clear(); }
  auto Clone() -> Scope;
  auto PushModule(ScritModHandle &&handle) -> void;
  static auto Create() -> Scope;
  static auto Create(Scope_T *scope) -> Scope;
  auto End() -> VarIter;
  

private:
  std::vector<ScritModHandle> module_handles;
  std::map<string, Type> types = {};
  std::map<Key, Value> variables = {};
};

using VarIter = Scope_T::VarIter;

struct Namespace {
  Namespace(const Namespace &) = default;
  Namespace(Namespace &&) = default;
  Namespace &operator=(const Namespace &) = delete;
  Namespace &operator=(Namespace &&) = delete;

  explicit Namespace(string name, shared_ptr<Namespace> parent)
      : name(name), parent(parent) {}

  const string name;
  // every namespace has to have a root scope by default.
  vector<Scope> scopes = {make_shared<Scope_T>()};

  std::weak_ptr<Namespace> parent;
  std::unordered_map<string, shared_ptr<Namespace>> imported_namespaces;
  std::unordered_map<string, shared_ptr<Namespace>> nested_namespaces;

  void RegisterModuleHandle(void *handle);
  Scope PushScope(Scope scope = nullptr);
  Scope PopScope();
  void Erase(const string &name);
  auto TypeExists(const string &name) -> bool;
  auto FindType(const string &name) -> Type;
  auto Find(const string &name) const -> Value;
  auto FindIter(const string &name) const -> VarIter;
  void Insert(const string &name, Value value, const Mutability &mutability);
  void Insert(const Scope_T::Key &key, Value value);

  static std::vector<std::string> split(const std::string &input) {
    std::vector<std::string> result;
    std::string::size_type start = 0;
    auto end = input.find("::");
    while (end != std::string::npos) {
      result.push_back(input.substr(start, end - start));
      start = end + 2; // Skip over the delimiter
      end = input.find("::", start);
    }
    result.push_back(input.substr(start));
    return result;
  }
};

struct Context {
  Context();
  Context(const Context &) = default;
  Context(Context &&) = default;
  Context &operator=(const Context &) = default;
  Context &operator=(Context &&) = default;

  shared_ptr<Namespace> root_namespace =
      make_shared<Namespace>("global", nullptr);

  shared_ptr<Namespace> current_namespace = root_namespace;

  Scope &ImmediateScope() { return current_namespace->scopes.back(); }

  void CreateNamespace(const vector<string> &identifiers) {
    shared_ptr<Namespace> current = root_namespace;

    for (const auto &id : identifiers) {
      if (!current->nested_namespaces.count(id)) {
        current->nested_namespaces[id] = make_shared<Namespace>(id, current);
      }
      current = current->nested_namespaces[id];
    }
  }

  void ImportNamespace(const vector<string> &identifiers) {
    auto nsToImport = FindNamespace(identifiers);
    if (!nsToImport) {
      throw std::runtime_error("Namespace to import does not exist.");
    }

    string nsName = identifiers.back();
    if (current_namespace->imported_namespaces.count(nsName) == 0) {
      current_namespace->imported_namespaces[nsName] = nsToImport;
    }
  }

  void SetCurrentNamespace(const vector<string> &identifiers) {
    shared_ptr<Namespace> current = root_namespace;

    for (const auto &id : identifiers) {
      if (!current->nested_namespaces.count(id)) {
        throw std::runtime_error("Namespace does not exist: " + id);
      }
      current = current->nested_namespaces[id];
    }
    current_namespace = current;
  }

  struct ResolvedPath {
    Value value = nullptr;
    shared_ptr<Namespace> _namespace = nullptr;
  };

  ResolvedPath Resolve(ScopeResolution *res);

  bool NamespaceExists(const vector<std::string> &identifiers) {
    return FindNamespace(identifiers) != nullptr;
  }
  std::shared_ptr<Namespace>
  FindNamespace(const vector<std::string> &identifiers) {
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

  void RegisterModuleHandle(void *handle) {
    current_namespace->RegisterModuleHandle(handle);
  }
  Scope PushScope(Scope scope = nullptr) {
    return current_namespace->PushScope(scope);
  }
  Scope PopScope() { return current_namespace->PopScope(); }
  void Erase(const string &name) { current_namespace->Erase(name); }
  auto TypeExists(const string &name) -> bool {
    return current_namespace->TypeExists(name);
  }
  auto FindType(const string &name) -> Type {
    return current_namespace->FindType(name);
  }
  auto Find(const vector<string> &identifiers) -> Value {
    vector<string> _namespace = identifiers;
    _namespace.pop_back();
    auto ns = FindNamespace(_namespace);

    if (!ns) {
      throw std::runtime_error("couldn't find namespace");
    }

    return ns->Find(identifiers.back());
  }
  auto Find(const string &name) const -> Value;
  auto FindIter(const string &name) const -> VarIter {
    return current_namespace->FindIter(name);
  }
  void Insert(const string &name, Value value, const Mutability &mutability) {
    current_namespace->Insert(name, value, mutability);
  }
  void Insert(const Scope_T::Key &key, Value value) {
    current_namespace->Insert(key, value);
  }
};
