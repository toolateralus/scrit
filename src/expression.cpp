#include "error.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "lexer.hpp"
#include "value.hpp"
#include <stdexcept>
#include "type.hpp"
#include <memory>
#include <stdexcept>
#include <vector>



ExpressionPtr Parser::ParseExpression() {
  if (tokens.empty()) {
    throw std::runtime_error("Unexpected end of input");
  }
  auto next = Peek();
  // for ++i, --i etc.  
  if (next.type == TType::Increment || next.type == TType::Decrement) {
    Eat();
    auto expr = ParseExpression();
    auto unary = make_unique<UnaryExpr>(info, expr->type, std::move(expr), next.type);
    return unary;
  }
  auto expr = ParseCompoundAssignment();
  return expr;
}
ExpressionPtr Parser::ParseCompoundAssignment() {
  auto left = ParseLogicalOr();

  if (!tokens.empty()) {
    auto next = Peek();
    if (!IsCompoundAssignmentOperator(next.type)) {
      return left;
    }
    // TODO: we need to make sure the LHS is not a literal.
    if (dynamic_cast<Literal *>(left.get())) {
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
    return make_unique<Literal>(info, TypeSystem::Current().String, String_T::New(std::move(token.value)));
  case TType::True:
    Eat();
    return make_unique<Literal>(info, TypeSystem::Current().Bool, Value_T::True);
  case TType::False:
    Eat();
    return make_unique<Literal>(info, TypeSystem::Current().Bool, Value_T::False);
  case TType::Undefined:
    Eat();
    return make_unique<Literal>(info, TypeSystem::Current().Undefined, Value_T::UNDEFINED);
  case TType::Null:
    Eat();
    return make_unique<Literal>(info, TypeSystem::Current().Null, Value_T::VNULL);
  case TType::Float:
    Eat();
    return make_unique<Literal>(info, TypeSystem::Current().Float, Float_T::New(stof(token.value)));
  case TType::Int:
    Eat();
    return make_unique<Literal>(info, TypeSystem::Current().Int, Int_T::New(stoi(token.value)));
  case TType::Identifier: {
    Eat();
    return make_unique<Identifier>(info, token.value);
  }
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
  auto returnType = ParseReturnType();
  auto body = ParseBlock();
  auto types = params->ParamTypes();
  auto callable = make_shared<Callable_T>(returnType, std::move(body), std::move(params));
  auto type = TypeSystem::Current().FromCallable(returnType, types);
  
  if (!type) {
    throw std::runtime_error("unable to get type for anonymous function");
  }
  return make_unique<AnonymousFunction>(info, type, callable);
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
    
    // ignore let tokens.
    case TType::Let:
      Eat();
    case TType::Mut:
    case TType::Const:
    case TType::Identifier: { 
      statements.push_back(ParseDeclaration());
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
  auto type = TypeSystem::Current().Get("object");
  return make_unique<ObjectInitializer>(info, type, make_unique<Block>(info, std::move(statements)));
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
    
    if (!t) {
      throw std::runtime_error("Lambda propertys must return a value.");
    }
    
    return make_unique<Lambda>(info, t, std::move(block));
  }
  // here we use lambda as basically an implicit return.
  // let .. => some_expression
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
    auto type = TypeSystem::Current().Get("array");
    return make_unique<Operand>(info, type, make_unique<ArrayInitializer>(info, type, std::vector<ExpressionPtr>()));
  } else {
    vector<ExpressionPtr> init_expressions = {};
    
    Type inner_type;
    while (Peek().type != TType::SubscriptRight) {
      auto val = ParseExpression();
      
      if (!inner_type) {
        inner_type = val->type;
      }
      
      if (inner_type && !Type_T::Equals(val->type.get(), inner_type.get())) {
        throw std::runtime_error("invalid type in array initializer\nexpected: " + inner_type->name + "\ngot: " + val->type->name);
      }
      
      init_expressions.push_back(std::move(val));
      
      if (Peek().type == TType::Comma) {
        Eat();
      }
    }
    Expect(TType::SubscriptRight);
    auto type = TypeSystem::Current().GetOrCreateTemplate("array<" + inner_type->name + ">", TypeSystem::Current().Get("array"), {inner_type});
    return make_unique<Operand>(info, type, make_unique<ArrayInitializer>(info, type, std::move(init_expressions)));
  }
}