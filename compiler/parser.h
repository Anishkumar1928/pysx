#pragma once

#include "lexer.h"
#include "ast.h"

#include <vector>
#include <memory>
#include <cstddef>

class Parser {
private:
    std::vector<Token> tokens;
    std::size_t pos;

    Token current();
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type);   // ✅ REQUIRED
    bool isAtEnd();

    std::shared_ptr<JSXNode> parseJSX();
    ImportNode parseImport();

public:
    explicit Parser(const std::vector<Token>& tokens);

    ProgramNode parseProgram();
    FunctionNode parseFunction();
};