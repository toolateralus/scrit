#include "debug.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "value.hpp"
#include <algorithm>
#include <gtest/gtest_prod.h>
#include <stdexcept>
#include <sys/select.h>

std::vector<int> Debug::breakpoints = {};
auto Debug::requestedStep = StepKind::None;

void Debug::m_setBreakpoint(std::string &line, const string &breakpointKey) {
  string num = "";
  for (int i = breakpointKey.length(); i < line.length(); ++i) {
    num += line[i];
  }
  int index = std::stoi(num);
  Debug::InsertBreakpoint(index);
}
void Debug::m_printScope() {
  auto scope = ASTNode::context.scopes.back();
  if (scope->variables.size() == 0) {
    std::cout << "scope empty." << '\n';
  }
  for (const auto &[key, var] : scope->variables) {
    std::cout << key << " :" << var->ToString() << "\n";
  }
  std::cout << std::flush;
}

void Debug::m_stepOver(ASTNode *&owner, ASTNode *&node) {
  if (auto program = dynamic_cast<Program *>(owner)) {
    bool hitSelf = false;
    for (const auto &statement : program->statements) {
      if (statement.get() == node) {
        hitSelf = true;
        continue;
      }
      if (hitSelf) {
        InsertBreakpoint(statement->srcInfo.loc);
        break;
      }
    }
  } else if (auto block = dynamic_cast<Block *>(owner)) {
    bool hitSelf = false;
    for (const auto &statement : block->statements) {
      if (statement.get() == node) {
        hitSelf = true;
        continue;
      }
      if (hitSelf) {
        InsertBreakpoint(statement->srcInfo.loc);
        break;
      }
    }
  } else {
    throw std::runtime_error("Invalid WaitForBreakpoint caller.");
  }
}
void Debug::m_stepIn(ASTNode *&node) {
  Block *blockPtr;
  if (auto fnCall = dynamic_cast<Call *>(node)) {
    auto callable =
        static_cast<Callable_T *>(fnCall->operand->Evaluate().get());
    if (auto native =
            dynamic_cast<NativeCallable_T *>(fnCall->operand->Evaluate().get());
        native != nullptr) {
      StepOver();
      return;
    }
    blockPtr = callable->block.get();
  } else if (auto ifStmnt = dynamic_cast<If *>(node)) {
    blockPtr = ifStmnt->block.get();
  } else if (auto elseStmnt = dynamic_cast<Else *>(node)) {
    blockPtr = elseStmnt->block.get();
  } else if (auto forStmnt = dynamic_cast<For *>(node)) {
    blockPtr = forStmnt->block.get();
  } else if (auto rangeFor = dynamic_cast<RangeBasedFor *>(node)) {
    blockPtr = rangeFor->block.get();
  } else if (auto objInit = dynamic_cast<ObjectInitializer *>(node)) {
    blockPtr = objInit->block.get();
  }
  
  if (!blockPtr) {
    StepOver();
    return;
  }
  
  if (blockPtr->statements.size() == 0) {
    StepOver();
    return;
  }

  auto &firstStmnt = blockPtr->statements[0];
  auto loc = firstStmnt->srcInfo.loc;
  InsertBreakpoint(loc);
  requestedStep = StepKind::None;
}
void Debug::WaitForBreakpoint(ASTNode *owner, ASTNode *node) {
  static const string breakpointKey = "breakpoint:";
  
  requestedStep = StepKind::None;
  
  bool match = false;
  int breakpoint;
  for (const auto &loc : breakpoints) {
    if (loc == node->srcInfo.loc) {
      match = true;
      breakpoint = loc;
    }
  }
  
  if (!match) {
    return;
  }
  
  std::cout << "breakpoint hit : line:" << breakpoint << "\n" << std::flush;
  std::cout << "node: " << typeid(*node).name() << "\n";
  
  while (requestedStep == StepKind::None && breakpoint == node->srcInfo.loc) {
    usleep(100'000);
    
    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(0, &set);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100'000;
    if (select(1, &set, NULL, NULL, &timeout) > 0) {
      std::string line;
      if (std::getline(std::cin, line)) {
        
        if (line == "over") {
          m_stepOver(owner, node);
          break;
        }
        
        if (line == "in") {
          m_stepIn(node);
          break;
        } 
        
        else if (line.length() > breakpointKey.length() &&
            line.substr(0, breakpointKey.length()) == breakpointKey) {
          m_setBreakpoint(line, breakpointKey);
          continue;
        }
        else if (line == "continue") {
          Continue();
          break;
        }
        else if (line == "inspect") {
          m_printScope();
        }
      }
    }
  }
  
  switch(requestedStep) {
  case StepKind::Over:
  m_stepOver(owner, node);
  case StepKind::In:
  m_stepIn(node);
  case StepKind::None:
  case StepKind::Out:
    break;
  }
}
void Debug::RemoveBreakpoint(int loc) {
  auto removed =
      std::remove(breakpoints.begin(), breakpoints.end(), loc);
  breakpoints.erase(removed, breakpoints.end());
}
void Debug::Continue() { requestedStep = StepKind::None; }

void Debug::InsertBreakpoint(int loc) {
  breakpoints.push_back(loc);
  requestedStep = StepKind::None;
}
