#pragma once
struct ASTNode;
struct Debug {
  Debug() = delete;
  static int currentBreakpoint;
  static bool stepRequested;
  static void InsertBreakpoint(int loc);
  static void Continue();
  static void Step() { stepRequested = true; }
  static void WaitForBreakpoint(ASTNode *node);
};