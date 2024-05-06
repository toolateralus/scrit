#pragma once
#include "value.hpp"
#include <map>
#include <stdexcept>

struct Scope {
  std::map<string, Value > variables = {};
};

struct Context {
  std::vector<std::shared_ptr<Scope>> scopes = {};

  std::shared_ptr<Scope> PushScope(std::shared_ptr<Scope> scope = nullptr) {
    if (scope == nullptr) {
      scope = std::make_shared<Scope>();
    }
    scopes.push_back(scope);
    return scope;
  }

  std::shared_ptr<Scope> PopScope() {
    if (scopes.empty()) {
      throw std::runtime_error("Cannot pop: Scope stack is empty");
    }
    auto scope = scopes.back();
    scopes.pop_back();
    return scope;
  }

  Value Find(string &name) {
    for (const auto &scope : scopes) {
      for (const auto &[key, var] : scope->variables) {
        if (key == name) {
          return var;
        }
      }
    }
    return nullptr;
  }

  void Insert(string &name, Value value) {
    for (const auto &scope : scopes) {
      for (auto &[key, var] : scope->variables) {
        if (key == name) {
          var = value;
          return;
        }
      }
    }
    auto &variables = scopes.back()->variables;
    variables[name] = value;
  }
};