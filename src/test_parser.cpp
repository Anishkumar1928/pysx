#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"

using namespace std;

string readFile(const string& path) {
    ifstream f(path);
    stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

string transformUseState(string body){
    size_t pos = 0;
    while(true){
        size_t call = body.find("useState", pos);
        if(call == string::npos) break;

        size_t eq = body.rfind('=', call);
        size_t comma = body.rfind(',', eq);

        if(eq == string::npos || comma == string::npos || comma >= eq || eq >= call) {
            pos = call + 8;
            continue;
        }

        size_t start = comma;
        while(start > pos && body[start-1] != ';' && body[start-1] != '{' && body[start-1] != '}') {
            start--;
        }

        auto trim = [](string s){
            size_t s_start=s.find_first_not_of(" \t\n\r");
            size_t s_end=s.find_last_not_of(" \t\n\r");
            if(s_start == string::npos) return string("");
            return s.substr(s_start, s_end-s_start+1);
        };

        string a = trim(body.substr(start, comma - start));
        string b = trim(body.substr(comma+1, eq - comma - 1));

        string arg="0";
        size_t p1 = body.find('(', call);
        size_t p2 = string::npos;
        if(p1 != string::npos) {
            int depth = 1;
            p2 = p1 + 1;
            while(p2 < body.size()) {
                if(body[p2] == '(') depth++;
                else if(body[p2] == ')') {
                    depth--;
                    if(depth == 0) break;
                }
                p2++;
            }
        }

        size_t replace_end;
        if(p1 != string::npos && p2 < body.size() && p2 > p1+1) {
            arg = body.substr(p1+1, p2-p1-1);
            replace_end = p2 + 1;
        } else {
            replace_end = call + 8;
        }

        string replacement = "const [" + a + ", " + b + "] = Pysx.useState(" + arg + ");";
        body.replace(start, replace_end - start, replacement);
        pos = start + replacement.size();
    }
    return body;
}

string transformUseEffect(string body){
    string result;
    size_t i = 0;
    while(i < body.size()){
        if(body.substr(i,9) == "useEffect"){
            if(i>=5 && body.substr(i-5,5)=="Pysx."){
                result += "useEffect";
                i += 9;
                continue;
            }
            result += "Pysx.useEffect";
            i += 9;
        }
        else{
            result += body[i];
            i++;
        }
    }
    return result;
}

string transformLambda(string expr){
    size_t pos = 0;
    while(true){
        pos = expr.find("lambda", pos);
        if(pos == string::npos) break;
        size_t colon = expr.find(':', pos);
        if(colon == string::npos){
            pos += 6;
            continue;
        }
        string params = expr.substr(pos+6, colon-(pos+6));
        size_t s=params.find_first_not_of(" \t");
        size_t e=params.find_last_not_of(" \t");
        if(s==string::npos) params="";
        else params=params.substr(s,e-s+1);
        expr.replace(pos, colon-pos+1, "(" + params + ") =>");
        pos += params.size()+6;
    }
    return expr;
}

int main() {
    string src = readFile("my-app/src/App.pysx");
    Lexer lexer(src);
    vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    try {
        ProgramNode ast = parser.parseProgram();
        for (auto& fn : ast.functions) {
            cout << "--- " << fn.name << " Base Body ---" << endl;
            cout << fn.body << endl;
            string s1 = transformUseState(fn.body);
            cout << "--- After State ---" << endl;
            cout << s1 << endl;
            string s2 = transformUseEffect(s1);
            cout << "--- After Effect ---" << endl;
            cout << s2 << endl;
            string s3 = transformLambda(s2);
            cout << "--- After Lambda ---" << endl;
            cout << s3 << endl;
        }
    } catch (const exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}
