#pragma once
#include <memory>
#include <vector>
struct Value_T;
typedef std::shared_ptr<Value_T> Value;

struct Scope_T;
typedef std::shared_ptr<Scope_T> Scope;

using std::vector;
using std::string;

struct Context {
  Context();
  vector<Scope> scopes;
  Scope PushScope(Scope scope = nullptr);
  Scope PopScope();
  Value Find(const string &name);
  void Insert(const string &name, Value value);
};
