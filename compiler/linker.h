#pragma once
#include <string>
#include <vector>

class Linker {
public:
    // Takes a list of filenames from your dist/ folder and merges them
    static bool bundle(const std::vector<std::string>& files, const std::string& outputPath);
};