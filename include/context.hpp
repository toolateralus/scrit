#pragma once
#include <map>
#include <memory>
#include <vector>

using std::vector;
using std::string;
using std::shared_ptr;

namespace Values {
  struct Value_T;
  typedef shared_ptr<Values::Value_T> Value;
  
}
using namespace Values;

struct Scope_T;
  typedef shared_ptr<Scope_T> Scope;

  struct Scope_T {
    std::map<string, Value > variables = {};
  };
  
  struct Context {
    Context();
    vector<Scope> scopes;
    Scope PushScope(Scope scope = nullptr);
    Scope PopScope();
    Value Find(const string &name);
    void Insert(const string &name, Value value);
    void Reset();
  };