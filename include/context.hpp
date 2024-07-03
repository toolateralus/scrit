#pragma once

#include "ast.hpp"
#include <stdexcept>
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

namespace Values {
struct Value_T;
typedef shared_ptr<Values::Value_T> Value;

} // namespace Values
using namespace Values;

struct Scope_T;
typedef shared_ptr<Scope_T> Scope;
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
  auto Find(const std::string &name)
      -> std::_Rb_tree_iterator<
          std::pair<const Scope_T::Key, std::shared_ptr<Values::Value_T>>>;
  auto Contains(const string &name) -> bool;
  auto Erase(const string &name) -> size_t;
  auto Members() -> std::map<Key, Value> &;

  auto ForwardDeclare(const string &name, const Type &type,
                      const Mutability &mut) -> void;

  auto ClearVariables() -> void;

  // Used to patch fwd declared types during struct decls.
  auto OverwriteType(const string &name, const Type &type) {
    types[name] = type;
  }

  auto InsertType(const string &name, const Type &type) {
    if (TypeExists(name) && FindType(name) != nullptr) {
      throw std::runtime_error("re-definition of type: " + name);
    }
    types[name] = type;
  }
  auto FindType(const string &name) -> Type {
    if (types.contains(name)) {
      return types[name];
    }
    throw std::runtime_error("couldn't find type: " + name + " in this scope");
  }
  auto TypeExists(const string &name) -> bool { return types.contains(name); }

  auto Get(const string &name) -> Value;
  auto Set(const Key &name, Value value) -> void;
  auto Set(const string &name, Value value,
           const Mutability &mutability = Mutability::Const) -> void;

  auto Clear() -> void { variables.clear(); }
  auto Clone() -> Scope;
  auto PushModule(ScritModHandle &&handle) {
    module_handles.push_back(std::move(handle));
  }
  Scope_T(Scope_T *scope) { variables = scope->variables; }
  static auto Create() -> Scope { return std::make_shared<Scope_T>(); }
  static auto Create(Scope_T *scope) -> Scope {
    return std::make_shared<Scope_T>(scope);
  }
  auto end() { return variables.end(); }

  Scope_T(const Scope_T &) = delete;
  Scope_T(Scope_T &&) = delete;
  Scope_T &operator=(const Scope_T &) = delete;
  Scope_T &operator=(Scope_T &&) = delete;

private:
  std::vector<ScritModHandle> module_handles;
  std::map<string, Type> types = {};
  std::map<Key, Value> variables = {};
};

struct Namespace {
  explicit Namespace(string name) : name(std::move(name)) {}
  const string name;
  // every namespace has to have a root scope by default.
  vector<Scope> scopes = { make_shared<Scope_T>() };
};

using VarIter =  std::_Rb_tree_iterator<
    std::pair<const Scope_T::Key, std::shared_ptr<Values::Value_T>>>;

struct Context {
  Context();
  Context(const Context &) = default;
  Context(Context &&) = default;
  Context &operator=(const Context &) = default;
  Context &operator=(Context &&) = default;
  
  vector<shared_ptr<Namespace>> namespaces;
  shared_ptr<Namespace> current_namespace;
  
  Scope& ImmediateScope() {
    return current_namespace->scopes.back();
  }
  
  void CreateNamespace(const std::string &name) {
    namespaces.push_back(make_shared<Namespace>(name));
  }
  shared_ptr<Namespace> FindNamespace(const std::string &name) {
    auto it = std::find_if(namespaces.begin(), namespaces.end(), [name](const shared_ptr<Namespace> &_namespace) {
        if (_namespace->name == name) {
          return true;
        }
        return false;
    });
    if (it == namespaces.end()) {
      throw std::runtime_error("cannot find namespace " + name);
    }
    return *it;
  }
    
  
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

  void Reset();
};
