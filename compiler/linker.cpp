#include "linker.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

using namespace std;

bool Linker::bundle(const vector<string>& files, const string& outputPath) {
    stringstream bundleBuffer;

    // 1. Add a global React check (for browser environments)
    bundleBuffer << "/* PYSX Generated Bundle */\n";
    bundleBuffer << "const React = window.React;\n\n";

    for (const string& fileName : files) {
        ifstream file(fileName);
        if (!file.is_open()) continue;

        string line;
        while (getline(file, line)) {
            // Remove local imports (e.g., import Header from "./Header.js")
            if (line.find("import") == 0 && line.find("./") != string::npos) continue;
            
            // Remove internal exports (except for the final App)
            if (line.find("export default") == 0 && line.find("App") == string::npos) continue;

            bundleBuffer << line << "\n";
        }
        bundleBuffer << "\n";
    }

    // 2. Write the final bundle
    ofstream out(outputPath, ios::out | ios::trunc);
    if (!out.is_open()) return false;

    out << bundleBuffer.str();
    out.close();
    return true;
}