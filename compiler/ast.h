#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

// ================= JSX =================

struct JSXNode;

enum ChildType {
    CHILD_TEXT,
    CHILD_EXPR,
    CHILD_ELEMENT
};

struct JSXChild {
    ChildType type;
    std::string value;
    std::shared_ptr<JSXNode> element;
};

struct PropValue {
    bool isExpression = false;
    std::string value;
};

struct JSXNode {
    std::string tag;
    std::map<std::string, PropValue> props;
    std::vector<JSXChild> children;
    bool selfClosing = false;
};

// ================= FUNCTION =================

struct FunctionNode {

    std::string name;

    // ⭐ PARAMETERS (FIX)
    std::vector<std::string> params;

    std::vector<std::shared_ptr<JSXNode>> jsx_list;

    // event body
    std::string body;
};

// ================= IMPORT =================

enum ImportType {
    IMPORT_DEFAULT
};

struct ImportNode {
    ImportType type;
    std::string module;
};

// ================= PROGRAM =================

struct ProgramNode {
    std::vector<ImportNode> imports;
    std::vector<FunctionNode> functions;
};