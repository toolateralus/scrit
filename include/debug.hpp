#pragma once
#include <vector>
struct ASTNode;
struct Debug {
  Debug() = delete;
  static std::vector<int> breakpoints;
  static bool stepRequested;
  static void InsertBreakpoint(int loc);
  static void RemoveBreakpoint(int loc);
  static void Continue();
  static void Step() { stepRequested = true; }
  static void WaitForBreakpoint(ASTNode *owner, ASTNode *node);
};