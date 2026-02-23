#include "generator.h"
#include "ast.h"
#include <cctype>
#include <vector>

using namespace std;

// --------------------------------
// indentation helper
// --------------------------------
string indent(int level){
    return string(level*2,' ');
}

// --------------------------------
// escape JS string
// --------------------------------
string escapeJS(const string& input){
    string out;
    for(char c:input){
        if(c=='\\') out+="\\\\";
        else if(c=='"') out+="\\\"";
        else out+=c;
    }
    return out;
}

// --------------------------------
// lambda → arrow transform
// --------------------------------
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

        expr.replace(pos, colon-pos+1,
                     "(" + params + ") =>");

        pos += params.size()+6;
    }

    return expr;
}

// --------------------------------
// Python -> JS keyword transform
// --------------------------------
string transformPythonKeywords(string expr){
    auto replaceWord = [](string& str, const string& from, const string& to) {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != string::npos) {
            bool left = (pos == 0 || (!isalnum(str[pos-1]) && str[pos-1] != '_'));
            bool right = (pos + from.length() >= str.length() || (!isalnum(str[pos+from.length()]) && str[pos+from.length()] != '_'));
            if (left && right) {
                str.replace(pos, from.length(), to);
                pos += to.length();
            } else {
                pos += from.length();
            }
        }
    };
    replaceWord(expr, "True", "true");
    replaceWord(expr, "False", "false");
    replaceWord(expr, "None", "null");
    return expr;
}

// --------------------------------
// useState transform (stable)
// --------------------------------
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

        // Trace back from comma to find the start of 'a', separated by spaces or newlines
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
            // Find matching parenthesis
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
        if(p1 != string::npos && p2 < body.size()) {
            if(p2 > p1 + 1) {
                arg = body.substr(p1+1, p2-p1-1);
            }
            replace_end = p2 + 1;
        } else {
            replace_end = call + 8;
        }

        string replacement = "const [" + a + ", " + b + "] = ReactXPy.useState(" + arg + ");";
        body.replace(start, replace_end - start, replacement);
        pos = start + replacement.size();
    }
    return body;
}

// --------------------------------
// useEffect transform (robust)
// --------------------------------
string transformUseEffect(string body){

    string result;
    size_t i = 0;

    while(i < body.size()){

        if(body.substr(i,9) == "useEffect"){

            // avoid double prefix
            if(i>=8 && body.substr(i-8,8)=="ReactXPy"){
                result += "useEffect";
                i += 9;
                continue;
            }

            result += "ReactXPy.useEffect";
            i += 9;
        }
        else{
            result += body[i];
            i++;
        }
    }

    return result;
}

// --------------------------------
// prefix props variables
// --------------------------------
string prefixProps(string expr,
                   const vector<string>& params)
{
    expr = transformLambda(expr);
    expr = transformPythonKeywords(expr);

    for(const auto& p:params){

        size_t pos=0;

        while((pos=expr.find(p,pos))!=string::npos){

            bool left=(pos==0||!isalnum(expr[pos-1]));
            bool right=(pos+p.size()>=expr.size()
                        ||!isalnum(expr[pos+p.size()]));

            if(left&&right){
                expr.replace(pos,p.size(),
                             "props."+p);
                pos+=6+p.size();
            }else pos++;
        }
    }
    return expr;
}

// --------------------------------
// INLINE JSX compiler
// --------------------------------
string compileInlineJSX(const string& jsx){

    size_t ts=jsx.find('<')+1;
    size_t te=jsx.find('>');

    string tagStr = jsx.substr(ts, te - ts);
    string tag = tagStr;
    string props = "null";

    size_t spacePos = tagStr.find(' ');
    if (spacePos != string::npos) {
        tag = tagStr.substr(0, spacePos);
        string attrs = tagStr.substr(spacePos + 1);

        size_t classPos = attrs.find("class=");
        if (classPos != string::npos) {
            size_t q1 = attrs.find('"', classPos);
            size_t q2 = (q1 != string::npos) ? attrs.find('"', q1 + 1) : string::npos;
            if (q1 != string::npos && q2 != string::npos) {
                string className = attrs.substr(q1 + 1, q2 - q1 - 1);
                props = "{ className: \"" + className + "\" }";
            }
        }
    }

    size_t cs=te+1;
    size_t ce=jsx.find("</");

    string text=jsx.substr(cs,ce-cs);

    return "React.createElement(\""+tag+
           "\", " + props + ", \""+escapeJS(text)+"\")";
}

bool looksLikeJSX(const string& e){
    return e.find('<')!=string::npos;
}

// --------------------------------
// props → JS object
// --------------------------------
string propsToJS(shared_ptr<JSXNode> node,
                 const vector<string>& params)
{
    if(node->props.empty())
        return "null";

    string r="{ ";
    bool first=true;

    for(auto &p:node->props){

        if(!first) r+=", ";

        string key =
            (p.first=="class")
            ? "className"
            : p.first;

        r+=key+": ";

        if(key.rfind("on",0)==0 &&
           p.second.isExpression)
            r+=transformLambda(p.second.value);

        else if(p.second.isExpression)
            r+=prefixProps(p.second.value,params);

        else
            r+="\""+escapeJS(p.second.value)+"\"";

        first=false;
    }

    r+=" }";
    return r;
}

// --------------------------------
// JSX GENERATOR
// --------------------------------
string generateJSX(shared_ptr<JSXNode> node,
                   const vector<string>& params,
                   int level)
{
    bool isComponent =
        !node->tag.empty() &&
        isupper(node->tag[0]);

    string out;

    if(isComponent)
        out+="React.createElement("+node->tag+", ";
    else
        out+="React.createElement(\""+
              node->tag+"\", ";

    out+=propsToJS(node,params);

    for(auto &child:node->children){

        out+=",\n"+indent(level);

        if(child.type==CHILD_TEXT)
            out+="\""+escapeJS(child.value)+"\"";

        else if(child.type==CHILD_EXPR){
            string expr = prefixProps(child.value, params);

            if(looksLikeJSX(expr)){
                size_t jsxStart = expr.find('<');
                string before = expr.substr(0, jsxStart);
                string jsxPart = expr.substr(jsxStart);
                out += before + compileInlineJSX(jsxPart);
            }
            else{
                out += expr;
            }
        }

        else if(child.type==CHILD_ELEMENT)
            out+=generateJSX(child.element,
                             params,
                             level+1);
    }

    out+=")";
    return out;
}

string generateJSX(shared_ptr<JSXNode> node){
    vector<string> empty;
    return generateJSX(node,empty,1);
}

// --------------------------------
// FUNCTION GENERATOR
// --------------------------------
string generateFunction(const FunctionNode& fn){

    string out;

    out+="function "+fn.name+"(props){\n";

    if(!fn.body.empty()){

        string body = transformUseState(fn.body);
        body = transformUseEffect(body);
        body = transformPythonKeywords(body);
        body = transformLambda(body);

        out+="  "+body+";\n";
    }

    if(fn.jsx){
        out+="  return ";
        out+=generateJSX(fn.jsx,fn.params,1);
        out+=";\n";
    }

    out+="}\n\n";
    return out;
}

// --------------------------------
// IMPORT GENERATOR
// --------------------------------
string generateImport(const ImportNode& imp){
    return "import "+imp.module+
           " from \"./"+imp.module+".js\";\n";
}

// --------------------------------
// PROGRAM GENERATOR
// --------------------------------
string Generator::generate(const ProgramNode& program)
{
    string out;

    out+="import * as ReactXPy from "
         "\"../runtime/runtime.js\";\n\n";

    for(const auto& imp:program.imports)
        out+=generateImport(imp);

    if(!program.imports.empty())
        out+="\n";

    for(const auto& fn:program.functions)
        out+=generateFunction(fn);

    return out;
}