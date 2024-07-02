#pragma once
#include "ast.hpp"

struct ASTVisitor {
  virtual ~ASTVisitor() = default;
  virtual void visit(Literal *literal) = 0;
  virtual void visit(ASTNode *node) = 0;
  virtual void visit(Executable *node) = 0;
  virtual void visit(Statement *node) = 0;
  virtual void visit(Program *program) = 0;
  virtual void visit(Declaration *) = 0;
  virtual void visit(Expression *expression) = 0;
  virtual void visit(Operand *operand) = 0;
  virtual void visit(Identifier *identifier) = 0;
  virtual void visit(Arguments *arguments) = 0;
  virtual void visit(TupleInitializer *tupleInitializer) = 0;
  virtual void visit(Property *property) = 0;
  virtual void visit(Parameters *parameters) = 0;
  virtual void visit(Continue *cont) = 0;
  virtual void visit(Break *brk) = 0;
  virtual void visit(Return *ret) = 0;
  virtual void visit(Delete *del) = 0;
  virtual void visit(Block *block) = 0;
  virtual void visit(ObjectInitializer *objInit) = 0;
  virtual void visit(Call *call) = 0;
  virtual void visit(If *ifStmt) = 0;
  virtual void visit(Else *elseStmt) = 0;
  virtual void visit(For *forStmt) = 0;
  virtual void visit(RangeBasedFor *rangeFor) = 0;
  virtual void visit(Assignment *assignment) = 0;
  virtual void visit(TupleDeconstruction *tupleDec) = 0;
  virtual void visit(CompAssignExpr *compAssignExpr) = 0;
  virtual void visit(CompoundAssignment *compoundAssign) = 0;
  virtual void visit(FunctionDecl *funcDecl) = 0;
  virtual void visit(Noop *noop) = 0;
  virtual void visit(DotExpr *dotExpr) = 0;
  virtual void visit(DotAssignment *dotAssign) = 0;
  virtual void visit(DotCallStmnt *dotCall) = 0;
  virtual void visit(Subscript *subscript) = 0;
  virtual void visit(SubscriptAssignStmnt *subscriptAssign) = 0;
  virtual void visit(UnaryExpr *unaryExpr) = 0;
  virtual void visit(UnaryStatement *unaryStmt) = 0;
  virtual void visit(BinExpr *binExpr) = 0;
  virtual void visit(Using *usingStmt) = 0;
  virtual void visit(Lambda *lambda) = 0;
  virtual void visit(Match *match) = 0;
  virtual void visit(MatchStatement *matchStmt) = 0;
  virtual void visit(MethodCall *method) = 0;
};