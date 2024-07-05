#pragma once

#include "ast.hpp"
#include <iostream>
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
    Key() = delete;
    Key(const std::string &value, const Mutability &mutability)
        : value(value), mutability(mutability) {}
    bool operator<(const Key &other) const { return value < other.value; }
    bool operator==(const Key &other) const {
      return value == other.value && mutability == other.mutability;
    }
  };
  
  Scope_T(Scope scope) {
    parent = scope;
  }
  
  Scope_T(const Scope_T &) = delete;
  Scope_T(Scope_T &&) = delete;
  Scope_T &operator=(const Scope_T &) = delete;
  Scope_T &operator=(Scope_T &&) = delete;
  
  std::weak_ptr<Scope_T> parent;
  
  ~Scope_T() {
    variables.clear();
    types.clear();
  }
  
  using VarIter = std::_Rb_tree_iterator<
          std::pair<const Scope_T::Key, std::shared_ptr<Values::Value_T>>>;
  
  auto Find(const std::string &name)
      -> std::pair<VarIter, bool>;
      
  auto Contains(const string &name) -> bool;
  auto Erase(const string &name) -> size_t;
  auto Members() -> std::map<Key, Value> &;
  
  auto ClearVariables() -> void;
  
  // Used to patch fwd declared types during struct decls.
  auto OverwriteType(const string &name, const Type &type) -> void;
  auto InsertType(const string &name, const Type &type) -> void;
  auto FindType(const string &name) -> Type;
  auto TypeExists(const string &name) -> bool { 
    auto exists = types.contains(name);
    if (!exists && parent.lock()){
      return parent.lock()->TypeExists(name);
    }
    return exists;
  }
  
  auto GetValue(const string &name) -> Value;
  
  auto Assign(const string &name, Value value) -> void;
  auto Declare(const string &name, Value value, const Mutability &mutability)
      -> void;

  auto Clear() -> void { variables.clear(); }
  auto Clone() -> Scope;
  auto PushModule(ScritModHandle &&handle) -> void;
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
      : name(name), parent(parent) {
        current_scope = root_scope;
      }
  
  const string name;
  
  Scope root_scope = make_shared<Scope_T>(nullptr);
  Scope current_scope;
  
  
  Scope CreateScope() {
    return make_shared<Scope_T>(this->current_scope);
  }
  void SetCurrentScope(Scope scope) {
    current_scope = scope;
  }
  
  std::weak_ptr<Namespace> parent;
  std::unordered_map<string, shared_ptr<Namespace>> imported_namespaces;
  std::unordered_map<string, shared_ptr<Namespace>> nested_namespaces;

  void RegisterModuleHandle(void *handle);
  void Erase(const string &name);
  auto TypeExists(const string &name) -> bool;
  auto FindType(const string &name) -> Type;
  
  auto GetValue(const string &name) const -> Value;
  auto FindIter(const string &name) const -> std::pair<VarIter, bool>;
  
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
  
  
  Scope &CurrentScope() { return current_namespace->current_scope; }
  Scope CreateScope() {
    return current_namespace->CreateScope();
  }
  void SetCurrentScope(Scope scope) {
    current_namespace->current_scope = scope;
  }

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

  bool NamespaceExists(const vector<std::string> &identifiers);
  std::shared_ptr<Namespace>
  FindNamespace(const vector<std::string> &identifiers);

  void RegisterModuleHandle(void *handle) {
    current_namespace->RegisterModuleHandle(handle);
  }
  
  void Erase(const string &name) { current_namespace->Erase(name); }
  
  auto TypeExists(const string &name) -> bool {
    return current_namespace->TypeExists(name);
  }
  auto FindType(const string &name) -> Type {
    return current_namespace->FindType(name);
  }
  auto GetValue(const vector<string> &identifiers) -> Value {
    vector<string> _namespace = identifiers;
    _namespace.pop_back();
    auto ns = FindNamespace(_namespace);

    if (!ns) {
      throw std::runtime_error("couldn't find namespace");
    }

    return ns->GetValue(identifiers.back());
  }
  
 
  
  auto Find(const string &name) const -> Value;
  auto FindIter(const string &name) const -> std::pair<VarIter, bool> {
    return current_namespace->FindIter(name);
  }
  
  
  auto PushStackFrame(Call *call) {
    call_stack.push_back({call});
  }
  auto PopStackFrame() {
    call_stack.pop_back();
  }
  auto PrintLastCall() {
    if (!call_stack.empty())
      std::cout << call_stack.back().ToString() << std::endl;
  }
  auto UnwindCallStack() {
    while (!call_stack.empty()) {
      std::cout << call_stack.back().ToString() << std::endl;
      call_stack.pop_back();
    }
  }
  
  private: 
  struct StackFrame {
    Call *call;
    
    string ToString();
  };
  vector<StackFrame> call_stack = {};
};
