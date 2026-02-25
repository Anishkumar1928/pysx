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
// IMPORT PARSER
// =================================
ImportNode Parser::parseImport() {

    ImportNode imp;
    imp.type = IMPORT_DEFAULT;

    match(IMPORT);

    if (!check(IDENTIFIER))
        throw runtime_error("Expected identifier after import");

    imp.module = advance().value;

    if (match(FROM)) {
        if (check(STRING))
            advance();
    }

    return imp;
}

// =================================
// PROGRAM PARSER
// =================================
ProgramNode Parser::parseProgram() {

    ProgramNode program;

    while (!isAtEnd()) {

        if (check(IMPORT))
            program.imports.push_back(parseImport());

        else if (check(DEF))
            program.functions.push_back(parseFunction());

        else
            advance();
    }

    return program;
}

// =================================
// FUNCTION PARSER  ⭐ FIXED
// Proper Python INDENT → JS blocks
// =================================
FunctionNode Parser::parseFunction() {

    FunctionNode fn;

    match(DEF);

    if (!check(IDENTIFIER))
        throw runtime_error("Expected function name");

    fn.name = advance().value;

    // -------- parameters --------
    match(LPAREN);

    while (!check(RPAREN) && !isAtEnd()) {

        if (check(IDENTIFIER))
            fn.params.push_back(advance().value);

        if (check(COMMA))
            advance();
    }

    match(RPAREN);
    match(COLON);

    string body;
    int indentLevel = 0;

    // =============================
    // BODY COLLECTION (REAL FIX)
    // =============================
    while (!isAtEnd()) {

        // next function begins
        if (check(DEF) && indentLevel == 0)
            break;

        // ----- INDENT -----
        if (match(INDENT)) {
            body += "{\n";
            indentLevel++;
            continue;
        }

        // ----- DEDENT -----
        if (match(DEDENT)) {
            body += "}\n";
            indentLevel--;
            continue;
        }

        // ----- RETURN -----
        if (match(RETURN)) {
            body += "return ";
            continue;
        }

        // ----- INLINE JSX -----
        if (check(TAG_OPEN)) {
            fn.jsx_list.push_back(parseJSX());
            body += "__JSX" + to_string(fn.jsx_list.size() - 1) + "__ ";
            continue;
        }

        Token t = advance();

        // preserve strings safely
        if (t.type == STRING) {

            string escaped;
            for(char c : t.value){
                if(c=='"') escaped+="\\\"";
                else if(c=='\\') escaped+="\\\\";
                else if(c=='\n') escaped+="\\n";
                else escaped+=c;
            }

            body += "\"" + escaped + "\"";
        }
        else {
            body += t.value;
        }

        body += " ";
    }

    fn.body = body;
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

    // ---------- props ----------
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

    // self closing
    if (match(SELF_CLOSE)) {
        node->selfClosing = true;
        return node;
    }

    // ---------- children ----------
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
            advance();
        }
    }

    throw runtime_error("JSX parse failed");
}