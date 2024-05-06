#include "parser.hpp"
#include "lexer.hpp"
#include "value.hpp"
#include <stdexcept>

unique_ptr<Statement>  Parser::ParseDeclarationOrAssignment() {
  
}
unique_ptr<Statement>  Parser::ParseStatement() {
  
}
unique_ptr<Statement>  Parser::ParseKeyword() {
  
}
unique_ptr<Statement>  Parser::ParseIdentifierStatement() {
  
}
unique_ptr<Expression>  Parser::ParseExpression() {
  
}
unique_ptr<Expression> Parser::ParseLogicalOr() {
  
}
unique_ptr<Expression> Parser::ParseLogicalAnd() {
  
}
unique_ptr<Expression> Parser::ParseEquality() {
  
}
unique_ptr<Expression> Parser::ParseComparison() {
  
}
unique_ptr<Expression> Parser::ParseTerm() {
  
}
unique_ptr<Expression> Parser::ParseFactor() {
  
}
unique_ptr<Expression> Parser::ParsePostfix() {
  
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
    }
    else {
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
  unique_ptr<Parameters> Parser::ParseParameters() {}
  unique_ptr<Arguments> Parser::ParseArguments() {}
  unique_ptr<Block> Parser::ParseBlock() {}
