#include "parser.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "type.hpp"
#include "value.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

Token Parser::Peek(size_t lookahead) {
  if (lookahead >= tokens.size()) {
    throw std::out_of_range("Lookahead is out of range");
  }
  auto &tkn = tokens[tokens.size() - 1 - lookahead];
  return tkn;
}
Token Parser::Eat() {
  auto tkn = tokens.back();
  info = tkn.info;
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
  // insert a global namespace.
  ASTNode::context.root_namespace = make_shared<Namespace>("global", nullptr);
  ASTNode::context.current_namespace = ASTNode::context.root_namespace;
  
  std::reverse(tokens.begin(), tokens.end());
  this->tokens = std::move(tokens);
  vector<StatementPtr> statements;
  while (this->tokens.size() > 0) {
    try {
      auto statement = ParseStatement();
      statements.push_back(std::move(statement));
    } catch (std::out_of_range oor) {
      std::cout << "unexpected end of input at \n"
                << info.ToString() << std::endl;
      std::exit(1);
    } catch (std::runtime_error e) {

      auto what = string(e.what());

      // add source info if it doesn't already exist.
      // This is pretty presumptuous and bad.
      if (!what.contains("{")) {
        what += " " + info.ToString();
      }

      std::cout << "\033[1;31mParser exception! : " << what << "\033[0m"
                << std::endl;
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

StatementPtr Parser::ParseStatement() {
  while (tokens.size() > 0) {
    auto token = Peek();

    info = token.info;

    if (token.type == TType::Increment || token.type == TType::Decrement) {
      auto inc_expr = ParseExpression();
      return make_unique<UnaryStatement>(info, std::move(inc_expr));
    }

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

      throw std::runtime_error("Failed to parse statement.\n");
    }
  }
  throw std::runtime_error("Unexpected end of input");
}

StatementPtr Parser::ParseKeyword(Token token) {
  switch (token.type) {
  case TType::Struct: {
    auto name = Expect(TType::Identifier).value;
    
    vector<string> template_args;
    if (Peek().type == TType::Less) {
      Eat();
      while (!tokens.empty()) {
        if (Peek().type == TType::Greater) {
          break;
        }
        template_args.push_back(Expect(TType::Identifier).value);
        if (Peek().type == TType::Comma) {
          Eat();
        }
      }
      Expect(TType::Greater);
    }    
    
    ASTNode::context.ImmediateScope()->InsertType(
        name, make_shared<StructType>(name, nullptr, template_args));
        
    auto ctor = ParseObjectInitializer();
    
    return make_unique<StructDeclaration>(info, name, std::move(ctor), template_args);
  }
  
  case TType::Namespace: {
    auto path = ParseScopeResolution();
    ASTNode::context.CreateNamespace(path->identifiers);
    ASTNode::context.SetCurrentNamespace(path->identifiers);
    return make_unique<Noop>(info);
  }
  case TType::Type: {
    auto name = Expect(TType::Identifier).value;
    Expect(TType::Assign);
    auto type = ParseType();
    return make_unique<TypeAlias>(info, name, type);
  }

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
    return make_unique<Noop>(info);
  }
  case TType::If: {
    return ParseIf();
  }
  case TType::Else: {
    throw std::runtime_error("An else must be preceeded by an if statement.");
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

// TODO: we need to refactor usings entirely for the way the type system mingles
// with types. usings should be equivalent to a CSharp namespace.
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
  // also parses 'using std.array'
  else if (next.type == TType::Identifier) {
    string name;
    auto iden = Expect(TType::Identifier);
    name = iden.value;
    if (Peek().type == TType::Dot) {
      name += Eat().value;
      while (!tokens.empty()) {
        name += Eat().value;
        if (Peek().type != TType::Dot) {
          break;
        }
      }
    }
    return make_unique<Using>(info, name, false);
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

StatementPtr Parser::ParseDeclaration(SourceInfo &info, const string &iden,
                                      const Mutability &mut) {
  Type type = nullptr;
  auto next = Peek();
  switch (next.type) {

  case TType::Colon: {
    Eat();
    type = ParseType();
    if (tokens.empty() || Peek().type != TType::Assign) {
      auto op = make_unique<Operand>(info, type,
                                     make_unique<DefaultValue>(info, type));
      return std::make_unique<Declaration>(info, iden, std::move(op), mut,
                                           type);
    }
    Eat();
    auto expr = ParseExpression();
    return std::make_unique<Declaration>(info, iden, std::move(expr), mut,
                                         type);
  }
  case TType::Lambda: {
    auto lambda = ParseLambda();
    if (type && type != lambda->type) {
      throw std::runtime_error(
          "explicit type: " + type->name +
          " did not match the property's type: " + lambda->type->name);
    }
    return make_unique<Property>(info, iden, std::move(lambda), mut);
  }
  case TType::Assign: {
    Eat();
    auto expr = ParseExpression();
    auto type = expr->type;
    return make_unique<Declaration>(info, iden, std::move(expr), mut, type);
  }
  default:
    throw std::runtime_error("failed to parse declaration, got token: " +
                             TTypeToString(next.type));
  }
}
StatementPtr Parser::ParseDeclaration() {
  auto next = Peek();

  switch (next.type) {
  case TType::Mut: {
    Expect(TType::Mut);
    auto next = Expect(TType::Identifier);
    return ParseDeclaration(info, next.value, Mutability::Mut);
  }
  case TType::Const: {
    Expect(TType::Const);
    auto next = Expect(TType::Identifier);
    return ParseDeclaration(info, next.value, Mutability::Const);
  }
  case TType::Identifier: {
    auto next = Expect(TType::Identifier);
    return ParseDeclaration(info, next.value, Mutability::Const);
  }

  default:
    break;
  }
  throw std::runtime_error("Unable to parse declaration.. got :" + next.value);
}
StatementPtr Parser::ParseAssignment(IdentifierPtr identifier) {
  Token next = Peek();

  Type type = nullptr;

  if (next.type == TType::Assign) {
    Eat();
    auto value = ParseExpression();
    
    if (type) {
      if (type->Equals(value->type.get())) {
        value->type = type;
      } else {
        throw TypeError(type, value->type, "invalid types in assignment");
      }
    }
    
    return make_unique<Assignment>(info, value->type, std::move(identifier),
                                   std::move(value));

  } else if (IsCompoundAssignmentOperator(next.type)) {
    Eat();
    auto value = ParseExpression();

    if (type && type != value->type) {
      throw std::runtime_error(
          "explicit type: " + type->name +
          " did not match the expressions type: " + value->type->name);
    }

    return make_unique<CompoundAssignment>(
        info, make_unique<CompAssignExpr>(info, std::move(identifier),
                                          std::move(value), next.type));
  } else {
    throw std::runtime_error("failed to parse assignment: invalid operator." +
                             TTypeToString(next.type));
  }
}

unique_ptr<Noop> Parser::ParseFunctionDeclaration() {
  auto info = this->info;
  auto name = Expect(TType::Identifier).value;
  auto parameters = ParseParameters();
  auto returnType = ParseReturnType();
  
  auto param_clone = parameters->Clone();
  
  auto callable =
      make_shared<Callable_T>(returnType, nullptr, std::move(parameters));
  ASTNode::context.ImmediateScope()->Set(name, callable, Mutability::Mut);
  
  auto block = ParseBlock(param_clone);
  
  callable->block = std::move(block);

  return std::make_unique<Noop>(info);
}
// for statements like
// obj.func()
// arr[0] = ...
// i++
// i += 10
// which are truly (Rvalue) expressions but occur as an LValue
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

    if (dynamic_cast<MethodCall *>(dotRight->right.get())) {
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

  if (auto call = dynamic_cast<MethodCall *>(expr.get())) {
    return make_unique<DotCallStmnt>(info, std::move(expr));
  }

  if (dynamic_cast<CompAssignExpr *>(expr.get())) {
    return make_unique<CompoundAssignment>(info, std::move(expr));
  }

  auto &raw = *expr.get();
  auto name = string(typeid(raw).name());
  throw std::runtime_error("Failed to parse LValue postfix statement:: " +
                           name);
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
// TODO: make this a more formal declaration,
// where we can do things like
// let f, v = ...
// let mut f, mut v = ...
StatementPtr Parser::ParseTupleDeconstruction(IdentifierPtr &&iden) {
  vector<string> idens;
  idens.push_back(std::move(iden->name));
  Expect(TType::Comma);

  while (!tokens.empty()) {
    if (Peek().type == TType::Assign) {
      break;
    }

    auto iden = Expect(TType::Identifier);
    idens.push_back(iden.value);

    if (Peek().type == TType::Comma) {
      Eat();
    }
  }

  Expect(TType::Assign);

  auto tuple = ParseExpression();

  return make_unique<TupleDeconstruction>(info, std::move(idens),
                                          std::move(tuple));
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
  auto returnType = ParseReturnType();
  auto body = ParseBlock(parameters);
  auto arguments = ParseArguments();
  auto type = Values::TypeSystem::Current().FromCallable(
      returnType, parameters->ParamTypes());
  auto callable = make_shared<Callable_T>(returnType, std::move(body),
                                          std::move(parameters));
  auto op = make_unique<AnonymousFunction>(info, type, callable);
  return make_unique<Call>(info, std::move(op), std::move(arguments));
}
// Call statement.
unique_ptr<Call> Parser::ParseCall(IdentifierPtr identifier) {
  auto info = this->info;
  auto args = ParseArguments();
  return make_unique<Call>(info, std::move(identifier), std::move(args));
}
ParametersPtr Parser::ParseParameters() {
  Expect(TType::LParen);
  auto next = Peek();
  std::vector<Parameters::Param> params = {};
  while (tokens.size() > 0 && next.type != TType::RParen) {
    
    Mutability mut = Mutability::Const;
    if (Peek().type == TType::Const || Peek().type == TType::Mut) {
      mut = Eat().type == TType::Mut ? Mutability::Mut : Mutability::Const;
    }
    
    auto value = Expect(TType::Identifier).value;
    Expect(TType::Colon);
    auto type = ParseType();
    
    Parameters::Param param = {
        .name = value, 
        .default_value = nullptr,
        .type = type, 
        .mutability = mut
    };

    // Default values for parameter.
    if (Peek().type == TType::Assign) {
      Eat();
      auto value = ParseExpression();
      param.default_value = value->Evaluate();
      param.type = value->type;
    }

    params.push_back(param);

    if (Peek().type == TType::Comma) {
      Eat();
    }
    next = Peek();
  }
  Expect(TType::RParen);
  return make_unique<Parameters>(info, std::move(params));
}

// TODO: add support for named parameters in arguments.
// some_func(iden: 0, is_something: false)
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

BlockPtr Parser::ParseBlock(ParametersPtr &params) {
  
  auto scope = ASTNode::context.PushScope();
  
  params->ForwardDeclare(scope);
  
  auto info = this->info;
  Expect(TType::LCurly);
  vector<StatementPtr> statements = {};
  auto next = Peek();

  // Empty block.
  if (next.type == TType::RCurly) {
    Eat();
    ASTNode::context.PopScope();
    return make_unique<Block>(info, std::move(statements), scope);
  }

  while (tokens.size() > 0) {

    // If the last line of a block is an identifier or a literal, we just create
    // an implicit return. I am not sure that it's possible, but I would love to
    // have a rust-like clone of their implicit returns. that is, any expression
    // can be the last 'statement' of a block and have an implicit return. like
    // a match or complex expression etc.
    if (Peek(1).type == TType::RCurly &&
        (Peek().family == TFamily::Identifier ||
         Peek().family == TFamily::Literal || Peek().type == TType::Undefined ||
         Peek().type == TType::Null || Peek().type == TType::False ||
         Peek().type == TType::True)) {
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
  ASTNode::context.PopScope();
  return make_unique<Block>(info, std::move(statements), scope);
}

BlockPtr Parser::ParseBlock() {
  auto scope = ASTNode::context.PushScope();
  
  auto info = this->info;
  Expect(TType::LCurly);
  vector<StatementPtr> statements = {};
  auto next = Peek();

  // Empty block.
  if (next.type == TType::RCurly) {
    Eat();
    ASTNode::context.PopScope();
    return make_unique<Block>(info, std::move(statements), scope);
  }

  while (tokens.size() > 0) {

    // If the last line of a block is an identifier or a literal, we just create
    // an implicit return. I am not sure that it's possible, but I would love to
    // have a rust-like clone of their implicit returns. that is, any expression
    // can be the last 'statement' of a block and have an implicit return. like
    // a match or complex expression etc.
    if (Peek(1).type == TType::RCurly &&
        (Peek().family == TFamily::Identifier ||
         Peek().family == TFamily::Literal || Peek().type == TType::Undefined ||
         Peek().type == TType::Null || Peek().type == TType::False ||
         Peek().type == TType::True)) {
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
  ASTNode::context.PopScope();
  return make_unique<Block>(info, std::move(statements), scope);
}

// ########## Control flow ###############
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

  if (Peek().type == TType::Let) {
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

  if (Peek().type == TType::Comma) {
    Eat();
    auto name = dynamic_cast<Identifier *>(expr.get());

    if (!name) {
      throw std::runtime_error(
          "invalid tuple deconstruction in range based for loop");
    }

    vector<string> names = {name->name};
    while (!tokens.empty()) {
      names.push_back(Expect(TType::Identifier).value);
      if (Peek().type == TType::Colon) {
        break;
      }
      if (Peek().type == TType::Comma) {
        Eat();
      }
    }
    Expect(TType::Colon);

    auto target = ParseExpression();
    return make_unique<RangeBasedFor>(info, names, std::move(target),
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
  if (tokens.empty() || IsLiteralOrExpression(next)) {
    return make_unique<Return>(info);
  }
  return make_unique<Return>(info, ParseExpression());
}
StatementPtr Parser::ParseBreak() { return make_unique<Break>(info); }


// ####### END CONTROL FLOW #########
unique_ptr<ScopeResolution> Parser::ParseScopeResolution() {
  vector<string> identifiers;
  while (!tokens.empty()) {
    identifiers.push_back(Expect(TType::Identifier).value);
    if (!tokens.empty() && Peek().type == TType::ScopeResolution) {
      Eat();
    }
    if (!tokens.empty() && Peek().type != TType::Identifier) {
      break;
    }
  }
  return make_unique<ScopeResolution>(info, identifiers);
}
