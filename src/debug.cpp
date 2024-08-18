#include "debug.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "ctx.hpp"
#include "value.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

Value eval(std::vector<Value>);

std::vector<std::string> splitString(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;

  while (std::getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }

  return tokens;
}

std::string joinStrings(const std::ranges::view auto &view, char delimiter) {
  std::ostringstream oss;
  for (auto it = view.begin(); it != view.end(); ++it) {
    if (it != view.begin()) {
      oss << delimiter;
    }
    oss << *it;
  }
  return oss.str();
}

std::vector<Breakpoint> Debug::breakpoints = {};
StepKind Debug::requestedStep = StepKind::None;
ASTNode *Debug::lastNode = nullptr;
int Debug::stepOutIndex = -1;
std::string Debug::breakpointKey = "br:";

DebugControlFlow Debug::HandleInput(std::string &line) {
  if (line.empty()) {
    return DebugControlFlow::None;
  }
  auto tokens = splitString(line, ' ');
  auto args = tokens | std::ranges::views::drop(1);
  auto command = tokens[0];
  if (line == "over" || line == "ov") {
    StepOver();
    return DebugControlFlow::Break;
  } else if (line == "in" || line == "i") {
    StepIn();
    return DebugControlFlow::Break;
  } else if (line == "out" || line == "o") {
    StepOut();
    return DebugControlFlow::Break;
  } else if (line.length() > Debug::breakpointKey.length() &&
             line.substr(0, Debug::breakpointKey.length()) ==
                 Debug::breakpointKey) {
    m_setBreakpoint(line, Debug::breakpointKey);
    return DebugControlFlow::Continue;
  } else if (line == "continue" || line == "c") {
    Continue();
    return DebugControlFlow::Break;
  } else if (line == "inspect" || line == "s") {
    m_printScope();
  } else if (command == "print" || command == "p" && args.size() > 0) {
    auto input = "println(" + joinStrings(args, ' ') + ")";
    eval({Ctx::CreateString(input)});
  } else if (command == "show") {
    m_printBreakpoints();
  } else if (command == "rm" && args.size() == 1) {
    int index = std::stoi(args[0]);
    m_removeBreakpoint(index);
  } else if (command == "help") {
    std::cout
      << "\033[1;32mrm <index>\033[0m - \033[1;30mRemove a breakpoint at a specific index\033[0m" << "\n"
      << "\033[1;32mshow\033[0m - \033[1;30mShow all breakpoints\033[0m" << "\n"
      << "\033[1;32mbr:<line>\033[0m - \033[1;30mSet a breakpoint at a specific line number\033[0m" << "\n"
      << "\033[1;32mover | ov\033[0m - \033[1;30mStep over to the next line of code\033[0m" << "\n"
      << "\033[1;32min | i\033[0m - \033[1;30mStep into a function or block\033[0m" << "\n"
      << "\033[1;32mout | o\033[0m - \033[1;30mStep out of the current function or block\033[0m" << "\n"
      << "\033[1;32mcontinue | c\033[0m - \033[1;30mContinue execution until the next breakpoint\033[0m" << "\n"
      << "\033[1;32minspect | s\033[0m - \033[1;30mPrint the current scope variables\033[0m" << "\n"
      << "\033[1;32mprint | p <expr>\033[0m - \033[1;30mPrint the value of an expression\033[0m" << "\n";
  } else {
    std::cout << "\033[1;31mCommand not found.\033[0m" << std::endl;
  }
  return DebugControlFlow::None;
}

void Debug::m_hangUpOnBreakpoint(ASTNode *owner, ASTNode *node) {
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
  std::cout << "Type \"help\" and press enter to see a list of commands."<< "\n";

  while (requestedStep == StepKind::None &&
         breakpoint.loc == node->srcInfo.loc) {
    std::string line;
    if (std::getline(std::cin, line)) {
      switch (HandleInput(line)) {
      case DebugControlFlow::None:
        goto loopnone;
      case DebugControlFlow::Break:
        goto loopbreak;
      case DebugControlFlow::Continue:
        goto loopcontinue;
      }
    }
  loopbreak:
    break;
  loopcontinue:
  loopnone:
    continue;
  }

  switch (requestedStep) {
  case StepKind::Over:
    m_stepOver(owner, node);
    break;
  case StepKind::In:
    m_stepIn(owner, node);
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
                                     return breakpoint.loc == loc &&
                                            breakpoint.isTemporary ==
                                                isTemporary;
                                   }),
                    breakpoints.end());
}
void Debug::InsertBreakpoint(const int &loc, const bool isTemporary = false) {
  breakpoints.push_back({loc, isTemporary});
}

void Debug::m_removeBreakpoint(int index) {
  if (index >= 0 && index < breakpoints.size()) {
    breakpoints.erase(breakpoints.begin() + index);
  }
}

void Debug::m_printBreakpoints() {
  std::cout << "Breakpoints:" << std::endl;
  for (int i = 0; const auto& breakpoint : breakpoints) {
    std::cout << i << ": loc: " << breakpoint.loc << std::endl;
    i++;
  }
}

void Debug::m_setBreakpoint(std::string &line, const string &breakpointKey) {
  string num = "";
  for (size_t i = breakpointKey.length(); i < line.length(); ++i) {
    num += line[i];
  }
  int index = std::stoi(num);
  Debug::InsertBreakpoint(index, false);
}
void Debug::m_printScope() {
  auto &scope = ASTNode::context.CurrentScope();
  if (scope->Members().size() == 0) {
    std::cout << "scope empty." << '\n';
  }
  for (const auto &[key, var] : scope->Members()) {
    std::cout << key.value << " :" << var->ToString() << "\n";
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

  if (statements != nullptr && (size_t)stepOutIndex < statements->size()) {
    auto &statement = (*statements)[stepOutIndex];
    InsertBreakpoint(statement->srcInfo.loc, true);
  }
}
void Debug::m_getInfo(ASTNode *&owner, ASTNode *&node) {
  lastNode = owner;
  vector<StatementPtr> *statements = nullptr;
  if (auto fnCall = dynamic_cast<Call *>(owner)) {
    auto callable =
        static_cast<Callable_T *>(fnCall->operand->Evaluate().get());
    if (auto native =
            dynamic_cast<NativeCallable_T *>(fnCall->operand->Evaluate().get());
        native != nullptr) {
      return;
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
void Debug::m_stepIn(ASTNode *&owner, ASTNode *&node) {
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
    } else {
      throw std::runtime_error("invalid step in");
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
