#include "gtest/gtest.h"
#include "lexer.hpp"

TEST(LexerTest, LexStringTest) {
    Lexer lexer;
    std::vector<Token> tokens = lexer.Lex("\"Hello, World!\"");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].ToString(), "Token(Hello, World!) type::String family::Literal\n");
}

TEST(LexerTest, LexNumTest) {
    Lexer lexer;
    std::vector<Token> tokens = lexer.Lex("42 3.14");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_EQ(tokens[0].ToString(), "Token(42) type::Int family::Literal\n");
    ASSERT_EQ(tokens[1].ToString(), "Token(3.14) type::Float family::Literal\n");
}

TEST(LexerTest, LexIdenTest) {
    Lexer lexer;
    std::vector<Token> tokens = lexer.Lex("foo _bar Baz123");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_EQ(tokens[0].type, TType::Identifier);
    ASSERT_EQ(tokens[1].type, TType::Identifier);
    ASSERT_EQ(tokens[2].type, TType::Identifier);

    ASSERT_EQ(tokens[0].value, "foo");
    ASSERT_EQ(tokens[1].value, "_bar");
    ASSERT_EQ(tokens[2].value, "Baz123");
}

TEST(LexerTest, LexOpTest) {
    Lexer lexer;
    std::vector<Token> tokens = lexer.Lex("+ - * / || && > < >= <= == != . ! ( ) { } [ ] , =");
    ASSERT_EQ(tokens.size(), 22);
    for (auto &token : tokens) {
        ASSERT_EQ(token.family, TFamily::Operator);
    }
    ASSERT_EQ(tokens[0].type, TType::Add);
    ASSERT_EQ(tokens[1].type, TType::Sub);
    ASSERT_EQ(tokens[2].type, TType::Mul);
    ASSERT_EQ(tokens[3].type, TType::Div);
    ASSERT_EQ(tokens[4].type, TType::Or);
    ASSERT_EQ(tokens[5].type, TType::And);
    ASSERT_EQ(tokens[6].type, TType::Greater);
    ASSERT_EQ(tokens[7].type, TType::Less);
    ASSERT_EQ(tokens[8].type, TType::GreaterEq);
    ASSERT_EQ(tokens[9].type, TType::LessEq);
    ASSERT_EQ(tokens[10].type, TType::Equals);
    ASSERT_EQ(tokens[11].type, TType::NotEquals);
    ASSERT_EQ(tokens[12].type, TType::Dot);
    ASSERT_EQ(tokens[13].type, TType::Not);
    ASSERT_EQ(tokens[14].type, TType::LParen);
    ASSERT_EQ(tokens[15].type, TType::RParen);
    ASSERT_EQ(tokens[16].type, TType::LCurly);
    ASSERT_EQ(tokens[17].type, TType::RCurly);
    ASSERT_EQ(tokens[18].type, TType::SubscriptLeft);
    ASSERT_EQ(tokens[19].type, TType::SubscriptRight);
    ASSERT_EQ(tokens[20].type, TType::Comma);
    ASSERT_EQ(tokens[21].type, TType::Assign);

    ASSERT_EQ(tokens[0].value, "+");
    ASSERT_EQ(tokens[1].value, "-");
    ASSERT_EQ(tokens[2].value, "*");
    ASSERT_EQ(tokens[3].value, "/");
    ASSERT_EQ(tokens[4].value, "||");
    ASSERT_EQ(tokens[5].value, "&&");
    ASSERT_EQ(tokens[6].value, ">");
    ASSERT_EQ(tokens[7].value, "<");
    ASSERT_EQ(tokens[8].value, ">=");
    ASSERT_EQ(tokens[9].value, "<=");
    ASSERT_EQ(tokens[10].value, "==");
    ASSERT_EQ(tokens[11].value, "!=");
    ASSERT_EQ(tokens[12].value, ".");
    ASSERT_EQ(tokens[13].value, "!");
    ASSERT_EQ(tokens[14].value, "(");
    ASSERT_EQ(tokens[15].value, ")");
    ASSERT_EQ(tokens[16].value, "{");
    ASSERT_EQ(tokens[17].value, "}");
    ASSERT_EQ(tokens[18].value, "[");
    ASSERT_EQ(tokens[19].value, "]");
    ASSERT_EQ(tokens[20].value, ",");
    ASSERT_EQ(tokens[21].value, "=");
}
TEST(LexerTest, LexKeywordTest) {
    Lexer lexer;
    std::vector<Token> tokens = lexer.Lex("func for continue break return if else false true null undefined import from");
    ASSERT_EQ(tokens.size(), 13);
    for (auto &token : tokens) {
        ASSERT_EQ(token.family, TFamily::Keyword);
    }
    ASSERT_EQ(tokens[0].type, TType::Func);
    ASSERT_EQ(tokens[1].type, TType::For);
    ASSERT_EQ(tokens[2].type, TType::Continue);
    ASSERT_EQ(tokens[3].type, TType::Break);
    ASSERT_EQ(tokens[4].type, TType::Return);
    ASSERT_EQ(tokens[5].type, TType::If);
    ASSERT_EQ(tokens[6].type, TType::Else);
    ASSERT_EQ(tokens[7].type, TType::False);
    ASSERT_EQ(tokens[8].type, TType::True);
    ASSERT_EQ(tokens[9].type, TType::Null);
    ASSERT_EQ(tokens[10].type, TType::Undefined);
    ASSERT_EQ(tokens[11].type, TType::Import);
    ASSERT_EQ(tokens[12].type, TType::From);

    ASSERT_EQ(tokens[0].value, "func");
    ASSERT_EQ(tokens[1].value, "for");
    ASSERT_EQ(tokens[2].value, "continue");
    ASSERT_EQ(tokens[3].value, "break");
    ASSERT_EQ(tokens[4].value, "return");
    ASSERT_EQ(tokens[5].value, "if");
    ASSERT_EQ(tokens[6].value, "else");
    ASSERT_EQ(tokens[7].value, "false");
    ASSERT_EQ(tokens[8].value, "true");
    ASSERT_EQ(tokens[9].value, "null");
    ASSERT_EQ(tokens[10].value, "undefined");
    ASSERT_EQ(tokens[11].value, "import");
    ASSERT_EQ(tokens[12].value, "from");
}
TEST(LexerTest, LexErrorTest) {
    Lexer lexer;
    EXPECT_THROW(lexer.Lex("$"), std::runtime_error);
    EXPECT_THROW(lexer.Lex("\"unclosed string"), std::runtime_error);
}