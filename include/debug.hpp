#pragma once
#include <string>
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
  
  static void m_setBreakpoint(std::string &line, const std::string &breakpointKey);
  static void m_printScope();
  static void m_stepOver(ASTNode *&owner, ASTNode *&node);
  
  static void WaitForBreakpoint(ASTNode *owner, ASTNode *node);
};