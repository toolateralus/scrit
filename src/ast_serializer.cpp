#include "ast_serializer.hpp"
#include "serializer.hpp"

ASTSerializer::ASTSerializer(int indent_level, int indent_size) {
	this->indent_level = indent_level;
	this->indent_size = indent_size;
}
void ASTSerializer::visit(ASTNode *_) {
  stream << string(indent_level, ' ') << "ASTNode\n";
}
void ASTSerializer::visit(Executable *_) {
  stream << string(indent_level, ' ') << "Executable\n";
}
void ASTSerializer::visit(Statement *_) {
  stream << string(indent_level, ' ') << "Statement\n";
}
void ASTSerializer::visit(Program *program) {
  stream << string(indent_level, ' ') << "Program: {\n";
  indent_level += indent_size;
  for (auto &stmt : program->statements) {
    stmt->Accept(this);
  }
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Expression *_) {
  stream << string(indent_level, ' ') << "Expression\n";
}
void ASTSerializer::visit(Operand *operand) {
  stream << string(indent_level, ' ') << "Operand: {\n";
  indent_level += indent_size;
  WriterSettings settings = {
    .StartingIndentLevel = indent_level,
    .IndentSize = indent_size
  };
  auto operandString = Writer::ToString(operand->value.get(), settings);
  stream << string(indent_level, ' ') << operandString << "\n";
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Identifier *identifier) {
  stream << string(indent_level, ' ') << "Identifier: " << identifier->name << "\n";
}
void ASTSerializer::visit(Arguments *arguments) {
  stream << string(indent_level, ' ') << "Arguments: {\n";
  indent_level += indent_size;
  for (auto &arg : arguments->values) {
    arg->Accept(this);
  }
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(TupleInitializer *tupleInitializer) {
  stream << string(indent_level, ' ') << "TupleInitializer: {\n";
  indent_level += indent_size;
  for (auto &arg : tupleInitializer->values) {
    arg->Accept(this);
  }
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Property *property) {
  stream << string(indent_level, ' ') << "Property: {\n";
  indent_level += indent_size;
  property->lambda->Accept(this);
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Parameters *parameters) {
  stream << string(indent_level, ' ') << "Parameters: {\n";
  indent_level += indent_size;
  for (auto &[name, value] : parameters->map) {
    stream << string(indent_level, ' ') << name << ": " << value << "\n";
  }
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Continue *_) {
  stream << string(indent_level, ' ') << "Continue\n";
}
void ASTSerializer::visit(Break *_) {
  stream << string(indent_level, ' ') << "Break\n";
}
void ASTSerializer::visit(Return *ret) {
  stream << string(indent_level, ' ') << "Return: {\n";
  indent_level += indent_size;
  ret->value->Accept(this);
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Delete *del) {
  stream << string(indent_level, ' ') << "Delete: {\n";
  indent_level += indent_size;
  if (del->iden != nullptr) {
    del->iden->Accept(this);
  }
  if (del->dot != nullptr) {
    del->dot->Accept(this);
  }
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Block *block) {
  stream << string(indent_level, ' ') << "Block: {\n";
  indent_level += indent_size;
  for (auto &stmt : block->statements) {
    stmt->Accept(this);
  }
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(ObjectInitializer *objInit) {
  stream << string(indent_level, ' ') << "ObjectInitializer: {\n";
  indent_level += indent_size;
  for (auto &stmt : objInit->block->statements) {
    stmt->Accept(this);
  }
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Call *call) {
  stream << string(indent_level, ' ') << "Call: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "Operand: {\n";
    {
      indent_level += indent_size;
      call->operand->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    call->args->Accept(this);
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(If *ifStmt) {
  stream << string(indent_level, ' ') << "If: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "Condition: {\n";
    {
      indent_level += indent_size;
      ifStmt->condition->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    ifStmt->block->Accept(this);
    if (ifStmt->elseStmnt != nullptr) {
      ifStmt->elseStmnt->Accept(this);
    }
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Else *elseStmt) {
  stream << string(indent_level, ' ') << "Else: {\n";
  indent_level += indent_size;
  if (elseStmt->ifStmnt != nullptr) {
    elseStmt->ifStmnt->Accept(this);
  }
  if (elseStmt->block != nullptr) {
    elseStmt->block->Accept(this);
  }
  indent_level -= indent_size;
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(For *forStmt) {
  stream << string(indent_level, ' ') << "For: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "Init: {\n";
    {
      indent_level += indent_size;
      forStmt->decl->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "Condition: {\n";
    {
      indent_level += indent_size;
      forStmt->condition->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "Update: {\n";
    {
      indent_level += indent_size;
      forStmt->increment->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    forStmt->block->Accept(this);
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(RangeBasedFor *rangeFor) {
  stream << string(indent_level, ' ') << "RangeBasedFor: {\n";
  {
    indent_level += indent_size;
    rangeFor->lhs->Accept(this);
    stream << string(indent_level, ' ') << "Range: {\n";
    {
      indent_level += indent_size;
      rangeFor->rhs->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    rangeFor->block->Accept(this);
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Assignment *assignment) {
  stream << string(indent_level, ' ') << "Assignment: {\n";
  {
    indent_level += indent_size;
    assignment->iden->Accept(this);
    stream << string(indent_level, ' ') << "Expression: {\n";
    {
      indent_level += indent_size;
      assignment->expr->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    string mut =
        assignment->mutability == Mutability::Mut ? "Mutable" : "Immutable";
    stream << string(indent_level, ' ') << "Mutability: " << mut << "\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(TupleDeconstruction *tupleDec) {
  stream << string(indent_level, ' ') << "TupleDeconstruction: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "Identifiers: {\n";
    {
      indent_level += indent_size;
      for (auto &iden : tupleDec->idens) {
        iden->Accept(this);
      }
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "TupleInitializer: {\n";
    {
      indent_level += indent_size;
      tupleDec->tuple->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(CompAssignExpr *compAssignExpr) {
  stream << string(indent_level, ' ') << "CompAssignExpr: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "Operand: {\n";
    {
      indent_level += indent_size;
      compAssignExpr->left->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream
           << string(indent_level, ' ')
           << "Operator: " << TTypeToString(compAssignExpr->op) << "\n";
    stream << string(indent_level, ' ') << "Expression: {\n";
    {
      indent_level += indent_size;
      compAssignExpr->right->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(CompoundAssignment *compoundAssign) {
  stream << string(indent_level, ' ') << "CompoundAssignment: {\n";
  {
    indent_level += indent_size;
    compoundAssign->expr->Accept(this);
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(FunctionDecl *funcDecl) {
  stream << string(indent_level, ' ') << "FunctionDecl: {\n";
  {
    indent_level += indent_size;
    stream
           << string(indent_level, ' ') << "Identifier: " << funcDecl->name << "\n";
    funcDecl->parameters->Accept(this);
    funcDecl->block->Accept(this);
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Noop *_) {
  stream << string(indent_level, ' ') << "Noop\n";
}
void ASTSerializer::visit(DotExpr *dotExpr) {
  stream << string(indent_level, ' ') << "DotExpr: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "Operand: {\n";
    {
      indent_level += indent_size;
      dotExpr->left->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "Identifier: {\n";
    {
      indent_level += indent_size;
      dotExpr->right->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(DotAssignment *dotAssign) {
  stream << string(indent_level, ' ') << "DotAssignment: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "DotExpr: {\n";
    {
      indent_level += indent_size;
      dotAssign->dot->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "Expression: {\n";
    {
      indent_level += indent_size;
      dotAssign->value->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(DotCallStmnt *dotCall) {
  stream << string(indent_level, ' ') << "DotCallStmnt: {\n";
  {
    indent_level += indent_size;
    dotCall->dot->Accept(this);
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Subscript *subscript) {
  stream << string(indent_level, ' ') << "Subscript: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "Operand: {\n";
    {
      indent_level += indent_size;
      subscript->left->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "Index: {\n";
    {
      indent_level += indent_size;
      subscript->index->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(SubscriptAssignStmnt *subscriptAssign) {
  stream << string(indent_level, ' ') << "SubscriptAssignStmnt: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "Subscript: {\n";
    {
      indent_level += indent_size;
      subscriptAssign->subscript->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "Expression: {\n";
    {
      indent_level += indent_size;
      subscriptAssign->value->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(UnaryExpr *unaryExpr) {
  stream << string(indent_level, ' ') << "UnaryExpr: {\n";
  {
    indent_level += indent_size;
    stream
           << string(indent_level, ' ')
           << "Operator: " << TTypeToString(unaryExpr->op) << "\n";
    stream << string(indent_level, ' ') << "Operand: {\n";
    {
      indent_level += indent_size;
      unaryExpr->operand->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(UnaryStatement *unaryStmt) {
  stream << string(indent_level, ' ') << "UnaryStatement: {\n";
  {
    indent_level += indent_size;
    unaryStmt->expr->Accept(this);
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(BinExpr *binExpr) {
  stream << string(indent_level, ' ') << "BinExpr: {\n";
  {
    indent_level += indent_size;
    stream
           << string(indent_level, ' ')
           << "Operator: " << TTypeToString(binExpr->op) << "\n";
    stream << string(indent_level, ' ') << "Left: {\n";
    {
      indent_level += indent_size;
      binExpr->left->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "Right: {\n";
    {
      indent_level += indent_size;
      binExpr->right->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Using *usingStmt) {
  stream << string(indent_level, ' ') << "Using: {\n";
  {
    indent_level += indent_size;
    stream
           << string(indent_level, ' ')
           << "IsWildcard: " << usingStmt->isWildcard << "\n";
    stream
           << string(indent_level, ' ')
           << "Identifier: " << usingStmt->moduleName << "\n";
    stream << string(indent_level, ' ') << "Symbols: {\n";
    {
      indent_level += indent_size;
      for (auto &symbol : usingStmt->symbols) {
        stream << string(indent_level, ' ') << symbol << "\n";
      }
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Lambda *lambda) {
  stream << string(indent_level, ' ') << "Lambda: {\n";
  {
    indent_level += indent_size;
    if (lambda->block != nullptr) {
      lambda->block->Accept(this);
    }
    if (lambda->expr != nullptr) {
      lambda->expr->Accept(this);
    }
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(Match *match) {
  stream << string(indent_level, ' ') << "Match: {\n";
  {
    indent_level += indent_size;
    stream << string(indent_level, ' ') << "Expression: {\n";
    {
      indent_level += indent_size;
      match->expr->Accept(this);
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "Cases: {\n";
    {
      indent_level += indent_size;
      for (size_t i = 0; i < match->branch_lhs.size(); i++) {
        auto &lhs = match->branch_lhs[i];
        auto &rhs = match->branch_rhs[i];
        stream << string(indent_level, ' ') << "Case: {\n";
        {
          indent_level += indent_size;
          stream << string(indent_level, ' ') << "Pattern: {\n";
          {
            indent_level += indent_size;
            lhs->Accept(this);
            indent_level -= indent_size;
          }
          stream << string(indent_level, ' ') << "}\n";
          stream << string(indent_level, ' ') << "Expression: {\n";
          {
            indent_level += indent_size;
            rhs->Accept(this);
            indent_level -= indent_size;
          }
          stream << string(indent_level, ' ') << "}\n";
          indent_level -= indent_size;
        }
        stream << string(indent_level, ' ') << "}\n";
      }
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    stream << string(indent_level, ' ') << "Default: {\n";
    {
      indent_level += indent_size;
      if (match->branch_default != nullptr) {
        match->branch_default->Accept(this);
      }
      indent_level -= indent_size;
    }
    stream << string(indent_level, ' ') << "}\n";
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
void ASTSerializer::visit(MatchStatement *matchStmt) {
  stream << string(indent_level, ' ') << "MatchStatement: {\n";
  {
    indent_level += indent_size;
    matchStmt->match->Accept(this);
    indent_level -= indent_size;
  }
  stream << string(indent_level, ' ') << "}\n";
}
