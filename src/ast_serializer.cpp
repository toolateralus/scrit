#include "ast_serializer.hpp"
#include "ast.hpp"
#include "value.hpp"

ASTSerializer::ASTSerializer(int indent_level, int indent_size) {
	this->indent_level = indent_level;
	this->indent_size = indent_size;
}
void ASTSerializer::visit(ASTNode *_) {
  Write("ASTNode");
}
void ASTSerializer::visit(Executable *_) {
  Write("Executable");
}
void ASTSerializer::visit(Statement *_) {
  Write("Statement");
}
void ASTSerializer::visit(Program *program) {
  Write("Program: {");
  auto _ = Indenter(this);
  for (auto &stmt : program->statements) {
    stmt->Accept(this);
  }
  
  Write("}");
}
void ASTSerializer::visit(Expression *_) {
  Write("Expression");
}
void ASTSerializer::visit(Operand *operand) {
  auto _ = Indenter(this);
  Write("Operand: {\n" + operand->Evaluate()->ToString() + "\n type" + operand->type->name  + "\n}");
  
}
void ASTSerializer::visit(Identifier *identifier) {
  Write("Identifier: " + identifier->name + "");
}
void ASTSerializer::visit(Arguments *arguments) {
  Write("Arguments: {");
  auto _ = Indenter(this);
  for (auto &arg : arguments->values) {
    arg->Accept(this);
  }
  
  Write("}");
}
void ASTSerializer::visit(TupleInitializer *tupleInitializer) {
  Write("TupleInitializer: {");
  auto _ = Indenter(this);
  for (auto &arg : tupleInitializer->values) {
    arg->Accept(this);
  }
  
  Write("}");
}
void ASTSerializer::visit(Property *property) {
  Write("Property: {");
  auto _ = Indenter(this);
  property->lambda->Accept(this);
  
  Write("}");
}
void ASTSerializer::visit(Parameters *parameters) {
  Write("Parameters: {");
  auto _ = Indenter(this);
  for (auto &param : parameters->values) {
    stream << Indent() << param.name << ": " << param.type->name << "";
  }
  
  Write("}");
}
void ASTSerializer::visit(Continue *_) {
  Write("Continue");
}
void ASTSerializer::visit(Break *_) {
  Write("Break");
}
void ASTSerializer::visit(Return *ret) {
  Write("Return: {");
  auto _ = Indenter(this);
  ret->value->Accept(this);
  
  Write("}");
}
void ASTSerializer::visit(Delete *del) {
  Write("Delete: {");
  auto _ = Indenter(this);
  if (del->iden != nullptr) {
    del->iden->Accept(this);
  }
  if (del->dot != nullptr) {
    del->dot->Accept(this);
  }
  
  Write("}");
}
void ASTSerializer::visit(Block *block) {
  Write("Block: {");
  auto _ = Indenter(this);
  for (auto &stmt : block->statements) {
    stmt->Accept(this);
  }
  
  Write("}");
}
void ASTSerializer::visit(ObjectInitializer *objInit) {
  Write("ObjectInitializer: {");
  auto _ = Indenter(this);
  for (auto &stmt : objInit->block->statements) {
    stmt->Accept(this);
  }
  
  Write("}");
}
void ASTSerializer::visit(Call *call) {
  Write("Call: {");
  {
    auto _ = Indenter(this);
    Write("Operand: {");
    {
      auto _ = Indenter(this);
      call->operand->Accept(this);
      
    }
    Write("}");
    call->args->Accept(this);
    
  }
  Write("}");
}
void ASTSerializer::visit(If *ifStmt) {
  Write("If: {");
  {
    auto _ = Indenter(this);
    Write("Condition: {");
    {
      auto _ = Indenter(this);
      ifStmt->condition->Accept(this);
      
    }
    Write("}");
    ifStmt->block->Accept(this);
    if (ifStmt->elseStmnt != nullptr) {
      ifStmt->elseStmnt->Accept(this);
    }
    
  }
  Write("}");
}
void ASTSerializer::visit(Else *elseStmt) {
  Write("Else: {");
  auto _ = Indenter(this);
  if (elseStmt->ifStmnt != nullptr) {
    elseStmt->ifStmnt->Accept(this);
  }
  if (elseStmt->block != nullptr) {
    elseStmt->block->Accept(this);
  }
  
  Write("}");
}
void ASTSerializer::visit(For *forStmt) {
  Write("For: {");
  {
    
    if (forStmt->decl) {
      auto _ = Indenter(this);
      Write("Init: {");
      {
        auto _ = Indenter(this);
        forStmt->decl->Accept(this);
      }
      Write("}");
    }
    if (forStmt->condition) {
      Write("Condition: {");
      {
        auto _ = Indenter(this);
        forStmt->condition->Accept(this);
        
      }
      Write("}");
    }
    if (forStmt->increment) {
      Write("Update: {");
      {
        auto _ = Indenter(this);
        forStmt->increment->Accept(this);
        
      }
      Write("}");
    }
    
    forStmt->block->Accept(this);
  }
  Write("}");
}
void ASTSerializer::visit(RangeBasedFor *rangeFor) {
  Write("RangeBasedFor: {");
  {
    auto _ = Indenter(this);
    
    if (rangeFor->lhs) {
      rangeFor->lhs->Accept(this);
    } else if (!rangeFor->names.empty()) {
      auto size = rangeFor->names.size();
      auto i =0;
      string output;
      for (const auto &name : rangeFor->names) {
        output += name;
        if (i != size - 1) {
          output += ", ";
        }
        i++;
      }
      Write("TupleDeconstruction: " + output);
      
    }
    
    Write("Range: {");
    {
      auto _ = Indenter(this);
      rangeFor->rhs->Accept(this);
      
    }
    Write("}");
    rangeFor->block->Accept(this);
    
  }
  Write("}");
}
void ASTSerializer::visit(Declaration *del) {
  Write("Declaration: {");
  {
    auto _ = Indenter(this);
    
    
    Write("Expression: {");
    {
      auto _ = Indenter(this);
      del->expr->Accept(this);
      
    }
    Write("}");
    Write(string("Mutability: ") + (del->mut == Mutability::Mut ? "mut" : "const") + "");
    
  }
  Write("}");
}
void ASTSerializer::visit(Assignment *assignment) {
  // TODO: implement
}
void ASTSerializer::visit(TupleDeconstruction *tupleDec) {
  Write("TupleDeconstruction: {");
  {
    auto _ = Indenter(this);
    Write("Identifiers: {");
    {
      auto _ = Indenter(this);
      for (auto &iden : tupleDec->idens) {
        Write(iden + "");
      }
      
    }
    Write("}");
    Write("TupleInitializer: {");
    {
      auto _ = Indenter(this);
      tupleDec->tuple->Accept(this);
      
    }
    Write("}");
    
  }
  Write("}");
}
void ASTSerializer::visit(CompAssignExpr *compAssignExpr) {
  Write("CompAssignExpr: {");
  {
    auto _ = Indenter(this);
    Write("Operand: {");
    {
      auto _ = Indenter(this);
      compAssignExpr->left->Accept(this);
    }
    Write("}");
    Write("Operator: " + TTypeToString(compAssignExpr->op) + "");
    Write("Expression: {");
    {
      auto _ = Indenter(this);
      compAssignExpr->right->Accept(this);
      
    }
    Write("}");
    
  }
  Write("}");
}
void ASTSerializer::visit(CompoundAssignment *compoundAssign) {
  Write("CompoundAssignment: {");
  {
    auto _ = Indenter(this);
    compoundAssign->expr->Accept(this);
    
  }
  Write("}");
}
void ASTSerializer::visit(FunctionDecl *funcDecl) {
  Write("FunctionDecl: {");
  {
    auto _ = Indenter(this);
    Write("Identifier: " + funcDecl->name);
    funcDecl->parameters->Accept(this);
    funcDecl->block->Accept(this);
    
  }
  Write("}");
}
void ASTSerializer::visit(Noop *_) {
  Write("Noop");
}
void ASTSerializer::visit(DotExpr *dotExpr) {
  Write("DotExpr: {");
  {
    auto _ = Indenter(this);
    Write("Operand: {");
    {
      auto _ = Indenter(this);
      dotExpr->left->Accept(this);
      
    }
    Write("}");
    Write("Identifier: {");
    {
      auto _ = Indenter(this);
      dotExpr->right->Accept(this);
      
    }
    Write("}");
    
  }
  Write("}");
}
void ASTSerializer::visit(DotAssignment *dotAssign) {
  Write("DotAssignment: {");
  {
    auto _ = Indenter(this);
    Write("DotExpr: {");
    {
      auto _ = Indenter(this);
      dotAssign->dot->Accept(this);
    }
    Write("}");
    Write("Expression: {");
    {
      auto _ = Indenter(this);
      dotAssign->value->Accept(this);
    }
    Write("}");
  }
  Write("}");
}
void ASTSerializer::visit(DotCallStmnt *dotCall) {
  Write("DotCallStmnt: {");
  {
    auto _ = Indenter(this);
    dotCall->dot->Accept(this);
  }
  Write("}");
}
void ASTSerializer::visit(Subscript *subscript) {
  Write("Subscript: {");
  {
    auto _ = Indenter(this);
    Write("Operand: {");
    {
      auto _ = Indenter(this);
      subscript->left->Accept(this);
    }
    Write("}");
    Write("Index: {");
    {
      auto _ = Indenter(this);
      subscript->index->Accept(this);
    }
    Write("}");
  }
  Write("}");
}
void ASTSerializer::visit(SubscriptAssignStmnt *subscriptAssign) {
  Write("SubscriptAssignStmnt: {");
  {
    auto _ = Indenter(this);
    Write("Subscript: {");
    {
      auto _ = Indenter(this);
      subscriptAssign->subscript->Accept(this);
    }
    Write("}");
    Write("Expression: {");
    {
      auto _ = Indenter(this);
      subscriptAssign->value->Accept(this);
    }
    Write("}");
  }
  Write("}");
}
void ASTSerializer::visit(UnaryExpr *unaryExpr) {
  Write("UnaryExpr: {");
  {
    auto _ = Indenter(this);
    Write("Operator: " + TTypeToString(unaryExpr->op));
    Write("Operand: {");
    {
      auto _ = Indenter(this);
      unaryExpr->operand->Accept(this);
    }
    Write("}");
  }
  Write("}");
}
void ASTSerializer::visit(UnaryStatement *unaryStmt) {
  Write("UnaryStatement: {");
  {
    auto _ = Indenter(this);
    unaryStmt->expr->Accept(this);
  }
  Write("}");
}
void ASTSerializer::visit(BinExpr *binExpr) {
  Write("BinExpr: {");
  {
    auto _ = Indenter(this);
    Write("Operator: " + TTypeToString(binExpr->op));
    Write("Left: {");
    {
      auto _ = Indenter(this);
      binExpr->left->Accept(this);
    }
    Write("}");
    Write("Right: {");
    {
      auto _ = Indenter(this);
      binExpr->right->Accept(this);
    }
    Write("}");
  }
  Write("}");
}
void ASTSerializer::visit(Using *usingStmt) {
  Write("Using: {");
  {
    auto _ = Indenter(this);
    Write("IsWildcard: " + std::to_string(usingStmt->isWildcard));
    Write("Identifier: " + usingStmt->moduleName);
    if (!usingStmt->symbols.empty()) {
      Write("Symbols: {");
      {
        auto _ = Indenter(this);
        for (auto &symbol : usingStmt->symbols) {
          Write(symbol);
        }
      }
      Write("}");
    }
  }
  Write("}");
}
void ASTSerializer::visit(Lambda *lambda) {
  Write("Lambda: {");
  {
    auto _ = Indenter(this);
    if (lambda->block != nullptr) {
      lambda->block->Accept(this);
    }
    if (lambda->expr != nullptr) {
      lambda->expr->Accept(this);
    }
  }
  Write("}");
}
void ASTSerializer::visit(Match *match) {
  Write("Match: {");
  {
    auto _ = Indenter(this);
    Write("Expression: {");
    {
      auto _ = Indenter(this);
      match->expr->Accept(this);
    }
    Write("}");
    Write("Cases: {");
    {
      auto _ = Indenter(this);
      for (size_t i = 0; i < match->patterns.size(); i++) {
        auto &lhs = match->patterns[i];
        auto &rhs = match->expressions[i];
        Write("Case: {");
        {
          auto _ = Indenter(this);
          Write("Pattern: {");
          {
            auto _ = Indenter(this);
            lhs->Accept(this);
            
          }
          Write("}");
          Write("Expression: {");
          {
            auto _ = Indenter(this);
            rhs->Accept(this);
            
          }
          Write("}");
          
        }
        Write("}");
      }
      
    }
    Write("}");
    if (match->branch_default != nullptr) {
      Write("Default: {");
      auto _ = Indenter(this);
      match->branch_default->Accept(this);
      Write("}");
    }
    
  }
  Write("}");
}
void ASTSerializer::visit(MatchStatement *matchStmt) {
  Write("MatchStatement: {");
  auto _ = Indenter(this);
  matchStmt->match->Accept(this);
  Write("}");
}
void ASTSerializer::visit(Literal *literal) {
  if (literal->type->name == "string") {
    Write("\"" + literal->Evaluate()->ToString() + "\"");
  } else {
    Write(literal->Evaluate()->ToString());
  }
}



// The auto indenter.
Indenter::Indenter(ASTSerializer *serializer) : serializer(serializer) {
  serializer->indent_level += serializer->indent_size;
}
Indenter::~Indenter() { serializer->indent_level -= serializer->indent_size; }
