#include "parser.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "lexer.hpp"
#include "value.hpp"
#include <algorithm>
#include <stdexcept>

#include <memory>
#include <stdexcept>

StatementPtr Parser::ParseLValuePostFix(ExpressionPtr &expr) {
  if (DotExpr *dot = dynamic_cast<DotExpr *>(expr.get())) {
    DotExpr *dotRight = dynamic_cast<DotExpr *>(dot->right.get());
    
    if (dotRight) {
      while (auto rDot = dynamic_cast<DotExpr*>(dotRight->right.get())) {
        dotRight = rDot; 
      }
    } else {
      dotRight = dot;
    }
    
    
    if (!tokens.empty() && Peek().type == TType::Assign) {
      Eat();
      auto value = ParseExpression();
      return make_unique<DotAssignment>(info,  std::move(expr), std::move(value));
    } 
   
    if (!tokens.empty() && Peek().type == TType::Assign) {
      Eat();
      auto value = ParseExpression();
      return make_unique<DotAssignment>(info,  std::move(expr), std::move(value));
    } 
    
    if (dynamic_cast<Call *>(dotRight->right.get())) {
      return make_unique<DotCallStmnt>(info,  std::move(expr));
    } 
    
    if (dynamic_cast<CompAssignExpr *>(dotRight->right.get())) {
      return make_unique<CompoundAssignment>(info,  std::move(expr));
    }
    
  } 
  
  if (dynamic_cast<Subscript *>(expr.get())) {
    if (Peek().type == TType::Assign) {
      Eat();
      auto value = ParseExpression();
      return make_unique<SubscriptAssignStmnt>(info,  std::move(expr),
                                               std::move(value));
    }
  } 
  if (auto call = dynamic_cast<Call *>(expr.get())) {
    return make_unique<Call>(info,  std::move(call->operand), std::move(call->args));
  } 
  if (dynamic_cast<CompAssignExpr *>(expr.get())) {
    return make_unique<CompoundAssignment>(info,  std::move(expr));
  }
  
  auto &raw = *expr.get();
  auto name = string(typeid(raw).name());
  throw std::runtime_error("Failed to parse LValue postfix statement:: " +
                           name);
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
        return make_unique<UnaryStatement>(info,  std::move(operand));
      }
      
      if (auto id = dynamic_cast<Identifier *>(operand.get())) {
        return ParseIdentifierStatement(make_unique<Identifier>(info,  id->name));
      } else if (dynamic_cast<CompAssignExpr *>(operand.get())) {
        return make_unique<CompoundAssignment>(info,  std::move(operand));
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
        auto info = this->info;
        auto parameters = ParseParameters();
        auto body = ParseBlock();
        auto arguments = ParseArguments();
        auto op = make_unique<Operand>(info,  
            make_shared<Callable_T>(std::move(body), std::move(parameters)));
        return make_unique<Call>(info,  std::move(op), std::move(arguments));
      }
      return ParseKeyword(token);
    }
    default:
      throw std::runtime_error("Failed to parse statement:: token " +
                               TTypeToString(token.type));
    }
  }
  throw std::runtime_error("Unexpecrted end of input");
}
StatementPtr Parser::ParseKeyword(Token token) {
  switch (token.type) {
  case TType::Func: {
    auto info = this->info;
    auto name = Expect(TType::Identifier);
    auto parameters = ParseParameters();
    auto block = ParseBlock();
    ASTNode::context.Insert(
        name.value,
        make_shared<Callable_T>(std::move(block), std::move(parameters)));
    return make_unique<Noop>(info);
  }
  case TType::If: {
    return ParseIf();
  }
  case TType::Else: {
    throw new std::runtime_error(
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
    // TODO: implement these functionalities.
  // case TType::Start: {
  //   return new Coroutine(ParseStatement());
  // }
  // case TType::Module: {
  //   Expect(TType::Identifier);
  //   return new NoopStatement();
  // }
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
  case TType::Assign: {
    return ParseAssignment(std::move(identifier));
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
StatementPtr Parser::ParseAssignment(IdentifierPtr identifier) {
  Token next = Peek();
  if (next.type == TType::Assign) {
    Eat();
    auto value = ParseExpression();
    return make_unique<Assignment>(info,  std::move(identifier), std::move(value));
  } else if (IsCompoundAssignmentOperator(next.type)) {
    Eat();
    auto value = ParseExpression();
    return make_unique<CompoundAssignment>(info,  make_unique<CompAssignExpr>(info, 
        std::move(identifier), std::move(value), next.type));
  } else {
    throw std::runtime_error("failed to parse assignment: invalid operator.");
  }
}
StatementPtr Parser::ParseCall(IdentifierPtr identifier) {
  auto info = this->info;
  auto args = ParseArguments();
  return make_unique<Call>(info,  std::move(identifier), std::move(args));
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
    if (dynamic_cast<Operand*>(left.get())) {
      throw std::runtime_error("cannot use compound assignment on a literal");
    }
    
    Eat();
    auto expr = ParseExpression();
    return make_unique<CompAssignExpr>(info,  std::move(left), std::move(expr),
                                       next.type);
  }
  return left;
}
ExpressionPtr Parser::ParseLogicalOr() {
  auto left = ParseLogicalAnd();
  
  bool wasNullCoalescing = false;
  
  while (!tokens.empty() && Peek().type == TType::NullCoalescing) {
    wasNullCoalescing =true;
    Eat();
    auto right = ParseLogicalAnd();
    left = make_unique<BinExpr>(info, std::move(left), std::move(right), TType::NullCoalescing);
  }
  
  if (wasNullCoalescing) {
    return left;
  }
  
  while (!tokens.empty() && Peek().type == TType::Or) {
    Eat();
    auto right = ParseLogicalAnd();
    left =  std::make_unique<BinExpr>(info,  std::move(left), std::move(right),
                                     TType::Or);
  }
  
  return left;
}
ExpressionPtr Parser::ParseLogicalAnd() {
  auto left = ParseEquality();

  while (!tokens.empty() && Peek().type == TType::And) {
    Eat();
    auto right = ParseEquality();
    left = std::make_unique<BinExpr>(info,  std::move(left), std::move(right),
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
    left = std::make_unique<BinExpr>(info,  std::move(left), std::move(right), op);
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
    left = std::make_unique<BinExpr>(info,  std::move(left), std::move(right), op);
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
    left = std::make_unique<BinExpr>(info,  std::move(left), std::move(right), op);
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
    left = std::make_unique<BinExpr>(info,  std::move(left), std::move(right), op);
  }

  return left;
}
ExpressionPtr Parser::ParsePostfix() {
  auto expr = ParseOperand();
  while (!tokens.empty()) {
    Token next = Peek();

    if (next.type == TType::Increment || next.type == TType::Decrement) {
      Eat();
      return make_unique<UnaryExpr>(info,  std::move(expr), next.type);
    }

    if (next.type != TType::LParen && next.type != TType::SubscriptLeft &&
        next.type != TType::Dot) {
      break;
    }
    if (next.type == TType::LParen) {
      auto args = ParseArguments();
      expr = std::make_unique<Call>(info,  std::move(expr), std::move(args));
    } else if (next.type == TType::SubscriptLeft) {
      Eat();
      auto index = ParseExpression();
      Expect(TType::SubscriptRight);
      expr = std::make_unique<Subscript>(info,  std::move(expr), std::move(index));
    } else if (next.type == TType::Dot) {
      Eat();
      auto right = ParseExpression();
      expr = std::make_unique<DotExpr>(info,  std::move(expr), std::move(right));
    }
  }

  return expr;
}
ExpressionPtr Parser::ParseOperand() {
  auto token = Peek();
  if (token.type == TType::Sub || token.type == TType::Not ||
      token.type == TType::Increment || token.type == TType::Decrement) {
    Eat();
    auto operand = ParseOperand();
    return make_unique<UnaryExpr>(info,  std::move(operand), token.type);
  }
  
  switch (token.type) {
  case TType::SubscriptLeft: {
    return ParseArrayInitializer();
  }
  case TType::Func: {
    auto info = this->info;
    Eat();
    auto params = ParseParameters();
    auto body = ParseBlock();
    auto callable = make_shared<Callable_T>(std::move(body), std::move(params));
    return make_unique<Operand>(info,  callable);
  }
  case TType::LCurly: {
    Eat();
    vector<StatementPtr> statements = {};
    auto scope = ASTNode::context.PushScope();
    while (tokens.size() > 0) {
      auto next = Peek();
      if (next.type == TType::Comma) {
        Eat();
        continue;
      }
      if (next.type == TType::RCurly) {
        break;
      }
      auto statement = ParseStatement();
      statements.push_back(std::move(statement));
    }
    Expect(TType::RCurly);
    ASTNode::context.PopScope();
    return make_unique<ObjectInitializer>(info,  make_unique<Block>(info,  std::move(statements)), std::move(scope));
  }
  case TType::String:
    Eat();
    return make_unique<Operand>(info,  String_T::New(std::move(token.value)));
  case TType::True:
    Eat();
    return make_unique<Operand>(info,  Value_T::True);
  case TType::False:
    Eat();
    return make_unique<Operand>(info,  Value_T::False);
  case TType::Undefined:
    Eat();
    return make_unique<Operand>(info,  Value_T::UNDEFINED);
  case TType::Null:
    Eat();
    return make_unique<Operand>(info,  Value_T::VNULL);
  case TType::Float:
    Eat();
    return make_unique<Operand>(info,  Float_T::New(stof(token.value)));
  case TType::Int:
    Eat();
    return make_unique<Operand>(info,  Int_T::New(stoi(token.value)));
  case TType::Identifier:
    Eat();
    return make_unique<Identifier>(info,  token.value);
  case TType::LParen: {
    Eat();
    auto expr = ParseExpression();
    Expect(TType::RParen);
    return expr;
  }
  default:
    throw std::runtime_error("Unexpected token: " + TTypeToString(token.type));
  }
}
OperandPtr Parser::ParseArrayInitializer() {
  Eat();
  if (Peek().type == TType::SubscriptRight) {
    Eat();
    auto array = Array_T::New();
    return make_unique<Operand>(info,  array);
  } else {
    vector<ExpressionPtr> values = {};
    while (Peek().type != TType::SubscriptRight) {
      auto val = ParseExpression();
      values.push_back(std::move(val));
      if (Peek().type == TType::Comma) {
        Eat();
      }
    }
    Expect(TType::SubscriptRight);
    auto array = Array_T::New(std::move(values));
    return make_unique<Operand>(info,  array);
  }
}
ParametersPtr Parser::ParseParameters() {
  Expect(TType::LParen);
  auto next = Peek();
  std::map<string, Value> params = {};
  while (tokens.size() > 0 && next.type != TType::RParen) {
    auto value = ParseOperand();
    if (auto iden = dynamic_cast<Identifier *>(value.get())) {
      params[iden->name] = nullptr;
      if (Peek().type == TType::Assign) {
        Eat();
        auto value = ParseExpression();
        params[iden->name] = value->Evaluate();
      }
    }
  
    if (Peek().type == TType::Comma) {
      Eat();
    }
    next = Peek();
  }
  Expect(TType::RParen);
  return make_unique<Parameters>(info,  std::move(params));
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
  return make_unique<Arguments>(info,  std::move(values));
}
BlockPtr Parser::ParseBlock() {
  auto info = this->info;
  Expect(TType::LCurly);
  vector<StatementPtr> statements = {};
  auto next = Peek();
  if (next.type == TType::RCurly) {
    Eat();
    return make_unique<Block>(info,  std::move(statements));
  }

  while (tokens.size() > 0) {
    auto statement = ParseStatement();
    statements.push_back(std::move(statement));
    next = Peek();
    if (next.type == TType::RCurly) {
      break;
    }
  }
  Expect(TType::RCurly);
  return make_unique<Block>(info,  std::move(statements));
}
ElsePtr Parser::ParseElse() {
  auto info = this->info;
  Eat();
  if (Peek().type == TType::If) {
    Eat();
    auto ifstmnt = ParseIf();
    return Else::New(info,  std::move(ifstmnt));
  } else {
    return Else::NoIf(info,  ParseBlock());
  }
}
IfPtr Parser::ParseIf() {
  auto info = this->info;
  auto condition = ParseExpression();
  auto block = ParseBlock();
  if (!tokens.empty() && Peek().type == TType::Else) {
    auto elseStmnt = ParseElse();
    return If::WithElse(info,  std::move(condition), std::move(block),
                        std::move(elseStmnt));
  }
  return If::NoElse(info,  std::move(condition), std::move(block));
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
    return make_unique<For>(info,  nullptr, nullptr, nullptr, ParseBlock(),
                            ASTNode::context.PopScope());
  }
  // for i=0,i<?,i=i+1 {}
  if (Peek().type == TType::Identifier) {
    auto idTok = Peek();
    auto op = ParseExpression();
    auto iden = dynamic_cast<Identifier *>(op.get());
    // Range based for loop for iden : array/obj {}
    if (iden && Peek().type == TType::Colon) {
      Eat();
      auto rhs = ParseExpression();
      return make_unique<RangeBasedFor>(info,  make_unique<Identifier>(info,  iden->name),
                                        std::move(rhs), ParseBlock());
    }

    // for i=0,i<..,i++
    if (Peek().type == TType::Assign) {
      decl = ParseAssignment(make_unique<Identifier>(info ,iden->name));
      Expect(TType::Comma);
      condition = ParseExpression();
      Expect(TType::Comma);
      inc = ParseStatement();
      return make_unique<For>(info,  std::move(decl), std::move(condition),
                              std::move(inc), ParseBlock(),
                              ASTNode::context.PopScope());
    } else {
      tokens.push_back(idTok);
    }
  }

  // for CONDITION {}
  condition = ParseExpression();
  return make_unique<For>(info,  std::move(decl), std::move(condition), std::move(inc),
                          ParseBlock(), ASTNode::context.PopScope());
}
StatementPtr Parser::ParseContinue() { return make_unique<Continue>(info); }
StatementPtr Parser::ParseReturn() {
  return make_unique<Return>(info,  ParseExpression());
}
StatementPtr Parser::ParseBreak() { return make_unique<Break>(info); }
Token Parser::Peek() { return tokens.back(); }
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
    auto statement = ParseStatement();
    statements.push_back(std::move(statement));
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
    return make_unique<Using>(info,  iden.value, true);
  
  }
  // plain 'using raylib' statement
  else if (next.type == TType::Identifier) {
    auto iden = Expect(TType::Identifier);
    return make_unique<Using>(info,  iden.value, false);
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
    
    return make_unique<Using>(info,  iden.value, names);
  }
  throw std::runtime_error("Failed to parse using statement");
}
