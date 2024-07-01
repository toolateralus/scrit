#include "ast_serializer.hpp"
#include "ast.hpp"

ASTSerializer::ASTSerializer(int indent_level, int indent_size) {
	this->indent_level = indent_level;
	this->indent_size = indent_size;
}
void ASTSerializer::visit(ASTNode *_) {
  Write("ASTNode\n");
}
void ASTSerializer::visit(Executable *_) {
  Write("Executable\n");
}
void ASTSerializer::visit(Statement *_) {
  Write("Statement\n");
}
void ASTSerializer::visit(Program *program) {
  Write("Program: {\n");
  indent_level += indent_size;
  for (auto &stmt : program->statements) {
    stmt->Accept(this);
  }
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(Expression *_) {
  Write("Expression\n");
}
void ASTSerializer::visit(Operand *operand) {
  // TODO: fix this.
  // Write("Operand: {\n");
  // indent_level += indent_size;
  // WriterSettings settings = {
  //   .StartingIndentLevel = indent_level,
  //   .IndentSize = indent_size
  // };
  // auto operandString = Writer::ToString(operand->value.get(), settings);
  // stream << Indent() << operandString << "\n";
  // indent_level -= indent_size;
  // Write("}\n");
}
void ASTSerializer::visit(Identifier *identifier) {
  Write("Identifier: " + identifier->name + "\n");
}
void ASTSerializer::visit(Arguments *arguments) {
  Write("Arguments: {\n");
  indent_level += indent_size;
  for (auto &arg : arguments->values) {
    arg->Accept(this);
  }
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(TupleInitializer *tupleInitializer) {
  Write("TupleInitializer: {\n");
  indent_level += indent_size;
  for (auto &arg : tupleInitializer->values) {
    arg->Accept(this);
  }
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(Property *property) {
  Write("Property: {\n");
  indent_level += indent_size;
  property->lambda->Accept(this);
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(Parameters *parameters) {
  Write("Parameters: {\n");
  indent_level += indent_size;
  for (auto &param : parameters->values) {
    stream << Indent() << param.name << ": " << param.type->name << "\n";
  }
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(Continue *_) {
  Write("Continue\n");
}
void ASTSerializer::visit(Break *_) {
  Write("Break\n");
}
void ASTSerializer::visit(Return *ret) {
  Write("Return: {\n");
  indent_level += indent_size;
  ret->value->Accept(this);
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(Delete *del) {
  Write("Delete: {\n");
  indent_level += indent_size;
  if (del->iden != nullptr) {
    del->iden->Accept(this);
  }
  if (del->dot != nullptr) {
    del->dot->Accept(this);
  }
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(Block *block) {
  Write("Block: {\n");
  indent_level += indent_size;
  for (auto &stmt : block->statements) {
    stmt->Accept(this);
  }
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(ObjectInitializer *objInit) {
  Write("ObjectInitializer: {\n");
  indent_level += indent_size;
  for (auto &stmt : objInit->block->statements) {
    stmt->Accept(this);
  }
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(Call *call) {
  Write("Call: {\n");
  {
    indent_level += indent_size;
    Write("Operand: {\n");
    {
      indent_level += indent_size;
      call->operand->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    call->args->Accept(this);
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(If *ifStmt) {
  Write("If: {\n");
  {
    indent_level += indent_size;
    Write("Condition: {\n");
    {
      indent_level += indent_size;
      ifStmt->condition->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    ifStmt->block->Accept(this);
    if (ifStmt->elseStmnt != nullptr) {
      ifStmt->elseStmnt->Accept(this);
    }
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(Else *elseStmt) {
  Write("Else: {\n");
  indent_level += indent_size;
  if (elseStmt->ifStmnt != nullptr) {
    elseStmt->ifStmnt->Accept(this);
  }
  if (elseStmt->block != nullptr) {
    elseStmt->block->Accept(this);
  }
  indent_level -= indent_size;
  Write("}\n");
}
void ASTSerializer::visit(For *forStmt) {
  Write("For: {\n");
  {
    indent_level += indent_size;
    Write("Init: {\n");
    {
      indent_level += indent_size;
      forStmt->decl->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("Condition: {\n");
    {
      indent_level += indent_size;
      forStmt->condition->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("Update: {\n");
    {
      indent_level += indent_size;
      forStmt->increment->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    forStmt->block->Accept(this);
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(RangeBasedFor *rangeFor) {
  Write("RangeBasedFor: {\n");
  {
    indent_level += indent_size;
    rangeFor->lhs->Accept(this);
    Write("Range: {\n");
    {
      indent_level += indent_size;
      rangeFor->rhs->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    rangeFor->block->Accept(this);
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(Declaration *del) {
  Write("Assignment: {\n");
  {
    indent_level += indent_size;
    
    
    Write("Expression: {\n");
    {
      indent_level += indent_size;
      del->expr->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    Write(string("Mutability: ") + (del->mut == Mutability::Mut ? "mut" : "const") + "\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(Assignment *assignment) {
  // TODO: implement
}
void ASTSerializer::visit(TupleDeconstruction *tupleDec) {
  Write("TupleDeconstruction: {\n");
  {
    indent_level += indent_size;
    Write("Identifiers: {\n");
    {
      indent_level += indent_size;
      for (auto &iden : tupleDec->idens) {
        iden->Accept(this);
      }
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("TupleInitializer: {\n");
    {
      indent_level += indent_size;
      tupleDec->tuple->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(CompAssignExpr *compAssignExpr) {
  Write("CompAssignExpr: {\n");
  {
    indent_level += indent_size;
    Write("Operand: {\n");
    {
      indent_level += indent_size;
      compAssignExpr->left->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    stream
           << Indent()
           << "Operator: " << TTypeToString(compAssignExpr->op) << "\n";
    Write("Expression: {\n");
    {
      indent_level += indent_size;
      compAssignExpr->right->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(CompoundAssignment *compoundAssign) {
  Write("CompoundAssignment: {\n");
  {
    indent_level += indent_size;
    compoundAssign->expr->Accept(this);
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(FunctionDecl *funcDecl) {
  Write("FunctionDecl: {\n");
  {
    indent_level += indent_size;
    stream
           << Indent() << "Identifier: " << funcDecl->name << "\n";
    funcDecl->parameters->Accept(this);
    funcDecl->block->Accept(this);
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(Noop *_) {
  Write("Noop\n");
}
void ASTSerializer::visit(DotExpr *dotExpr) {
  Write("DotExpr: {\n");
  {
    indent_level += indent_size;
    Write("Operand: {\n");
    {
      indent_level += indent_size;
      dotExpr->left->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("Identifier: {\n");
    {
      indent_level += indent_size;
      dotExpr->right->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(DotAssignment *dotAssign) {
  Write("DotAssignment: {\n");
  {
    indent_level += indent_size;
    Write("DotExpr: {\n");
    {
      indent_level += indent_size;
      dotAssign->dot->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("Expression: {\n");
    {
      indent_level += indent_size;
      dotAssign->value->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(DotCallStmnt *dotCall) {
  Write("DotCallStmnt: {\n");
  {
    indent_level += indent_size;
    dotCall->dot->Accept(this);
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(Subscript *subscript) {
  Write("Subscript: {\n");
  {
    indent_level += indent_size;
    Write("Operand: {\n");
    {
      indent_level += indent_size;
      subscript->left->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("Index: {\n");
    {
      indent_level += indent_size;
      subscript->index->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(SubscriptAssignStmnt *subscriptAssign) {
  Write("SubscriptAssignStmnt: {\n");
  {
    indent_level += indent_size;
    Write("Subscript: {\n");
    {
      indent_level += indent_size;
      subscriptAssign->subscript->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("Expression: {\n");
    {
      indent_level += indent_size;
      subscriptAssign->value->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(UnaryExpr *unaryExpr) {
  Write("UnaryExpr: {\n");
  {
    indent_level += indent_size;
    stream
           << Indent()
           << "Operator: " << TTypeToString(unaryExpr->op) << "\n";
    Write("Operand: {\n");
    {
      indent_level += indent_size;
      unaryExpr->operand->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(UnaryStatement *unaryStmt) {
  Write("UnaryStatement: {\n");
  {
    indent_level += indent_size;
    unaryStmt->expr->Accept(this);
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(BinExpr *binExpr) {
  Write("BinExpr: {\n");
  {
    indent_level += indent_size;
    stream
           << Indent()
           << "Operator: " << TTypeToString(binExpr->op) << "\n";
    Write("Left: {\n");
    {
      indent_level += indent_size;
      binExpr->left->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("Right: {\n");
    {
      indent_level += indent_size;
      binExpr->right->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(Using *usingStmt) {
  Write("Using: {\n");
  {
    indent_level += indent_size;
    stream
           << Indent()
           << "IsWildcard: " << usingStmt->isWildcard << "\n";
    stream
           << Indent()
           << "Identifier: " << usingStmt->moduleName << "\n";
    Write("Symbols: {\n");
    {
      indent_level += indent_size;
      for (auto &symbol : usingStmt->symbols) {
        stream << Indent() << symbol << "\n";
      }
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(Lambda *lambda) {
  Write("Lambda: {\n");
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
  Write("}\n");
}
void ASTSerializer::visit(Match *match) {
  Write("Match: {\n");
  {
    indent_level += indent_size;
    Write("Expression: {\n");
    {
      indent_level += indent_size;
      match->expr->Accept(this);
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("Cases: {\n");
    {
      indent_level += indent_size;
      for (size_t i = 0; i < match->branch_lhs.size(); i++) {
        auto &lhs = match->branch_lhs[i];
        auto &rhs = match->branch_rhs[i];
        Write("Case: {\n");
        {
          indent_level += indent_size;
          Write("Pattern: {\n");
          {
            indent_level += indent_size;
            lhs->Accept(this);
            indent_level -= indent_size;
          }
          Write("}\n");
          Write("Expression: {\n");
          {
            indent_level += indent_size;
            rhs->Accept(this);
            indent_level -= indent_size;
          }
          Write("}\n");
          indent_level -= indent_size;
        }
        Write("}\n");
      }
      indent_level -= indent_size;
    }
    Write("}\n");
    Write("Default: {\n");
    {
      indent_level += indent_size;
      if (match->branch_default != nullptr) {
        match->branch_default->Accept(this);
      }
      indent_level -= indent_size;
    }
    Write("}\n");
    indent_level -= indent_size;
  }
  Write("}\n");
}
void ASTSerializer::visit(MatchStatement *matchStmt) {
  Write("MatchStatement: {\n");
  {
    indent_level += indent_size;
    matchStmt->match->Accept(this);
    indent_level -= indent_size;
  }
  Write("}\n");
}
