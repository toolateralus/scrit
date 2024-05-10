#include "debug.hpp"
#include "ast.hpp"
#include <sys/select.h>
#include "context.hpp"
#include "value.hpp"

auto Debug::currentBreakpoint = -1;
auto Debug::stepRequested = false;

void Debug::WaitForBreakpoint(ASTNode *node) {
  if (currentBreakpoint == -1 || node->loc != currentBreakpoint) {
    return;
  }
  std::cout << "breakpoint hit : line:" << currentBreakpoint << "\n" << std::flush;
  while (!stepRequested && currentBreakpoint == node->loc) {
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
        if (line == "continue") {
          currentBreakpoint = -1;
          return;
        }
        if (line == "inspect") {
          auto scope = ASTNode::context.scopes.back();
          if (scope->variables.size() == 0) {
            std::cout << "scope empty." << '\n';
          }
          for (const auto &[key,var] : scope->variables) {
            std::cout << key << " :" << var->ToString() << "\n";
          }
          std::cout << std::flush;
        }
      }
    }
  }
  if (stepRequested) {
    stepRequested = false;
    currentBreakpoint++;
  }
}

  void Debug::Continue() {
    currentBreakpoint = -1;
    stepRequested = false;
  }
  void Debug::InsertBreakpoint(int loc) {
    currentBreakpoint = loc;
    stepRequested = false;
  }
