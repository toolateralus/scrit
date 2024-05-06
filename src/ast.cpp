#include "ast.hpp"
#include <iostream>

Context ASTNode::context = {};

unique_ptr<ASTNode> If::EvaluateStatement() {
  auto condResult = condition->Evaluate();
  if (auto b = dynamic_cast<Bool_T *>(condResult.get())) {
    if (b->Equals(Value_T::True)) {
      auto result = block->EvaluateStatement();
      if (dynamic_cast<Return *>(result.get()) ||
          dynamic_cast<Continue *>(result.get()) ||
          dynamic_cast<Break *>(result.get())) {
        return result;
      }
    } else {
    std::cout << "evalutaing else" << "\n";
    return elseStmnt->EvaluateStatement();
    }
  } 
  return nullptr;
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
