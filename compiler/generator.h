#pragma once
#include <string>
#include "ast.h"

class Generator {
public:
    static std::string generate(const ProgramNode& program);
};