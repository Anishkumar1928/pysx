#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "generator.h"
#include "linker.h" // ⭐ New: Include the Linker

using namespace std;

void printUsage() {
    cout << "Usage:\n";
    cout << "  Compile: ./pysx <input.pysx> -o <output.js>\n";
    cout << "  Bundle:  ./pysx --bundle <file1.js> <file2.js> ... <output.js>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    string firstArg = argv[1];

    // =====================================================
    // ⭐ MODE 1: BUNDLE / LINKER
    // =====================================================
    if (firstArg == "--bundle") {
        if (argc < 4) {
            cout << "❌ Error: Not enough files to bundle.\n";
            printUsage();
            return 1;
        }

        vector<string> filesToLink;
        // All arguments between --bundle and the last argument are input files
        for (int i = 2; i < argc - 1; ++i) {
            filesToLink.push_back(argv[i]);
        }
        
        // The very last argument is the destination bundle name
        string bundlePath = argv[argc - 1];

        cout << "⚙️  Linking " << filesToLink.size() << " files...\n";
        if (Linker::bundle(filesToLink, bundlePath)) {
            cout << "✅ Bundle Created → " << bundlePath << "\n";
        } else {
            cout << "❌ Linking failed.\n";
            return 1;
        }
        return 0;
    }

    // =====================================================
    // ⭐ MODE 2: COMPILE
    // =====================================================
    string inputFile = "";
    string outputFile = "";

    if (argc == 2) {
        inputFile = argv[1];
        outputFile = inputFile;
        size_t dot = outputFile.find_last_of('.');
        if (dot != string::npos) outputFile = outputFile.substr(0, dot);
        outputFile += ".js";
    } 
    else if (argc == 4 && string(argv[2]) == "-o") {
        inputFile = argv[1];
        outputFile = argv[3];
    } 
    else {
        printUsage();
        return 1;
    }

    // --- READ SOURCE ---
    ifstream in(inputFile);
    if (!in.is_open()) {
        cout << "❌ Cannot open " << inputFile << "\n";
        return 1;
    }
    stringstream buffer;
    buffer << in.rdbuf();
    string source = buffer.str();
    in.close();

    // --- LEXER ---
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    // --- PARSER ---
    Parser parser(tokens);
    ProgramNode program = parser.parseProgram();

    // --- GENERATOR ---
    string output = Generator::generate(program);

    // --- WRITE OUTPUT ---
    ofstream out(outputFile, ios::out | ios::trunc);
    if (!out.is_open()) {
        cout << "❌ Cannot write " << outputFile << "\n";
        return 1;
    }
    out << output;
    out.close();

    cout << "✅ Compiled → " << outputFile << "\n";
    return 0;
}