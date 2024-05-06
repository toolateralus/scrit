#include "parser.hpp"
#include "lexer.hpp"
#include "value.hpp"
#include <memory>
#include <stdexcept>

unique_ptr<Statement> Parser::ParseLValuePostFix(unique_ptr<Expression> &&expr) {
  if (DotExpr* dot = dynamic_cast<DotExpr*>(expr.get())) {
    auto next = Peek().type;
    // TODO: allow for compound assignment on LValue postfix
    // obj.value += 01
    // obj.array[]++ 
    // etc
    if (next == TType::Assign) {
      Eat();
      auto value = ParseExpression();
      return make_unique<DotAssignment>(std::move(expr), std::move(value));
    }
    else if (Call* callable = dynamic_cast<Call*>(dot->right.get())) {
      return make_unique<DotCallStmnt>(std::move(expr));
    }
  }
  else if (Subscript* subscript = dynamic_cast<Subscript*>(expr.get())) {
    if (Peek().type == TType::Assign) {
      Eat();
      auto value = ParseExpression();
      return make_unique<SubscriptAssignStmnt>(std::move(expr), std::move(value));
    }
  }
  else if (auto call = dynamic_cast<Call*>(expr.get())) {
    return make_unique<Call>(std::move(call->operand), std::move(call->args));
  }
  
  throw std::runtime_error("Failed to parse LValue postfix statement");
}

unique_ptr<Statement> Parser::ParseStatement() {
  while (tokens.size() > 0) {
    auto token = Peek();
    switch (token.family) {
      case TFamily::Identifier: {
        auto operand = ParsePostfix();
        if (auto id = dynamic_cast<Identifier *>(operand.get())) {
          return ParseIdentifierStatement(make_unique<Identifier>(id->name));
        } else  {
          return ParseLValuePostFix(std::move(operand));
        }
        // } else if (auto nativeCallable = dynamic_cast<NativeCallableExpr *>(operand.get())) {
        //   return NativeCallableStatement::FromExpr(std::move(nativeCallable));
        // } else if (auto compoundAssign = dynamic_cast<CompoundAssignExpr *>(operand.get())) {
        //   return make_unique<CompoundAssignStmnt>(std::move(compoundAssign));
        // }
        throw std::runtime_error("Failed to start identifier statement");
      }
      case TFamily::Keyword: {
        Eat(); // consume kw
        // for func(..) {...}() anonymous func declaration & invocation statements.
        if (token.type == TType::Func && Peek().type == TType::LParen) {
          auto parameters = ParseParameters();
          auto body = ParseBlock();
          auto arguments = ParseArguments();
          auto op = make_unique<Operand>(make_shared<Callable_T>(std::move(body), std::move(parameters)));
          return make_unique<Call>(std::move(op), std::move(arguments));
        }
        return ParseKeyword(token);
      }
      default: throw std::runtime_error("Failed to parse statement");
    }
  }
  throw std::runtime_error("Unexpecrted end of input");
}
unique_ptr<Statement> Parser::ParseKeyword(Token token) {
  
   switch (token.type) {
      case TType::Func: {
          return ParseFuncDecl();
        }
      case TType::If: {
          return ParseIf();
        }
      case TType::Else: {
          throw new std::runtime_error("An else must be preceeded by an if statement.");
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
      // case TType::Import: {
      //     Expect(TType::String);
      //     return new NoopStatement();
      //   }
      case TType::Break: {
          return ParseBreak();
        }
      
      default: throw std::runtime_error("Failed to parse keyword" + TTypeToString(token.type));
    }
  
}
unique_ptr<Statement> Parser::ParseIdentifierStatement(unique_ptr<Identifier> identifier) {
  switch (Peek().type) {
  case TType::Assign: {
    return ParseAssignment(std::move(identifier));
  }
  case TType::LParen: {
    return ParseCall(std::move(identifier));
  }
  default:
    throw std::runtime_error("failed to parse identifier statement");
  }
}
unique_ptr<Statement> Parser::ParseAssignment(unique_ptr<Identifier> identifier) {
  Expect(TType::Assign);
  auto value = ParseExpression();
  return make_unique<Assignment>(std::move(identifier), std::move(value));
}
unique_ptr<Statement> Parser::ParseCall(unique_ptr<Identifier> identifier) {
  auto args = ParseArguments();
  return make_unique<Call>(std::move(identifier), std::move(args));
}
unique_ptr<Expression> Parser::ParseExpression() {
  if (tokens.empty()) {
    throw std::runtime_error("Unexpected end of input");
  }
  return ParseLogicalOr();
}

unique_ptr<Expression> Parser::ParseLogicalOr() {
  auto left = ParseLogicalAnd();

  if (!tokens.empty() && Peek().type == TType::Or) {
    Eat();
    auto right = ParseLogicalAnd();
    return std::make_unique<BinExpr>(std::move(left), std::move(right), TType::Or);
  }

  return left;
}

unique_ptr<Expression> Parser::ParseLogicalAnd() {
  auto left = ParseEquality();

  if (!tokens.empty() && Peek().type == TType::And) {
    Eat();
    auto right = ParseEquality();
    return std::make_unique<BinExpr>(std::move(left), std::move(right), TType::And);
  }

  return left;
}

unique_ptr<Expression> Parser::ParseEquality() {
  auto left = ParseComparison();

  while (!tokens.empty() && (Peek().type == TType::Equals || Peek().type == TType::NotEquals)) {
    auto op = Peek().type;
    Eat();
    auto right = ParseComparison();
    left = std::make_unique<BinExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

unique_ptr<Expression> Parser::ParseComparison() {
  auto left = ParseTerm();

  while (!tokens.empty() && (Peek().type == TType::Less || Peek().type == TType::LessEQ ||
                             Peek().type == TType::Greater || Peek().type == TType::GreaterEQ)) {
    auto op = Peek().type;
    Eat();
    auto right = ParseTerm();
    left = std::make_unique<BinExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

unique_ptr<Expression> Parser::ParseTerm() {
  auto left = ParseFactor();

  while (!tokens.empty() && (Peek().type == TType::Add || Peek().type == TType::Sub)) {
    auto op = Peek().type;
    Eat();
    auto right = ParseFactor();
    left = std::make_unique<BinExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

unique_ptr<Expression> Parser::ParseFactor() {
  auto left = ParsePostfix();

  while (!tokens.empty() && (Peek().type == TType::Mul || Peek().type == TType::Div)) {
    auto op = Peek().type;
    Eat();
    auto right = ParsePostfix();
    left = std::make_unique<BinExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

unique_ptr<Expression> Parser::ParsePostfix() {
  auto expr = ParseOperand();
  while (!tokens.empty()) {
    Token next = Peek();
    if (next.type != TType::LParen && next.type != TType::SubscriptLeft) {
      break;
    }
    if (next.type == TType::LParen) {
      auto args = ParseArguments();
      expr = std::make_unique<Call>(std::move(expr), std::move(args));
    } else if (next.type == TType::SubscriptLeft) {
      Eat();
      auto index = ParseExpression();
      Expect(TType::SubscriptRight);
      expr = std::make_unique<Subscript>(std::move(expr), std::move(index));
    }
  }
  
  return expr;
}
unique_ptr<Expression> Parser::ParseOperand() {
  auto token = Peek();
  if (token.type == TType::Sub || token.type == TType::Not) {
    Eat();
    auto operand = ParseOperand();
    return make_unique<UnaryExpr>(std::move(operand), token.type);
  }

  switch (token.type) {
  case TType::SubscriptLeft: {
    return ParseArrayInitializer();
  }
  case TType::Func: {
    Eat();
    auto params = ParseParameters();
    auto body = ParseBlock();
    auto callable = make_shared<Callable_T>(std::move(body), std::move(params));
    return make_unique<Operand>(callable);
  }
  case TType::LCurly: {
    Eat();
    vector<unique_ptr<Statement>> statements = {};
    while (tokens.size() > 0) {
      auto next = Peek();
      if (next.type == TType::RCurly) {
        break;
      }
      auto statement = ParseStatement();
      statements.push_back(std::move(statement));
    }
    Expect(TType::RCurly);
    auto block = make_unique<Block>(std::move(statements));
    return make_unique<ObjectInitializer>(std::move(block));
  }
  case TType::String:
    Eat();
    return make_unique<Operand>(make_shared<String_T>(std::move(token.value)));
  case TType::True:
    Eat();
    return make_unique<Operand>(Value_T::True);
  case TType::False:
    Eat();
    return make_unique<Operand>(Value_T::False);
  case TType::Undefined:
    Eat();
    return make_unique<Operand>(Value_T::Undefined);
  case TType::Null:
    Eat();
    return make_unique<Operand>(Value_T::Null);
  case TType::Float:
    Eat();
    return make_unique<Operand>(make_shared<Int_T>(stoi(token.value)));
  case TType::Int:
    Eat();
    return make_unique<Operand>(make_shared<Float_T>(stof(token.value)));
  case TType::Identifier:
    Eat();
    return make_unique<Identifier>(token.value);
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
unique_ptr<Operand> Parser::ParseArrayInitializer() {
  Eat();
  if (Peek().type == TType::SubscriptRight) {
    Eat();
    auto array = Array_T::New();
    return make_unique<Operand>(array);
  } else {
    vector<unique_ptr<Expression>> values = {};
    while (Peek().type != TType::SubscriptRight) {
      auto val = ParseExpression();
      values.push_back(std::move(val));
      if (Peek().type == TType::Comma) {
        Eat();
      }
    }
    Expect(TType::SubscriptRight);
    auto array = Array_T::New(std::move(values));
    return make_unique<Operand>(array);
  }
}
unique_ptr<Parameters> Parser::ParseParameters() {
  Expect(TType::LParen);
  auto next = Peek();
  vector<string> values = {};
  while (tokens.size() > 0 && next.type != TType::RParen) {
    auto value = ParseOperand();
    if (auto iden = dynamic_cast<Identifier *>(value.get())) {
      values.push_back(std::move(iden->name));
    }
    if (Peek().type == TType::Comma) {
      Eat();
    }
  }
  Expect(TType::RParen);
  return make_unique<Parameters>(std::move(values));
}
unique_ptr<Arguments> Parser::ParseArguments() {
  Expect(TType::LParen);
  auto next = Peek();
  vector<unique_ptr<Expression>> values = {};
  
  if (next.type == TType::RParen) {
    Eat();
    return make_unique<Arguments>(std::move(values));
  }
  
  while (tokens.size() > 0 && next.type != TType::RParen) {
    auto value = ParseExpression();
    values.push_back(std::move(value));
    if (Peek().type == TType::Comma) {
      Eat();
    }
  }
  Expect(TType::RParen);
  return make_unique<Arguments>(std::move(values));
}
unique_ptr<Block> Parser::ParseBlock() {
  Expect(TType::LCurly);
  vector<unique_ptr<Statement>> statements = {};
  auto next = Peek();
  
  if (next.type == TType::RCurly) {
    Eat();
    return make_unique<Block>(std::move(statements));
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
  return make_unique<Block>(std::move(statements));
}

unique_ptr<Statement> Parser::ParseFuncDecl() {
    auto name = Expect(TType::Identifier);
    auto parameters = ParseParameters();
    auto body = ParseBlock();
    auto id = make_unique<Identifier>(name.value);
    auto func = make_unique<FuncDecl>(std::move(id), std::move(body) ,std::move(parameters));
    return func;
  
}
unique_ptr<Else> Parser::ParseElse() { 
   Eat();
    if (Peek().type == TType::If) {
      Eat();
      auto ifstmnt = ParseIf();
      return Else::New(std::move(ifstmnt));
    }
    else {
      return Else::NoIf(ParseBlock());
    }
  
}
unique_ptr<If> Parser::ParseIf() {
    auto condition = ParseExpression();
    auto block = ParseBlock();
    if (Peek().type == TType::Else) {
      auto elseStmnt = ParseElse();
      return If::WithElse(std::move(condition), std::move(block), std::move(elseStmnt));
    }
    return If::NoElse(std::move(condition), std::move(block));
}

unique_ptr<Statement> Parser::ParseFor() {
  auto scope = ASTNode::context.PushScope();
  if (Peek().type == TType::LParen) {
  Eat();
  }
  unique_ptr<Statement> decl = nullptr;
  unique_ptr<Expression> condition = nullptr;
  unique_ptr<Statement> inc = nullptr;
  
  if (Peek().type == TType::LCurly) {
  return make_unique<For>(nullptr, nullptr, nullptr, ParseBlock(), ASTNode::context.PopScope());
  }
  
  if (Peek().type == TType::Identifier) {
  auto idTok = Peek();
  auto op = ParseOperand();
  
  auto iden = dynamic_cast<Identifier*>(op.get());
  
  if (Peek().type == TType::Assign) {
    decl = ParseAssignment(make_unique<Identifier>(iden->name));
  }
  else {
    tokens.push_back(idTok);
    condition = ParseExpression();
  }
  }
  
  if (Peek().type == TType::Comma) {
  Eat();
  condition = ParseExpression();
  }
  
  if (Peek().type == TType::Comma) {
  Eat();
  inc = ParseStatement();
  }
  
  return make_unique<For>(std::move(decl), std::move(condition), std::move(inc), ParseBlock(), ASTNode::context.PopScope());
}
unique_ptr<Statement> Parser::ParseContinue() {
  return make_unique<Continue>();
}
unique_ptr<Statement> Parser::ParseReturn() {
  return make_unique<Return>(ParseExpression());
}
unique_ptr<Statement> Parser::ParseBreak() {
  return make_unique<Break>();
}
