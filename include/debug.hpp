#pragma once
#include <string>
#include <vector>
struct ASTNode;

enum struct StepKind {
  None,
  Over,
  In,
  Out,
};

struct Breakpoint {
  int loc = -1;
  bool isTemporary = false;
};

struct Debug {
  Debug() = delete;
  static std::vector<Breakpoint> breakpoints;
  static StepKind requestedStep;
  
  static ASTNode *lastNode;
  static int stepOutIndex;
  
  static void InsertBreakpoint(const int &loc, const bool isTemporary);
  static void Continue() { requestedStep = StepKind::None; }
  static void StepOver() { requestedStep = StepKind::Over; }
  static void StepIn()   { requestedStep = StepKind::In;   }
  static void StepOut()  { requestedStep = StepKind::Out;  }
  static void RemoveBreakpoint(const int &loc, const bool isTemporary);
  
  static void m_hangUpOnBreakpoint(ASTNode *owner, ASTNode *node);
  private:
  static void m_printScope();
  static void m_stepOut();
  static void m_getInfo(ASTNode *&owner, ASTNode *&node);
  static void m_stepOver(ASTNode *&owner, ASTNode *&node);
  static void m_stepIn(ASTNode *&owner, ASTNode *&node);
  static void m_setBreakpoint(std::string &line, const std::string &breakpointKey);
};