#include "gtest/gtest.h"
#include "lexer.hpp"

TEST(LexerTest, SimpleLexTest) {
    Lexer lexer;
    std::string input = "if x > 5 { return true } else { return false }";
    std::vector<Token> tokens = lexer.Lex(input);

    // Assert the number of tokens
    ASSERT_EQ(tokens.size(), 13);

    // Assert the types of some specific tokens
    ASSERT_EQ(tokens[0].type, TType::If);
    ASSERT_EQ(tokens[2].type, TType::Identifier);
    ASSERT_EQ(tokens[3].type, TType::Greater);
    ASSERT_EQ(tokens[4].type, TType::Int);
    ASSERT_EQ(tokens[6].type, TType::LCurly);
    ASSERT_EQ(tokens[7].type, TType::Return);
    ASSERT_EQ(tokens[8].type, TType::True);
    ASSERT_EQ(tokens[10].type, TType::RCurly);
    ASSERT_EQ(tokens[11].type, TType::Else);
    ASSERT_EQ(tokens[12].type, TType::LCurly);
    ASSERT_EQ(tokens[13].type, TType::Return);
    ASSERT_EQ(tokens[14].type, TType::False);
    ASSERT_EQ(tokens[15].type, TType::RCurly);
}
