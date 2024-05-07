#include "native.hpp"

std::unordered_map<std::string, NativeFunctionPtr> &
NativeFunctions::GetRegistry() {
  static std::unordered_map<std::string, NativeFunctionPtr> reg;
  return reg;
}
bool NativeFunctions::Exists(const std::string &name) {
  return GetRegistry().contains(name);
}
void RegisterFunction(const std::string &name,
                             NativeFunctionPtr function) {
  NativeFunctions::GetRegistry()[name] = function;
}
