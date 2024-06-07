#pragma once
#include <dlfcn.h>
#include <map>
#include <memory>
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
  ScritModHandle(ScritModHandle *copy) = delete;
  ScritModHandle(ScritModHandle &copy) = delete;
  ScritModHandle(ScritModHandle &&move);
  ScritModHandle() = delete;
  void *handle;
  ScritModHandle(void *handle);
  ~ScritModHandle();
};

struct Scope_T {
  Scope_T() {}
  auto Contains(const string &name) -> bool;
  auto Erase(const string &name) -> size_t;
  auto Members() -> std::map<string, Value>&;
  auto Get(const string &name) -> Value;
  auto Set(const string &name, Value value) -> void;
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
  private:
  std::vector<ScritModHandle> module_handles;
  std::map<string, Value> variables = {};
};

struct Context {
  Context();
  vector<Scope> scopes;
  void RegisterModuleHandle(void *handle);
  Scope PushScope(Scope scope = nullptr);
  Scope PopScope();
  Value Find(const string &name);
  void Insert(const string &name, Value value);
  void Reset();
};
