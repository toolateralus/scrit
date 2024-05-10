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

struct Debug {
  Debug() = delete;
  static std::vector<int> breakpoints;
  static StepKind requestedStep;
  
  static ASTNode *lastSteppedIntoNode;
  static int lastSteppedIntoStatementIndex;
  
  static void InsertBreakpoint(int loc);
  static void RemoveBreakpoint(int loc);
  static void Continue();
  static void StepOver() { requestedStep = StepKind::Over; }
  static void StepIn() { requestedStep = StepKind::In; }
  static void StepOut() { requestedStep = StepKind::Out; }
  
  static void m_setBreakpoint(std::string &line, const std::string &breakpointKey);
  static void m_printScope();
  static void m_stepOver(ASTNode *&owner, ASTNode *&node);
  static void m_stepOut();
  static void m_stepIn(ASTNode *&owner, ASTNode *&node);
  static void WaitForBreakpoint(ASTNode *owner, ASTNode *node, const int &statementIndex);
};