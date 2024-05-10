#include "debug.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "value.hpp"
#include <algorithm>
#include <sys/select.h>

std::vector<int> Debug::breakpoints = {};
auto Debug::stepRequested = false;


void Debug::WaitForBreakpoint(ASTNode *owner, ASTNode *node) {
  
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
  while (!stepRequested && breakpoint == node->srcInfo.loc) {
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
        if (line == "next") {
          stepRequested = true;
          break;
        }
        
        static const string breakpointKey = "breakpoint:";
        if (line.length() > breakpointKey.length() &&
            line.substr(0, breakpointKey.length()) == breakpointKey) {
          string num = "";
          for (int i = breakpointKey.length(); i < line.length(); ++i) {
            num += line[i];
          }
          int index = std::stoi(num);
          Debug::InsertBreakpoint(index);
          continue;
        }
        
        if (line == "continue") {
          return;
        }
        if (line == "inspect") {
          auto scope = ASTNode::context.scopes.back();
          if (scope->variables.size() == 0) {
            std::cout << "scope empty." << '\n';
          }
          for (const auto &[key, var] : scope->variables) {
            std::cout << key << " :" << var->ToString() << "\n";
          }
          std::cout << std::flush;
        }
      }
    }
  }
  if (stepRequested) {
    stepRequested = false;
    
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
    }
    
  }
}
void Debug::RemoveBreakpoint(int loc) {
  auto removed =
      std::remove(breakpoints.begin(), breakpoints.end(), loc);
  breakpoints.erase(removed, breakpoints.end());
}
void Debug::Continue() { stepRequested = false; }
void Debug::InsertBreakpoint(int loc) {
  breakpoints.push_back(loc);
  stepRequested = false;
}
