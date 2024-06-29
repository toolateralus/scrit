#include "parser.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "lexer.hpp"
#include "value.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include "type.hpp"
#include <memory>
#include <stdexcept>

StatementPtr Parser::ParseLValuePostFix(ExpressionPtr &expr) {
  if (DotExpr *dot = dynamic_cast<DotExpr *>(expr.get())) {
    DotExpr *dotRight = dynamic_cast<DotExpr *>(dot->right.get());

    if (dotRight) {
      while (auto rDot = dynamic_cast<DotExpr *>(dotRight->right.get())) {
        dotRight = rDot;
      }
    } else {
      dotRight = dot;
    }

    if (!tokens.empty() && Peek().type == TType::Assign) {
      Eat();
      auto value = ParseExpression();
      return make_unique<DotAssignment>(info, std::move(expr),
                                        std::move(value));
    }

    if (!tokens.empty() && Peek().type == TType::Assign) {
      Eat();
      auto value = ParseExpression();
      return make_unique<DotAssignment>(info, std::move(expr),
                                        std::move(value));
    }

    if (dynamic_cast<Call *>(dotRight->right.get())) {
      return make_unique<DotCallStmnt>(info, std::move(expr));
    }

    if (dynamic_cast<CompAssignExpr *>(dotRight->right.get())) {
      return make_unique<CompoundAssignment>(info, std::move(expr));
    }
  }

  if (dynamic_cast<Subscript *>(expr.get())) {
    if (Peek().type == TType::Assign) {
      Eat();
      auto value = ParseExpression();
      return make_unique<SubscriptAssignStmnt>(info, std::move(expr),
                                               std::move(value));
    }
  }
  if (auto call = dynamic_cast<Call *>(expr.get())) {
    return make_unique<Call>(info, std::move(call->operand),
                             std::move(call->args));
  }
  if (dynamic_cast<CompAssignExpr *>(expr.get())) {
    return make_unique<CompoundAssignment>(info, std::move(expr));
  }

  auto &raw = *expr.get();
  auto name = string(typeid(raw).name());
  throw std::runtime_error("Failed to parse LValue postfix statement:: " +
                           name);
}
FunctionDeclPtr Parser::ParseFunctionDeclaration() {
  auto info = this->info;
  auto name = Expect(TType::Identifier).value;
  auto parameters = ParseParameters();
  Expect(TType::Arrow);
  auto returnType = ParseType();
  auto block = ParseBlock();
  return make_unique<FunctionDecl>(info, name, std::move(block),
                                   std::move(parameters), returnType);
}
StatementPtr Parser::ParseStatement() {
  while (tokens.size() > 0) {
    auto token = Peek();

    info = token.info;

    switch (token.family) {
    case TFamily::Identifier: {
      auto operand = ParseExpression();

      if (auto unary = dynamic_cast<UnaryExpr *>(operand.get())) {
        if (unary->op != TType::Increment && unary->op != TType::Decrement) {
          throw std::runtime_error("unexpected unary expression statement : " +
                                   TTypeToString(unary->op));
        }
        return make_unique<UnaryStatement>(info, std::move(operand));
      }

      if (auto id = dynamic_cast<Identifier *>(operand.get())) {
        return ParseIdentifierStatement(
            make_unique<Identifier>(info, id->name));
      } else if (dynamic_cast<CompAssignExpr *>(operand.get())) {
        return make_unique<CompoundAssignment>(info, std::move(operand));
      } else {
        return ParseLValuePostFix(operand);
      }
      throw std::runtime_error("Failed to start identifier statement");
    }
    case TFamily::Keyword: {
      Eat(); // consume kw
      // for func(..) {...}() anonymous func declaration & invocation
      // statements.
      if (token.type == TType::Func && Peek().type == TType::LParen) {
        return ParseAnonFuncInlineCall();
      }
      return ParseKeyword(token);
    }
    default:
      throw std::runtime_error(string("Failed to parse statement. ") +
                               info.ToString() + "\ntoken " +
                               TTypeToString(token.type));
    }
  }
  throw std::runtime_error("Unexpecrted end of input");
}

StatementPtr Parser::ParseDeclaration() {
  auto next = Peek();
  switch(next.type){
    case TType::Mut: {
      Expect(TType::Mut);
      auto next = Expect(TType::Identifier);
      auto iden = make_unique<Identifier>(info, next.value);
      return ParseAssignment(std::move(iden), Mutability::Mut);
    }
    case TType::Const: {
      Expect(TType::Const);
      auto next = Expect(TType::Identifier);
      auto iden = make_unique<Identifier>(info, next.value);
      return ParseAssignment(std::move(iden), Mutability::Const);
    }
    case TType::Identifier: {
      auto next = Expect(TType::Identifier);
      auto iden = make_unique<Identifier>(info, next.value); 
      return ParseAssignment(std::move(iden), Mutability::Const);
    }
    default: break;
  }
  throw std::runtime_error("Unable to parse declaration");
}

StatementPtr Parser::ParseKeyword(Token token) {
  switch (token.type) {
  case TType::Delete: {
    return ParseDelete();
  }
  case TType::Let: {
    return ParseDeclaration();
  }
  
  case TType::Match: {
    return ParseMatchStatement();
  }
  case TType::Func: {
    auto funcdecl = ParseFunctionDeclaration();
    auto callable = make_shared<Callable_T>(funcdecl->returnType, std::move(funcdecl->block), std::move(funcdecl->parameters));
    ASTNode::context.Insert(funcdecl->name, callable, Mutability::Const);
    return make_unique<Noop>(info);
  }
  case TType::If: {
    return ParseIf();
  }
  case TType::Else: {
    throw std::runtime_error(
        "An else must be preceeded by an if statement.");
  }
  case TType::For: {
    return ParseFor();
  }
  case TType::Continue: {
    return ParseContinue();
  }
  case TType::Return: {
    return ParseReturn();
  }
  case TType::Using: {
    return ParseUsing();
  }
  case TType::Break: {
    return ParseBreak();
  }

  default:
    throw std::runtime_error("Failed to parse keyword" +
                             TTypeToString(token.type));
  }
}
StatementPtr Parser::ParseIdentifierStatement(IdentifierPtr identifier) {
  auto token = Peek();
  switch (token.type) {
  case TType::AddEq:
  case TType::MulEq:
  case TType::SubEq:
  case TType::DivEq:
  case TType::NullCoalescingEq:
  case TType::Lambda:
  case TType::Assign: {
    if (!ASTNode::context.Find(identifier->name))  {
      throw std::runtime_error("cannot assign a non-existant identifier. use 'let ___ = ...' or let ___ : type = ...' syntax.");
    }
    return ParseAssignment(std::move(identifier));
  }
  case TType::Comma: {
    return ParseTupleDeconstruction(std::move(identifier));
  }
  case TType::LParen: {
    return ParseCall(std::move(identifier));
  }
  default:
    throw std::runtime_error(
        "failed to parse identifier statement, unexpected token: " +
        token.ToString());
  }
}
StatementPtr Parser::ParseAssignment(IdentifierPtr identifier,
                                     Mutability mutability) {
  Token next = Peek();
  
  Type type = nullptr;
  if (next.type == TType::Colon) {
    Eat();
    type = ParseType();
    
    if (tokens.empty() || Peek().type != TType::Assign) {
      return std::make_unique<Assignment>(info, type, std::move(identifier), make_unique<Operand>(info, type, TypeSystem::Current().GetDefault(type)), mutability);
    }
    next = Peek();
  }
  
  if (next.type == TType::Lambda) {
    auto lambda = ParseLambda();
    if (type && type != lambda->type) {
      throw std::runtime_error("explicit type: " + type->name + " did not match the property's type: " + lambda->type->name);
    }
    return make_unique<Property>(info,  std::move(identifier), std::move(lambda), mutability);
  }
  if (next.type == TType::Assign) {
    Eat();
    auto value = ParseExpression();
    
    if (type && type != value->type) {
      auto array_t = std::dynamic_pointer_cast<ArrayType>(type);
      if (array_t && value->type->name == "array") {
        value->type = type;
      } else {
        throw std::runtime_error("explicit type: " + type->name + " did not match the assignment's type: " + value->type->name);
      }
    }
    
    return make_unique<Assignment>(info, value->type, std::move(identifier),
                                   std::move(value), mutability);

  } else if (IsCompoundAssignmentOperator(next.type)) {
    Eat();
    auto value = ParseExpression();
    
    if (type && type != value->type) {
      throw std::runtime_error("explicit type: " + type->name + " did not match the expressions type: " + value->type->name);
    }
    
    return make_unique<CompoundAssignment>(
        info, make_unique<CompAssignExpr>(info,std::move(identifier),
                                          std::move(value), next.type));
  } else {
    throw std::runtime_error("failed to parse assignment: invalid operator." +
                             TTypeToString(next.type));
  }
}
StatementPtr Parser::ParseCall(IdentifierPtr identifier) {
  auto info = this->info;
  auto args = ParseArguments();
  return make_unique<Call>(info, std::move(identifier), std::move(args));
}
ExpressionPtr Parser::ParseExpression() {
  if (tokens.empty()) {
    throw std::runtime_error("Unexpected end of input");
  }
  return ParseCompoundAssignment();
}
ExpressionPtr Parser::ParseCompoundAssignment() {
  auto left = ParseLogicalOr();

  if (!tokens.empty()) {
    auto next = Peek();
    if (!IsCompoundAssignmentOperator(next.type)) {
      return left;
    }
    // TODO: we need to make sure the LHS is not a literal.
    if (dynamic_cast<Operand *>(left.get())) {
      throw std::runtime_error("cannot use compound assignment on a literal");
    }

    Eat();
    auto expr = ParseExpression();
    return make_unique<CompAssignExpr>(info, std::move(left), std::move(expr),
                                       next.type);
  }
  return left;
}
ExpressionPtr Parser::ParseLogicalOr() {
  auto left = ParseLogicalAnd();

  bool wasNullCoalescing = false;

  while (!tokens.empty() && Peek().type == TType::NullCoalescing) {
    wasNullCoalescing = true;
    Eat();
    auto right = ParseLogicalAnd();
    
    left = make_unique<BinExpr>(info, std::move(left), std::move(right),
                                TType::NullCoalescing);
  }

  if (wasNullCoalescing) {
    return left;
  }

  while (!tokens.empty() && Peek().type == TType::Or) {
    Eat();
    auto right = ParseLogicalAnd();
    left = std::make_unique<BinExpr>(info, std::move(left), std::move(right),
                                     TType::Or);
  }

  return left;
}
ExpressionPtr Parser::ParseLogicalAnd() {
  auto left = ParseEquality();

  while (!tokens.empty() && Peek().type == TType::And) {
    Eat();
    auto right = ParseEquality();
    left = std::make_unique<BinExpr>(info, std::move(left), std::move(right),
                                     TType::And);
  }

  return left;
}
ExpressionPtr Parser::ParseEquality() {
  auto left = ParseComparison();

  while (!tokens.empty() &&
         (Peek().type == TType::Equals || Peek().type == TType::NotEquals)) {
    auto op = Peek().type;
    Eat();
    auto right = ParseComparison();
    left =
        std::make_unique<BinExpr>(info, std::move(left), std::move(right), op);
  }

  return left;
}
ExpressionPtr Parser::ParseComparison() {
  auto left = ParseTerm();

  while (!tokens.empty() &&
         (Peek().type == TType::Less || Peek().type == TType::LessEq ||
          Peek().type == TType::Greater || Peek().type == TType::GreaterEq)) {
    auto op = Peek().type;
    Eat();
    auto right = ParseTerm();
    left =
        std::make_unique<BinExpr>(info, std::move(left), std::move(right), op);
  }

  return left;
}
ExpressionPtr Parser::ParseTerm() {
  auto left = ParseFactor();

  while (!tokens.empty() &&
         (Peek().type == TType::Add || Peek().type == TType::Sub)) {
    auto op = Peek().type;
    Eat();
    auto right = ParseFactor();
    left =
        std::make_unique<BinExpr>(info, std::move(left), std::move(right), op);
  }

  return left;
}
ExpressionPtr Parser::ParseFactor() {
  auto left = ParsePostfix();

  while (!tokens.empty() &&
         (Peek().type == TType::Mul || Peek().type == TType::Div)) {
    auto op = Peek().type;
    Eat();
    auto right = ParsePostfix();
    left =
        std::make_unique<BinExpr>(info, std::move(left), std::move(right), op);
  }

  return left;
}
ExpressionPtr Parser::ParsePostfix() {
  auto expr = ParseOperand();
  while (!tokens.empty()) {
    Token next = Peek();

    if (next.type == TType::Increment || next.type == TType::Decrement) {
      Eat();
      return make_unique<UnaryExpr>(info, expr->type, std::move(expr), next.type);
    }

    if (next.type != TType::LParen && next.type != TType::SubscriptLeft &&
        next.type != TType::Dot) {
      break;
    }
    if (next.type == TType::LParen) {
      auto args = ParseArguments();
      expr = std::make_unique<Call>(info, std::move(expr), std::move(args));
    } else if (next.type == TType::SubscriptLeft) {
      Eat();
      auto index = ParseExpression();
      Expect(TType::SubscriptRight);
      expr =
          std::make_unique<Subscript>(info, expr->type, std::move(expr), std::move(index));
    } else if (next.type == TType::Dot) {
      Eat();
      auto right = ParseExpression();
      expr = std::make_unique<DotExpr>(info, right->type, std::move(expr), std::move(right));
    }
  }

  return expr;
}
/*
   This is for functions as such:
   // declare an anon func.
   func(){
      // do something
   }() // <- this '()' operator calls this anon func in place.


   This is equivalent to a block scope in a C function body, like:

   `int main() {
    // some c code...
    {
      // some scoped c code...
    }
    return 0;
   }`

*/
StatementPtr Parser::ParseAnonFuncInlineCall() {
  auto info = this->info;
  auto parameters = ParseParameters();
  auto returnType = ParseType();
  auto body = ParseBlock();
  auto arguments = ParseArguments();
  
  auto type = Values::TypeSystem::Current().FromCallable(returnType, parameters->ParamTypes());
  auto op = make_unique<Operand>(info, type, make_shared<Callable_T>(returnType, std::move(body), std::move(parameters)));
  return make_unique<Call>(info, std::move(op), std::move(arguments));
}

/*
  This parses
  func() { ... }

  See ParseAnonFuncInlineCall for anon functions that call themselsves
  immedately, ie a C style block scope.


*/
ExpressionPtr Parser::ParseAnonFunc() {
  auto info = this->info;
  Eat(); // eat the 'func' keyword.
  auto params = ParseParameters();
  Type returnType = nullptr;
  if (Peek().type == TType::LCurly) {
    returnType = TypeSystem::Current().Undefined();
  } else {
    Expect(TType::Arrow);
    returnType = ParseType();
  }
  auto body = ParseBlock();
  auto types = params->ParamTypes();
  auto callable = make_shared<Callable_T>(returnType, std::move(body), std::move(params));
  return make_unique<Operand>(info, TypeSystem::Current().FromCallable(returnType, types), callable);
}

ExpressionPtr Parser::ParseObjectInitializer() {
  Expect(TType::LCurly);
  vector<StatementPtr> statements = {};
  
  while (tokens.size() > 0) {
    auto next = Peek();
    
    switch (next.type) {
    //
    case TType::Comma: {
      Eat();
      continue;
    }
    
    // break the loop not the switch.
    case TType::RCurly:
      goto endloop;
    
    case TType::Func:
      Eat(); // eat keyword.
      statements.push_back(ParseFunctionDeclaration());
      break;
    
    case TType::Mut:
    case TType::Const: {
      auto mutability =
          Eat().type == TType::Mut ? Mutability::Mut : Mutability::Const;
      auto iden = Expect(TType::Identifier);
      statements.push_back(ParseAssignment(
          make_unique<Identifier>(info, iden.value), mutability));
      break;
    }
    case TType::Identifier: {
      auto iden = Eat();
      
      // this.something = blah
      // dot assignment for aliases: 
      // purely to prevent naming conflicts.
      if (Peek().type == TType::Dot) {
        tokens.push_back(iden);
        auto dot = ParsePostfix();
        auto assign = ParseLValuePostFix(dot);
        statements.push_back(std::move(assign));
        break;
      }
      
      statements.push_back(ParseAssignment(
        make_unique<Identifier>(info, iden.value), Mutability::Const));
      break;
    }

    default:
      throw std::runtime_error(
          "Invalid statement in object initalizer: " +
          TTypeToString(next.type) +
          " you may only have variable declarations/assignment and function "
          "declarations." +
          info.ToString());
    }
  }
endloop:
  
  Expect(TType::RCurly);
  
  // todo: redo the object system and type it.
  return make_unique<ObjectInitializer>(info, nullptr, make_unique<Block>(info, std::move(statements)));
}

ExpressionPtr Parser::ParseTuple(ExpressionPtr &&expr) {
  Eat(); // eat first comma.
  vector<ExpressionPtr> values;
  values.push_back(std::move(expr));
  
  while (!tokens.empty()) {
    auto next = Peek();
    
    if (next.type == TType::RParen) {
      break;
    }
    
    values.push_back(ParseExpression());
    
    if (Peek().type == TType::Comma) {
      Eat();
    }
  }
  Expect(TType::RParen);
  return make_unique<TupleInitializer>(info, std::move(values));
}

ExpressionPtr Parser::ParseOperand() {
  auto token = Peek();
  if (token.type == TType::Sub || token.type == TType::Not ||
      token.type == TType::Increment || token.type == TType::Decrement) {
    Eat();
    auto operand = ParseExpression();
    return make_unique<UnaryExpr>(info, operand->type, std::move(operand), token.type);
  }
  
  switch (token.type) {
  case TType::Match: {
    Eat();
    return ParseMatch();
  }
  case TType::SubscriptLeft: {
    return ParseArrayInitializer();
  }
  case TType::Func: {
    return ParseAnonFunc();
  }
  case TType::LCurly: {
    return ParseObjectInitializer();
  }
  case TType::Lambda: {
    return ParseLambda();
  }
  case TType::String:
    Eat();
    return make_unique<Operand>(info, TypeSystem::Current().Get("string"), String_T::New(std::move(token.value)));
  case TType::True:
    Eat();
    return make_unique<Operand>(info, TypeSystem::Current().Get("bool"), Value_T::True);
  case TType::False:
    Eat();
    return make_unique<Operand>(info, TypeSystem::Current().Get("bool"), Value_T::False);
  case TType::Undefined:
    Eat();
    return make_unique<Operand>(info, TypeSystem::Current().Get("undefined"), Value_T::UNDEFINED);
  case TType::Null:
    Eat();
    return make_unique<Operand>(info, TypeSystem::Current().Get("null"), Value_T::VNULL);
  case TType::Float:
    Eat();
    return make_unique<Operand>(info, TypeSystem::Current().Get("float"), Float_T::New(stof(token.value)));
  case TType::Int:
    Eat();
    return make_unique<Operand>(info, TypeSystem::Current().Get("int"),Int_T::New(stoi(token.value)));
  case TType::Identifier:
    Eat();
    return make_unique<Identifier>(info, token.value);
  case TType::LParen: {
    Eat();
    auto expr = ParseExpression();
    
    if (Peek().type == TType::Comma) {
      return ParseTuple(std::move(expr));
    }
    
    Expect(TType::RParen);
    return expr;
  }
  default:
    throw std::runtime_error("Unexpected token: " + TTypeToString(token.type));
  }
}

ExpressionPtr Parser::ParseLambda() {
  Expect(TType::Lambda);
  // here we use the lambda to say this block's result is a value, an expression
  // block. this differs from the normal = {} which declares an object.
  // => {}
  if (!tokens.empty() && Peek().type == TType::LCurly) {
    auto block = ParseBlock();
    
    Type t;
    for (const auto &statement: block->statements) {
      if (auto ret = dynamic_cast<Return *>(statement.get())) {
        t = ret->value->type;
      }
    }
    
    return make_unique<Lambda>(info, t, std::move(block));
  }
  // here we use lambda as basically an implicit return.
  // => some_expression
  else {
    auto expr = ParseExpression();
    return make_unique<Lambda>(info, expr->type, std::move(expr));
  }
}

ExpressionPtr Parser::ParseMatch() {
  auto expr = ParseExpression();
  Expect(TType::LCurly);

  std::vector<ExpressionPtr> statements = {};
  std::vector<ExpressionPtr> expressions = {};

  ExpressionPtr default_branch = nullptr;

  while (!tokens.empty() && Peek().type != TType::RCurly) {

    if (Peek().type == TType::Default) {
      Eat();
      default_branch = ParseLambda();
    } else {
      // 0 => {}
      expressions.push_back(ParseExpression());
      statements.push_back(ParseLambda());
    }
  }

  Expect(TType::RCurly);
  
  // TODO: fix this. this restricts a match expression to only return the type its matching against,
  // which makes literally 0 sense.
  return make_unique<Match>(info, expr->type, std::move(expr), std::move(expressions),
                            std::move(statements), std::move(default_branch));
}

OperandPtr Parser::ParseArrayInitializer() {
  Eat();
  if (Peek().type == TType::SubscriptRight) {
    Eat();
    auto array = Array_T::New();
    return make_unique<Operand>(info, TypeSystem::Current().Get("array"), array);
  } else {
    vector<ExpressionPtr> values = {};
    
    Type inner_type;
    while (Peek().type != TType::SubscriptRight) {
      auto val = ParseExpression();
      
      if (!inner_type) {
        inner_type = val->type;
      }
      
      if (inner_type && !Type_T::equals(val->type.get(), inner_type.get())) {
        throw std::runtime_error("invalid type in array initializer\nexpected: " + inner_type->name + "\ngot: " + val->type->name);
      }
      
      values.push_back(std::move(val));
      
      if (Peek().type == TType::Comma) {
        Eat();
      }
    }
    Expect(TType::SubscriptRight);
    auto array = Array_T::New(std::move(values));
    auto type = TypeSystem::Current().ArrayTypeFromInner(inner_type);
    array->type = type;
    std::cout << "parsing array of type : " << type->name  << std::endl;
    return make_unique<Operand>(info, type, array);
  }
}
ParametersPtr Parser::ParseParameters() {
  Expect(TType::LParen);
  auto next = Peek();
  std::map<string, Parameters::Param> params = {};
  while (tokens.size() > 0 && next.type != TType::RParen) {
    
    auto value = ParseOperand();
    Expect(TType::Colon);
    auto tname = ParseType();
    
    if (auto iden = dynamic_cast<Identifier *>(value.get())) {
      params[iden->name] = {  .value = nullptr, .type= tname, };
      
      if (Peek().type == TType::Assign) {
        Eat();
        auto value = ParseExpression();
        params[iden->name] = { .value = value->Evaluate(), .type =  value->type};
      }
    }
    
    if (Peek().type == TType::Comma) {
      Eat();
    }
    next = Peek();
  }
  Expect(TType::RParen);
  return make_unique<Parameters>(info, std::move(params));
}
ArgumentsPtr Parser::ParseArguments() {
  Expect(TType::LParen);
  auto next = Peek();
  vector<ExpressionPtr> values = {};
  
  while (tokens.size() > 0 && next.type != TType::RParen) {
    auto value = ParseExpression();
    values.push_back(std::move(value));
    if (tokens.empty()) {
      throw std::runtime_error("unmatched parens, or incomplete expression");
    }
    
    if (Peek().type == TType::Comma) {
      Eat();
    } else {
      next = Peek();
      if (next.type != TType::RParen) {
        throw std::runtime_error("Expected a comma in arguments list.");
      }
    }
  }
  Expect(TType::RParen);
  return make_unique<Arguments>(info, std::move(values));
}
BlockPtr Parser::ParseBlock() {
  auto info = this->info;
  Expect(TType::LCurly);
  vector<StatementPtr> statements = {};
  auto next = Peek();

  // Empty block.
  if (next.type == TType::RCurly) {
    Eat();
    return make_unique<Block>(info, std::move(statements));
  }

  while (tokens.size() > 0) {

    // If the last line of a block is an identifier or a literal, we just create
    // an implicit return. I am not sure that it's possible, but I would love to
    // have a rust-like clone of their implicit returns. that is, any expression
    // can be the last 'statement' of a block and have an implicit return. like
    // a match or complex expression etc.
    if (Peek(1).type == TType::RCurly &&
        (Peek().family == TFamily::Identifier ||
         Peek().family == TFamily::Literal)) {
      statements.push_back(make_unique<Return>(info, ParseExpression()));
      break;
    }

    // add each statement to the block.
    auto statement = ParseStatement();
    statements.push_back(std::move(statement));
    next = Peek();

    // If we get a return statement, we just discard any unreachable code.
    // this reduces some memory usage and simplifies the AST.
    if (dynamic_cast<Return *>(statement.get())) {
      // eat up the remainder of the block following the return statement we
      // just paresed.
      while (!tokens.empty() && next.type != TType::RCurly) {
        next = Eat();
      }
      // break the parse loop.
      break;
    }

    if (next.type == TType::RCurly) {
      break;
    }
  }
  Expect(TType::RCurly);
  return make_unique<Block>(info, std::move(statements));
}
ElsePtr Parser::ParseElse() {
  auto info = this->info;
  Eat();
  if (Peek().type == TType::If) {
    Eat();
    auto ifstmnt = ParseIf();
    return Else::New(info, std::move(ifstmnt));
  } else {
    return Else::NoIf(info, ParseBlock());
  }
}
IfPtr Parser::ParseIf() {
  auto info = this->info;
  auto condition = ParseExpression();
  auto block = ParseBlock();
  if (!tokens.empty() && Peek().type == TType::Else) {
    auto elseStmnt = ParseElse();
    return If::WithElse(info, std::move(condition), std::move(block),
                        std::move(elseStmnt));
  }
  return If::NoElse(info, std::move(condition), std::move(block));
}
StatementPtr Parser::ParseFor() {
  auto info = this->info;

  auto scope = ASTNode::context.PushScope();

  if (!tokens.empty() && Peek().type == TType::LParen) {
    Eat();
  }
  StatementPtr decl = nullptr;
  ExpressionPtr condition = nullptr;
  StatementPtr inc = nullptr;

  // for {}
  if (Peek().type == TType::LCurly) {
    return make_unique<For>(info, nullptr, nullptr, nullptr, ParseBlock(),
                            ASTNode::context.PopScope());
  }

  if ((tokens.size() > 1 && Peek(1).type == TType::Assign) ||
      (tokens.size() > 2 && Peek(2).type == TType::Assign)) {
    decl = ParseStatement();
    Expect(TType::Comma);
    condition = ParseExpression();
    Expect(TType::Comma);
    inc = ParseStatement();
    return make_unique<For>(info, std::move(decl), std::move(condition),
                            std::move(inc), ParseBlock(),
                            ASTNode::context.PopScope());
  }

  auto expr = ParseExpression();

  // for expr : array/obj {}
  if (Peek().type == TType::Colon) {
    Eat();
    auto rhs = ParseExpression();
    return make_unique<RangeBasedFor>(info, std::move(expr), std::move(rhs),
                                      ParseBlock());
  }

  // for CONDITION {}
  return make_unique<For>(info, std::move(decl), std::move(expr),
                          std::move(inc), ParseBlock(),
                          ASTNode::context.PopScope());
}
StatementPtr Parser::ParseContinue() { return make_unique<Continue>(info); }
StatementPtr Parser::ParseReturn() {
  auto next = Peek();

  // This riduculous if statement is to check if the next token is a part of an
  // expression. this is how we judge whether to parse a return expression or
  // not. We should probably make a better way to do this, just don't know how.
  if (tokens.empty() ||
      (next.family == TFamily::Keyword && next.type != TType::Null &&
       next.type != TType::Undefined && next.type != TType::False &&
       next.type != TType::True && next.type != TType::Match) ||
      (next.family == TFamily::Operator && next.type != TType::LParen &&
       next.type != TType::LCurly &&
       (next.type != TType::Sub && next.type != TType::Not))) {
    return make_unique<Return>(info);
  }
  return make_unique<Return>(info, ParseExpression());
}
StatementPtr Parser::ParseBreak() { return make_unique<Break>(info); }
Token Parser::Peek(size_t lookahead) {
  if (lookahead >= tokens.size()) {
    throw std::out_of_range("Lookahead is out of range");
  }
  return tokens[tokens.size() - 1 - lookahead];
}
Token Parser::Eat() {
  auto tkn = tokens.back();
  tokens.pop_back();
  return tkn;
}
Token Parser::Expect(const TType ttype) {
  if (tokens.back().type != ttype) {
    throw std::runtime_error("Expected " + TTypeToString(ttype) + " got " +
                             TTypeToString(tokens.back().type));
  }
  auto tkn = tokens.back();
  tokens.pop_back();
  return tkn;
}
unique_ptr<Program> Parser::Parse(vector<Token> &&tokens) {
  std::reverse(tokens.begin(), tokens.end());
  this->tokens = std::move(tokens);
  vector<StatementPtr> statements;
  while (this->tokens.size() > 0) {
    try {
      auto statement = ParseStatement();
      statements.push_back(std::move(statement));
    } catch (std::runtime_error e) {

      auto what = string(e.what());

      if (!what.contains("source_info:")) {
        what += info.ToString();
      }

      std::cout << "Parser exception! : " << what << std::endl;
      // try to eat a token. This will eventuallya llow the parser to continue
      // normally if any well formed code exists past this exception. However,
      // this results in a bulk of unrelated errors.

      // if there are none left, just stop.
      if (tokens.empty()) {
        break;
      }
      Eat();
    }
  }
  auto program = make_unique<Program>(std::move(statements));
  return program;
}
StatementPtr Parser::ParseUsing() {
  auto next = Peek();

  // using all * widlcard
  if (next.type == TType::Mul) {
    Eat();
    Expect(TType::From);
    auto iden = Expect(TType::Identifier);
    return make_unique<Using>(info, iden.value, true);

  }
  // plain 'using raylib' statement
  else if (next.type == TType::Identifier) {
    auto iden = Expect(TType::Identifier);
    return make_unique<Using>(info, iden.value, false);
  }
  // 'using {iden, iden} from raylib'
  else if (next.type == TType::LCurly) {
    Eat();
    vector<string> names = {};
    while (!tokens.empty()) {
      auto next = Peek();

      if (next.type == TType::Comma) {
        Eat();
        continue;
      }

      if (next.type == TType::RCurly) {
        break;
      }
      auto operand = ParseOperand();

      if (auto iden = dynamic_cast<Identifier *>(operand.get())) {
        names.push_back(iden->name);
      } else {
        throw std::runtime_error("invalid using statement");
      }
    }
    Expect(TType::RCurly);
    Expect(TType::From);
    auto iden = Expect(TType::Identifier);

    return make_unique<Using>(info, iden.value, names);
  }
  throw std::runtime_error("Failed to parse using statement");
}

// This makes it seem like we could use an 'Expression Statement' kind of
// wrapper for generalized cases. Then, for and if can be expressions and we can
// just have several uses for them. I like returning from for loops into a
// variable. Just like rust.
StatementPtr Parser::ParseMatchStatement() {
  return make_unique<MatchStatement>(info, ParseMatch());
}
DeletePtr Parser::ParseDelete() {
  
  if (Peek(1).type == TType::Dot) {
    return make_unique<Delete>(info, ParsePostfix());
  }
  
  auto iden = Expect(TType::Identifier);
  return make_unique<Delete>(info, make_unique<Identifier>(info, iden.value));
}

StatementPtr Parser::ParseTupleDeconstruction(IdentifierPtr &&iden) {
  vector<IdentifierPtr> idens;
  idens.push_back(std::move(iden));
  Expect(TType::Comma);
  
  while (!tokens.empty()) {
    if (Peek().type == TType::Assign) {
      break;
    }
    
    auto iden = Expect(TType::Identifier);
    idens.push_back(make_unique<Identifier>(info, iden.value));
    
    if (Peek().type == TType::Comma) {
      Eat();
    }
  }
  
  Expect(TType::Assign);
  
  auto tuple = ParseExpression();
  
  return make_unique<TupleDeconstruction>(info, std::move(idens), std::move(tuple));
}

Type Parser::ParseType() {
  auto tname = Eat();
  
  if (Peek().type == TType::Less) {
    Eat(); 
    vector<string> names;
    while (!tokens.empty()) {
      auto next = Peek();
      
      if (next.type == TType::Greater) {
        break;
      }
      
      names.push_back(Expect(TType::Identifier).value);
      
      if (Peek().type == TType::Comma) {
        Eat();
      }
      
    }
    Expect(TType::Greater);
    
    std::string typeString = tname.value + "<";
    for (const auto& name : names) {
      typeString += name + ",";
    }
    typeString.pop_back(); // remove the last comma
    typeString += ">";
    auto t = TypeSystem::Current().Get(typeString);
    
    if (t == nullptr) {
      auto template_t = TypeSystem::Current().Get(tname.value);
      if (template_t == nullptr || template_t->name != "array") {
        throw std::runtime_error("right now generics are only supported for typed arrays.\ncannot make generic type of non existant type: " + tname.value);
      }
      auto n0 = names[0];
      auto n0_t = TypeSystem::Current().Get(n0);
      if (!n0_t) {
        throw std::runtime_error("cannot make generic type argument for non existant type: " + n0);
      }
      return TypeSystem::Current().ArrayTypeFromInner(n0_t);
    }
    return t;
  }
  
  return TypeSystem::Current().Get(tname.value);
}
