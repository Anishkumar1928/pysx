#include "parser.h"
#include <stdexcept>

using namespace std;

// --------------------------------
// Constructor
// --------------------------------
Parser::Parser(const vector<Token>& t) {
    tokens = t;
    pos = 0;
}

bool Parser::isAtEnd() {
    return pos >= tokens.size() ||
           tokens[pos].type == END;
}

Token Parser::current() {
    if (isAtEnd()) return tokens.back();
    return tokens[pos];
}

Token Parser::advance() {
    if (!isAtEnd()) pos++;
    return tokens[pos - 1];
}

bool Parser::match(TokenType type) {
    if (!isAtEnd() && current().type == type) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return current().type == type;
}

// =================================
// PROGRAM PARSER
// =================================
ProgramNode Parser::parseProgram() {

    ProgramNode program;

    while (!isAtEnd()) {
        if (check(DEF))
            program.functions.push_back(parseFunction());
        else
            advance();
    }

    return program;
}

// =================================
// FUNCTION PARSER
// Supports:
//
// def App():
//     statements...
//     return <JSX>
//
// OR event functions
// =================================
FunctionNode Parser::parseFunction() {

    FunctionNode fn;

    // def
    match(DEF);

    if (!check(IDENTIFIER))
        throw runtime_error("Expected function name");

    fn.name = advance().value;

    // ---------- PARAMETERS ----------
    match(LPAREN);

    while (!check(RPAREN) && !isAtEnd()) {

        if (check(IDENTIFIER))
            fn.params.push_back(advance().value);

        if (check(COMMA))
            advance();
    }

    match(RPAREN);
    match(COLON);

    // ---------- BODY COLLECTION ----------
    string body;

    while (!isAtEnd()) {

        // stop if next function begins
        if (check(DEF))
            break;

        // ⭐ return JSX detected
        if (match(RETURN)) {
            fn.body = body;
            fn.jsx = parseJSX();
            return fn;
        }

        Token t = advance();

        // preserve strings (output as standard JS strings, escaping newlines to support multiline)
        if (t.type == STRING) {
            string escaped;
            for(char c : t.value) {
                if(c == '"') escaped += "\\\"";
                else if(c == '\\') escaped += "\\\\";
                else if(c == '\n') escaped += "\\n";
                else if(c == '\r') escaped += "\\r";
                else if(c == '\t') escaped += "\\t";
                else escaped += c;
            }
            body += "\"" + escaped + "\"";
        }
        else
            body += t.value;

        // controlled spacing
        if (t.type != LPAREN &&
            t.type != RPAREN &&
            t.type != COMMA)
        {
            body += " ";
        }
    }

    // event-only function (no JSX)
    fn.body = body;
    fn.jsx = nullptr;

    return fn;
}

// =================================
// JSX PARSER
// =================================
shared_ptr<JSXNode> Parser::parseJSX() {

    if (!match(TAG_OPEN))
        throw runtime_error("Expected opening tag");

    auto node = make_shared<JSXNode>();
    node->tag = tokens[pos - 1].value;

    // ---------- PROPS ----------
    while (check(ATTRIBUTE_NAME)) {

        string key = advance().value;
        match(EQUAL);

        PropValue prop;

        if (check(ATTRIBUTE_VALUE)) {
            prop.value = advance().value;
        }
        else if (check(ATTRIBUTE_EXPR) || check(EXPRESSION)) {
            prop.isExpression = true;
            prop.value = advance().value;
        }

        node->props[key] = prop;
    }

    // ---------- SELF CLOSING ----------
    if (match(SELF_CLOSE)) {
        node->selfClosing = true;
        return node;
    }

    // ---------- CHILDREN ----------
    while (!isAtEnd()) {

        if (check(TEXT)) {
            JSXChild c;
            c.type = CHILD_TEXT;
            c.value = advance().value;
            node->children.push_back(c);
        }
        else if (check(EXPRESSION)) {
            JSXChild c;
            c.type = CHILD_EXPR;
            c.value = advance().value;
            node->children.push_back(c);
        }
        else if (check(TAG_OPEN)) {
            JSXChild c;
            c.type = CHILD_ELEMENT;
            c.element = parseJSX();
            node->children.push_back(c);
        }
        else if (check(TAG_CLOSE)) {
            advance();
            return node;
        }
        else {
            advance(); // skip unknown tokens safely
        }
    }

    throw runtime_error("JSX parse failed");
}