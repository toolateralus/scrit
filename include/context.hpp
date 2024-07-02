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
    
    Key() = delete;
    Key(const std::string &value, const Mutability &mutability)
        : value(value), mutability(mutability) {}
        
    bool operator<(const Key &other) const { return value < other.value; }
    bool operator==(const Key &other) const {
      return value == other.value && mutability == other.mutability;
    }
  };
  Scope_T() {}
  auto Find(const std::string &name) -> std::_Rb_tree_iterator<
      std::pair<const Scope_T::Key, std::shared_ptr<Values::Value_T>>>;
  auto Contains(const string &name) -> bool;
  auto Erase(const string &name) -> size_t;
  auto Members() -> std::map<Key, Value> &;
  
  
  auto InsertType(const string &name, const Type &type) {
    if (TypeExists(name)) {
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
  auto TypeExists(const string &name) -> bool {
    return types.contains(name); 
  }
  
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
  
  Scope_T(const Scope_T&) = delete;
  Scope_T(Scope_T&&) = delete;
  Scope_T& operator=(const Scope_T&) = delete;
  Scope_T& operator=(Scope_T&&) = delete;

private:
  std::vector<ScritModHandle> module_handles;
  std::map<string, Type> types = {};
  std::map<Key, Value> variables = {};
};

typedef std::_Rb_tree_iterator<
    std::pair<const Scope_T::Key, std::shared_ptr<Values::Value_T>>>
    VarIter;

struct Context {
  Context();
  vector<Scope> scopes = {};
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
