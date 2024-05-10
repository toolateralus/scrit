#include "debug.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "value.hpp"
#include <algorithm>
#include <gtest/gtest_prod.h>
#include <stdexcept>
#include <sys/select.h>

std::vector<Breakpoint> Debug::breakpoints = {};
auto Debug::requestedStep = StepKind::None;
ASTNode *Debug::lastNode = nullptr;
int Debug::stepOutIndex = -1;

void Debug::WaitForBreakpoint(ASTNode *owner, ASTNode *node,
                              const int &statementIndex) {
  static const string breakpointKey = "breakpoint:";
  
  if (lastNode == nullptr)
    m_getInfo(owner, node);
  
  requestedStep = StepKind::None;
  
  bool match = false;
  Breakpoint breakpoint;
  for (const auto &br : breakpoints) {
    if (br.loc == node->srcInfo.loc) {
      match = true;
      breakpoint = br;
    }
  }
  
  if (!match) {
    return;
  }
  
  std::cout << "breakpoint hit : line:" << breakpoint.loc << "\n" << std::flush;
  std::cout << "node: " << typeid(*node).name() << "\n";
  
  while (requestedStep == StepKind::None && breakpoint.loc == node->srcInfo.loc) {
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
          StepOver();
          break;
        } else if (line == "in") {
          StepIn();
          break;
        } else if (line == "out") {
          StepOut();
          break;
        } else if (line.length() > breakpointKey.length() &&
                   line.substr(0, breakpointKey.length()) == breakpointKey) {
          m_setBreakpoint(line, breakpointKey);
          continue;
        } else if (line == "continue") {
          Continue();
          break;
        } else if (line == "inspect") {
          m_printScope();
        }
      }
    }
  }
  
  switch (requestedStep) {
  case StepKind::Over:
    m_stepOver(owner, node);
    break;
  case StepKind::In:
    m_stepIn(owner, node, statementIndex);
    break;
  case StepKind::Out:
    m_stepOut();
    break;
  case StepKind::None:
    lastNode = nullptr;
    break;
  }
}
void Debug::RemoveBreakpoint(const int &loc, const bool isTemporary) {
  breakpoints.erase(std::remove_if(breakpoints.begin(), breakpoints.end(),
                                   [loc, isTemporary](Breakpoint breakpoint) {
                                     return breakpoint.loc == loc && breakpoint.isTemporary;
                                   }),
                    breakpoints.end());
}
void Debug::InsertBreakpoint(const int &loc, const bool isTemporary = false) {
  breakpoints.push_back({loc, isTemporary});
}

void Debug::m_setBreakpoint(std::string &line, const string &breakpointKey) {
  string num = "";
  for (int i = breakpointKey.length(); i < line.length(); ++i) {
    num += line[i];
  }
  int index = std::stoi(num);
  Debug::InsertBreakpoint(index, false);
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
void Debug::m_stepOut() {
  if (!lastNode || stepOutIndex == -1) {
    return;
  }
  auto *node = lastNode;
  vector<StatementPtr> *statements = nullptr;

  // search for the statements of the block from the node that was last stepped
  // into
  {
    if (auto fnCall = dynamic_cast<Call *>(node)) {
      auto callable =
          static_cast<Callable_T *>(fnCall->operand->Evaluate().get());
      if (auto native = dynamic_cast<NativeCallable_T *>(
              fnCall->operand->Evaluate().get());
          native != nullptr) {
        StepOver();
        return;
      }
      statements = &callable->block.get()->statements;
    } else if (auto ifStmnt = dynamic_cast<If *>(node)) {
      statements = &ifStmnt->block.get()->statements;
    } else if (auto elseStmnt = dynamic_cast<Else *>(node)) {
      statements = &elseStmnt->block.get()->statements;
    } else if (auto forStmnt = dynamic_cast<For *>(node)) {
      statements = &forStmnt->block.get()->statements;
    } else if (auto rangeFor = dynamic_cast<RangeBasedFor *>(node)) {
      statements = &rangeFor->block.get()->statements;
    } else if (auto objInit = dynamic_cast<ObjectInitializer *>(node)) {
      statements = &objInit->block.get()->statements;
    } else if (auto block = dynamic_cast<Block *>(node)) {
      statements = &block->statements;
    } else if (auto program = dynamic_cast<Program *>(node)) {
      statements = &program->statements;
    }
  }

  if (statements != nullptr && stepOutIndex < statements->size()) {
    auto &statement = (*statements)[stepOutIndex];
    InsertBreakpoint(statement->srcInfo.loc, true);
  }
}
void Debug::m_getInfo(ASTNode *&owner, ASTNode *&node) {
  lastNode = owner;
  vector<StatementPtr> *statements;
  if (auto fnCall = dynamic_cast<Call *>(owner)) {
    auto callable =
        static_cast<Callable_T *>(fnCall->operand->Evaluate().get());
    if (auto native =
            dynamic_cast<NativeCallable_T *>(fnCall->operand->Evaluate().get());
        native != nullptr) {
    } else {
      statements = &callable->block.get()->statements;
    }
  } else if (auto ifStmnt = dynamic_cast<If *>(owner)) {
    statements = &ifStmnt->block.get()->statements;
  } else if (auto elseStmnt = dynamic_cast<Else *>(owner)) {
    statements = &elseStmnt->block.get()->statements;
  } else if (auto forStmnt = dynamic_cast<For *>(owner)) {
    statements = &forStmnt->block.get()->statements;
  } else if (auto rangeFor = dynamic_cast<RangeBasedFor *>(owner)) {
    statements = &rangeFor->block.get()->statements;
  } else if (auto objInit = dynamic_cast<ObjectInitializer *>(owner)) {
    statements = &objInit->block.get()->statements;
  } else if (auto block = dynamic_cast<Block *>(owner)) {
    statements = &block->statements;
  } else if (auto program = dynamic_cast<Program *>(owner)) {
    statements = &program->statements;
  }

  if (statements) {
    int i = 0;
    for (const auto &statement : *statements) {
      if (node == statement.get()) {
        stepOutIndex = i + 1;
        break;
      } else {
        i++;
      }
    }
  } else {
    lastNode = nullptr;
  }
}
void Debug::m_stepIn(ASTNode *&owner, ASTNode *&node, const int &index) {
  Block *blockPtr;
  
  m_getInfo(owner, node);
  
  int loc = -1;
  // actually search which type of node that contains a block is being stepped
  // into and set a breakpoint on the first statement of that block;
  {
    if (auto fnCall = dynamic_cast<Call *>(node)) {
      auto callable =
          static_cast<Callable_T *>(fnCall->operand->Evaluate().get());
      if (auto native = dynamic_cast<NativeCallable_T *>(
              fnCall->operand->Evaluate().get());
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
    loc = firstStmnt->srcInfo.loc;
  }

  if (loc != -1) {
    InsertBreakpoint(loc, true);
  }
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
        InsertBreakpoint(statement->srcInfo.loc, true);
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
        InsertBreakpoint(statement->srcInfo.loc, true);
        break;
      }
    }
  } else {
    throw std::runtime_error("Invalid WaitForBreakpoint caller.");
  }
}
