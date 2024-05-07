#include "ast.hpp"
#include "value.hpp"

Context ASTNode::context = {};
auto ExecutionResult::None = ExecutionResult(ControlChange::None, Value_T::Undefined);
auto ExecutionResult::Break = ExecutionResult(ControlChange::Break, Value_T::Undefined);
auto ExecutionResult::Continue = ExecutionResult(ControlChange::Continue, Value_T::Undefined);

ExecutionResult If::Execute() {
  auto condResult = condition->Evaluate();
  if (condResult->type != ValueType::Bool) {
    return ExecutionResult::None;
  }
  auto b = static_cast<Bool_T *>(condResult.get());
  
  if (b->Equals(Value_T::True)) {
    auto result = block->Execute();
    switch (result.controlChange) {
    case ControlChange::None:
      break;
    case ControlChange::Return:
    case ControlChange::Exception:
    case ControlChange::Continue:
    case ControlChange::Break:
      return result;
    case ControlChange::Goto:
    case ControlChange::ContinueLabel:
    case ControlChange::BreakLabel:
      // TODO: Check for label Here
      throw std::runtime_error(CC_ToString(result.controlChange) + " not implemented");
    }
  }
  if (elseStmnt) {
    auto result = elseStmnt->Execute();
    switch (result.controlChange) {
    case ControlChange::None:
      break;
    case ControlChange::Return:
    case ControlChange::Exception:
    case ControlChange::Continue:
    case ControlChange::Break:
      return result;
    case ControlChange::Goto:
    case ControlChange::ContinueLabel:
    case ControlChange::BreakLabel:
      // TODO: Check for label Here
      throw std::runtime_error(CC_ToString(result.controlChange) + " not implemented");
    }
  }
  return ExecutionResult::None;
}
If::If(unique_ptr<Expression> &&condition, unique_ptr<Block> &&block,
       unique_ptr<Else> &&elseStmnt)
    : condition(std::move(condition)), block(std::move(block)),
      elseStmnt(std::move(elseStmnt)) {}
If::If(unique_ptr<Expression> &&condition, unique_ptr<Block> &&block): condition(std::move(condition)), block(std::move(block)) {}
unique_ptr<If> If::NoElse(unique_ptr<Expression> &&condition,
                          unique_ptr<Block> &&block) {
  return make_unique<If>(std::move(condition), std::move(block));
}
unique_ptr<If> If::WithElse(unique_ptr<Expression> &&condition,
                            unique_ptr<Block> &&block,
                            unique_ptr<Else> &&elseStmnt) {
  return make_unique<If>(std::move(condition), std::move(block),
                         std::move(elseStmnt));
}
unique_ptr<Else> Else::New(unique_ptr<If> &&ifStmnt) {
  auto elseStmnt = make_unique<Else>();
  elseStmnt->ifStmnt = std::move(ifStmnt);
  return elseStmnt;
}
unique_ptr<Else> Else::NoIf(unique_ptr<Block> &&block) {
  auto elseStmnt = make_unique<Else>();
  ;
  elseStmnt->block = std::move(block);
  return elseStmnt;
}
