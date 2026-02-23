#pragma once
#include <string>
#include <vector>
#include <cstddef>

// ============================================
// Token Types
// ============================================
enum TokenType {

    // keywords
    DEF,
    RETURN,
    IMPORT,
    FROM,
    AS,

    IDENTIFIER,

    // ⭐ JS literals
    TRUE,
    FALSE,
    NULLTOK,

    // JSX
    TAG_OPEN,
    TAG_CLOSE,
    SELF_CLOSE,

    ATTRIBUTE_NAME,
    ATTRIBUTE_VALUE,
    ATTRIBUTE_EXPR,
    EQUAL,

    EXPRESSION,
    TEXT,

    // syntax
    LPAREN,
    RPAREN,
    COLON,
    COMMA,
    LBRACKET,
    RBRACKET,
    STAR,

    STRING,

    END
};

// ============================================

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
private:
    std::string source;
    std::size_t pos;
    bool insideJSX;
    int jsxDepth;

public:
    explicit Lexer(const std::string& src);
    std::vector<Token> tokenize();
};