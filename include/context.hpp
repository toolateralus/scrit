#pragma once
#include <memory>
#include <vector>

using std::vector;
using std::string;
using std::shared_ptr;

struct Value_T;
struct Scope_T;

typedef shared_ptr<Value_T> Value;
typedef shared_ptr<Scope_T> Scope;

struct Context {
  Context();
  vector<Scope> scopes;
  Scope PushScope(Scope scope = nullptr);
  Scope PopScope();
  Value Find(const string &name);
  void Insert(const string &name, Value value);
};
