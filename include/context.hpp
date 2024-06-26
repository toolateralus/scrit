#pragma once

#include "ast.hpp"
#ifdef __linux__
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#include <map>
#include <memory>
#include <vector>
#include <string>

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
struct ScritModHandle  {
  void *handle;
  ScritModHandle() noexcept  = delete;
  ScritModHandle(void *handle) noexcept;
  
  ScritModHandle(const ScritModHandle &copy) noexcept = delete;
  ScritModHandle(ScritModHandle &&move) noexcept;
  
  ScritModHandle &operator=(const ScritModHandle&) noexcept = delete;
  ScritModHandle &operator=(ScritModHandle &&other) noexcept;
  ~ScritModHandle() noexcept ;
};

struct Scope_T {
  struct Key {
    const std::string value;
    const Mutability mutability;
    Key(const std::string &value, const Mutability &mutability) : value(value), mutability(mutability){}
    bool operator<(const Key& other) const {
      return value < other.value;
    }
    bool operator==(const Key& other) const {
      return value == other.value && mutability == other.mutability;
    }
    
  };
  Scope_T() {}
  
  ~Scope_T() {
    module_handles.clear();
  }
  
  auto Find(const std::string &name) -> std::_Rb_tree_iterator<std::pair<const Scope_T::Key, std::shared_ptr<Values::Value_T>>>;
  
  auto Contains(const string &name) -> bool;
  auto Erase(const string &name) -> size_t;
  auto Members() -> std::map<Key, Value>&;
  auto Get(const string &name) -> Value;
  
  auto Set(const Key &name, Value value) -> void;
  
  auto Set(const string &name, Value value, const Mutability &mutability = Mutability::Const) -> void;
  auto Clear() -> void {
    this->variables.clear();
  }
  auto Clone() -> Scope;
  auto PushModule(ScritModHandle &&handle) {
    module_handles.push_back(std::move(handle));
  }
  Scope_T(Scope_T *scope) {
    variables = scope->variables;
  } 
  
  static auto create() -> Scope {
    return std::make_shared<Scope_T>();
  }
  static auto create(Scope_T *scope) -> Scope {
    return std::make_shared<Scope_T>(scope);
  }
  
  auto end() {
    return variables.end();
  }
  
  private:
  std::vector<ScritModHandle> module_handles;
  std::map<Key, Value> variables = {};
};


struct Context {
  Context();
  vector<Scope> scopes = {};
  void RegisterModuleHandle(void *handle);
  Scope PushScope(Scope scope = nullptr);
  Scope PopScope();
  auto Find(const string &name) const -> Value;
  auto FindIter(const string &name) const -> std::_Rb_tree_iterator<std::pair<const Scope_T::Key, std::shared_ptr<Values::Value_T>>>;
  void Insert(const string &name, Value value, const Mutability &mutability);
  void Reset();
};
