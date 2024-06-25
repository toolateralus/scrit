#pragma once

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


struct Scope_T {
  Scope_T() {}
  
  // Why did i randomly start using this convention? Not sure.
  // I'd like to use it more.
  auto Contains(const string &name) -> bool;
  auto Erase(const string &name) -> size_t;
  auto Members() -> std::map<string, Value>&;
  auto Get(const string &name) -> Value;
  auto Set(const string &name, Value value) -> void;
  auto Clear() -> void {
    this->variables.clear();
  }
  auto Clone() -> Scope;
  Scope_T(Scope_T *scope) {
    variables = scope->variables;
  } 
  private:
  std::map<string, Value> variables = {};
};

struct Context {
  Context();
  vector<Scope> scopes = {};
  Scope PushScope(Scope scope = nullptr);
  Scope PopScope();
  Value Find(const string &name);
  void Insert(const string &name, Value value);
  void Reset();
};
