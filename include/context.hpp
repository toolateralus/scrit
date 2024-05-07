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
  vector<std::shared_ptr<Scope_T>> scopes;
  std::shared_ptr<Scope_T> PushScope(std::shared_ptr<Scope_T> scope = nullptr);
  std::shared_ptr<Scope_T> PopScope();
  Value Find(const string &name);
  void Insert(const string &name, Value value);
};