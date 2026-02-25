#include "generator.h"
#include "ast.h"
#include <cctype>
#include <vector>
#include <regex>
#include <iostream>

using namespace std;

/* =========================================================
   INDENT
========================================================= */
string indent(int level){
    return string(level*2,' ');
}

/* =========================================================
   ESCAPE JS STRING
========================================================= */
string escapeJS(const string& input){
    string out;
    for(char c:input){
        if(c=='\\') out+="\\\\";
        else if(c=='"') out+="\\\"";
        else out+=c;
    }
    return out;
}

/* =========================================================
   LAMBDA → ARROW
========================================================= */
string transformLambda(string expr){

    size_t pos=0;

    while(true){
        pos=expr.find("lambda",pos);
        if(pos==string::npos) break;

        size_t colon=expr.find(':',pos);
        if(colon==string::npos){
            pos+=6;
            continue;
        }

        string params=expr.substr(pos+6,colon-(pos+6));
        expr.replace(pos,colon-pos+1,"("+params+") =>");
        pos+=params.size()+6;
    }

    return expr;
}

/* =========================================================
   PYTHON → JS KEYWORDS
========================================================= */
string transformPythonKeywords(string expr){

    expr = regex_replace(expr, regex("\\bTrue\\b"), "true");
    expr = regex_replace(expr, regex("\\bFalse\\b"), "false");
    expr = regex_replace(expr, regex("\\bNone\\b"), "null");
    expr = regex_replace(expr, regex("\\belif\\b"), "else if");

    // Explicitly target Python Block Colons
    expr = std::regex_replace(expr, std::regex("\\b(if|else if|while|for)\\s+([^{]+?)\\s*:\\s*\\{"), "$1 ($2) {");
    expr = std::regex_replace(expr, std::regex("\\b(else|try|finally)\\s*:\\s*\\{"), "$1 {");
    expr = std::regex_replace(expr, std::regex("\\bexcept\\s+([^{]+?)\\s*:\\s*\\{"), "catch ($1) {");
    expr = std::regex_replace(expr, std::regex("\\bexcept\\s*:\\s*\\{"), "catch {");

    expr = regex_replace(expr,
        regex("([a-zA-Z0-9_])\\s+return"),
        "$1; return");

    // ES6 STRICT MODE VARIABLE DECLARATIONS
    try {
        std::regex assign_re("(^|\\n|[^a-zA-Z0-9_.*!<>+=\\-])([a-zA-Z_][a-zA-Z0-9_]*)\\s+=\\s+(?![=>])");
        expr = std::regex_replace(expr, assign_re, "$1var $2 = ");
        
        expr = std::regex_replace(expr, std::regex("var\\s+var\\s+"), "var ");
        expr = std::regex_replace(expr, std::regex("let\\s+var\\s+"), "let ");
        expr = std::regex_replace(expr, std::regex("const\\s+var\\s+"), "const ");
        
        // Healing multiple adjacent variables lacking semicolons (ReactXPy ASI)
        expr = std::regex_replace(expr, std::regex("([^;{\\s])\\s*\\b(var|let|const)\\b\\s+"), "$1; $2 ");
        
        // Healing flattened String assignments merged into Blocks (e.g. `var x = "str" if (`)
        expr = std::regex_replace(expr, std::regex("([\"']|__STR\\d+__)\\s*\\b(if|for|while)\\b"), "$1; $2");
    } catch(std::exception& e) {}

    // Python ternary: `value if cond else alt` → `cond ? value : alt`
    // Run AFTER var injection so the semicolon healing doesn't break it
    try {
        // Simple form: word/JSX-token if condition else word/null
        expr = std::regex_replace(expr,
            std::regex("(\\b[\\w.]+\\b|__JSX\\d+__)\\s+if\\s+([\\w.()]+)\\s+else\\s+(\\b[\\w.]+\\b|__JSX\\d+__)"),
            "$2 ? $1 : $3");
    } catch(std::exception&) {}

    return expr;
}

/* =========================================================
   useState
========================================================= */
string transformUseState(string body){

    // Replace: `val, setVal = useState(...)` with `const [val, setVal] = ReactXPy.useState(...)`
    // We can't use a simple regex `[^)]*` because of nested parentheses like `JSON.parse(...)`.
    // Instead of a complex regex, we do a manual matched-parenthesis string replacement.
    
    size_t pos = 0;
    while((pos = body.find("useState", pos)) != string::npos) {
        
        // Find the assignment part before `useState`
        size_t eq_pos = body.rfind('=', pos);
        if (eq_pos == string::npos) { pos += 8; continue; }
        
        string before_eq = body.substr(0, eq_pos);
        size_t comma_pos = before_eq.rfind(',');
        if (comma_pos == string::npos) { pos += 8; continue; }
        
        // Find variable names (very basic extraction)
        size_t start_var1 = comma_pos - 1;
        while(start_var1 > 0 && (isalnum(before_eq[start_var1]) || before_eq[start_var1] == '_' || before_eq[start_var1] == ' ' || before_eq[start_var1] == '\t' || before_eq[start_var1] == '\n')) {
            start_var1--;
        }
        
        if (!isalnum(before_eq[start_var1]) && before_eq[start_var1] != '_') {
            start_var1++; // Move past the delimiter (like `\n`, `{`, etc)
        }
        
        string var1 = before_eq.substr(start_var1, comma_pos - start_var1);
        string var2 = before_eq.substr(comma_pos + 1);
        
        // Trim whitespace
        var1.erase(0, var1.find_first_not_of(" \t\n\r"));
        var1.erase(var1.find_last_not_of(" \t\n\r") + 1);
        var2.erase(0, var2.find_first_not_of(" \t\n\r"));
        var2.erase(var2.find_last_not_of(" \t\n\r") + 1);
        
        // Find the `(` after `useState`
        size_t open_paren = body.find('(', pos + 8);
        if (open_paren == string::npos) { pos += 8; continue; }
        
        // Find matching closing parenthesis
        int depth = 1;
        size_t close_paren = open_paren + 1;
        while(close_paren < body.length() && depth > 0) {
            if (body[close_paren] == '(') depth++;
            else if (body[close_paren] == ')') depth--;
            close_paren++;
        }
        
        if (depth == 0) {
            string args = body.substr(open_paren + 1, close_paren - open_paren - 2);
            
            string replacement = "const [" + var1 + ", " + var2 + "] = ReactXPy.useState(" + args + ");";
            
            body.replace(start_var1, close_paren - start_var1, replacement);
            pos = start_var1 + replacement.length();
        } else {
            pos += 8;
        }
    }

    return body;
}

/* =========================================================
   useEffect
========================================================= */
string transformUseEffect(string body){
    size_t pos = 0;
    while((pos = body.find("useEffect", pos)) != string::npos) {
        
        // Find the `(` after `useEffect`
        size_t open_paren = body.find('(', pos + 9);
        if (open_paren == string::npos) { pos += 9; continue; }
        
        // Find matching closing parenthesis
        int depth = 1;
        size_t close_paren = open_paren + 1;
        while(close_paren < body.length() && depth > 0) {
            if (body[close_paren] == '(') depth++;
            else if (body[close_paren] == ')') depth--;
            close_paren++;
        }
        
        if (depth == 0) {
            string args = body.substr(open_paren + 1, close_paren - open_paren - 2);
            string replacement = "ReactXPy.useEffect(" + args + ");";
            
            body.replace(pos, close_paren - pos, replacement);
            pos += replacement.length();
        } else {
            pos += 9;
        }
    }
    return body;
}

/* =========================================================
   CONVERT INLINE JSX  (JSX inside {…} expression blocks)
   Converts:  <Tag Class="x">text</Tag>   →  React.createElement("Tag", {className:"x"}, "text")
              <Tag />                       →  React.createElement("Tag", null)
========================================================= */
string convertInlineJSX(string expr) {

    // --- Self-closing tags:  <Tag key="val" ... />  ---
    // Loop because there may be several in one expression
    while (true) {
        // Find a `<` that starts a tag name (alpha)
        size_t lt = string::npos;
        for (size_t i = 0; i < expr.size(); i++) {
            if (expr[i] == '<' && i + 1 < expr.size() && isalpha((unsigned char)expr[i+1])) {
                lt = i;
                break;
            }
        }
        if (lt == string::npos) break;

        // Find the matching `>` or `/>`
        size_t gt = expr.find('>', lt);
        if (gt == string::npos) break;

        bool selfClose = (gt > 0 && expr[gt-1] == '/');

        string tagChunk = expr.substr(lt + 1, gt - lt - 1); // inside < … >
        if (selfClose) tagChunk = tagChunk.substr(0, tagChunk.size() - 1); // strip trailing /

        // Parse tag name
        size_t ti = 0;
        while (ti < tagChunk.size() && (isalnum((unsigned char)tagChunk[ti]) || tagChunk[ti] == '_')) ti++;
        string tagName = tagChunk.substr(0, ti);
        string attrStr = tagChunk.substr(ti);

        // Build props object from attribute string
        // Handles: key="value"  and  key={expr}
        string propsCode = "null";
        if (!attrStr.empty()) {
            string propsContent;
            size_t ai = 0;
            while (ai < attrStr.size()) {
                while (ai < attrStr.size() && isspace((unsigned char)attrStr[ai])) ai++;
                if (ai >= attrStr.size()) break;
                // Read attr name
                size_t nameStart = ai;
                while (ai < attrStr.size() && attrStr[ai] != '=' && !isspace((unsigned char)attrStr[ai])) ai++;
                string attrName = attrStr.substr(nameStart, ai - nameStart);
                if (attrName == "Class") attrName = "className";
                while (ai < attrStr.size() && isspace((unsigned char)attrStr[ai])) ai++;
                if (ai >= attrStr.size() || attrStr[ai] != '=') {
                    if (!propsContent.empty()) propsContent += ", ";
                    propsContent += attrName + ": true";
                    continue;
                }
                ai++; // skip '='
                while (ai < attrStr.size() && isspace((unsigned char)attrStr[ai])) ai++;
                string attrVal;
                bool isExpr = false;
                if (ai < attrStr.size() && attrStr[ai] == '"') {
                    ai++;
                    while (ai < attrStr.size() && attrStr[ai] != '"') attrVal += attrStr[ai++];
                    if (ai < attrStr.size()) ai++; // skip closing "
                    attrVal = "\"" + attrVal + "\"";
                } else if (ai < attrStr.size() && attrStr[ai] == '{') {
                    ai++; int d = 1;
                    while (ai < attrStr.size() && d > 0) {
                        if (attrStr[ai] == '{') d++;
                        else if (attrStr[ai] == '}') d--;
                        if (d > 0) attrVal += attrStr[ai];
                        ai++;
                    }
                    isExpr = true;
                }
                if (!propsContent.empty()) propsContent += ", ";
                propsContent += attrName + ": " + attrVal;
            }
            if (!propsContent.empty()) propsCode = "{" + propsContent + "}";
        }

        string replacement;
        bool isComponent = !tagName.empty() && isupper((unsigned char)tagName[0]);
        string tagRef = isComponent ? tagName : "\"" + tagName + "\"";

        if (selfClose) {
            replacement = "React.createElement(" + tagRef + ", " + propsCode + ")";
            expr.replace(lt, gt - lt + 1, replacement);
        } else {
            // Look for closing tag </TagName>
            string closeTag = "</" + tagName + ">";
            size_t closePos = expr.find(closeTag, gt + 1);
            if (closePos == string::npos) break; // malformed, bail

            string innerContent = expr.substr(gt + 1, closePos - gt - 1);
            // Trim whitespace
            size_t first = innerContent.find_first_not_of(" \t\n\r");
            size_t last  = innerContent.find_last_not_of(" \t\n\r");
            string children = (first == string::npos) ? "" : innerContent.substr(first, last - first + 1);

            string childrenArg;
            if (children.empty()) {
                childrenArg = "";
            } else if (children.front() == '<') {
                // Nested JSX — recurse
                childrenArg = ", " + convertInlineJSX(children);
            } else {
                // Plain text
                childrenArg = ", \"" + children + "\"";
            }

            replacement = "React.createElement(" + tagRef + ", " + propsCode + childrenArg + ")";
            expr.replace(lt, closePos + closeTag.size() - lt, replacement);
        }
    }

    return expr;
}

/* =========================================================
   PREFIX PROPS
========================================================= */
string prefixProps(string expr,const vector<string>& params){

    expr = convertInlineJSX(expr);     // ← convert any <tag> inside expressions first
    expr=transformLambda(expr);
    expr=transformPythonKeywords(expr);

    for(const auto& p:params){
        regex r("\\b"+p+"\\b");
        expr=regex_replace(expr,r,"props."+p);
    }

    return expr;
}

/* =========================================================
   JSX PROPS
========================================================= */
string propsToJS(shared_ptr<JSXNode> node,
                 const vector<string>& params){

    if(node->props.empty())
        return "null";

    string r="{ ";
    bool first=true;

    for(auto &p:node->props){

        if(!first) r+=", ";

        string key=p.first;
        if(key=="class"||key=="Class")
            key="className";

        r+=key+": ";

        if(p.second.isExpression)
            r+=prefixProps(p.second.value,params);
        else
            r+="\""+escapeJS(p.second.value)+"\"";

        first=false;
    }

    r+=" }";
    return r;
}

/* =========================================================
   JSX GENERATOR
========================================================= */
string generateJSX(shared_ptr<JSXNode> node,
                   const vector<string>& params,
                   int level){

    bool isComponent =
        !node->tag.empty() &&
        isupper(node->tag[0]);

    string out;

    if(isComponent)
        out+="React.createElement("+node->tag+", ";
    else
        out+="React.createElement(\""+node->tag+"\", ";

    out+=propsToJS(node,params);

    for(auto &child:node->children){

        out+=",\n"+indent(level);

        if(child.type==CHILD_TEXT)
            out+="\""+escapeJS(child.value)+"\"";

        else if(child.type==CHILD_EXPR)
            out+=prefixProps(child.value,params);

        else if(child.type==CHILD_ELEMENT)
            out+=generateJSX(child.element,params,level+1);
    }

    out+=")";
    return out;
}

/* =========================================================
   REMOVE OUTER PYTHON BLOCK {}
========================================================= */
string stripOuterBlock(string body){

    body = regex_replace(body, regex("^\\s*\\{"), "");
    body = regex_replace(body, regex("\\}\\s*$"), "");
    return body;
}

/* =========================================================
   FUNCTION GENERATOR ⭐ FIXED
========================================================= */
string generateFunction(const FunctionNode& fn){

    string out="function "+fn.name+"(props){\n";

    if(!fn.body.empty()){

        string body = fn.body;
        
        if (fn.name == "SidebarLink") {
            std::cout << "--- SidebarLink RAW BODY ---" << std::endl;
            std::cout << body << std::endl;
            std::cout << "----------------------------" << std::endl;
        }

        body = stripOuterBlock(body);

        if (fn.name == "SidebarLink") {
            std::cout << "--- SidebarLink AFTER STRIP ---" << std::endl;
            std::cout << body << std::endl;
            std::cout << "----------------------------" << std::endl;
        }

        body = transformUseState(body);
        body = transformUseEffect(body);
        body = transformPythonKeywords(body);
        body = transformLambda(body);

        body = regex_replace(body,regex("#.*"),"");

        /* JSX REPLACEMENTS */
        for(size_t i=0;i<fn.jsx_list.size();i++){

            string token="__JSX"+to_string(i)+"__";
            string jsx=generateJSX(fn.jsx_list[i],fn.params,1);

            body=regex_replace(body, regex(token), jsx);
        }

        /* AUTO RETURN */
        if(body.find("return")==string::npos &&
           !fn.jsx_list.empty()){
            body="return "+
                generateJSX(fn.jsx_list.back(),
                            fn.params,1);
        }

        out += indent(1) + body + "\n";
    }

    out+="}\n\n";
    return out;
}

/* =========================================================
   IMPORT
========================================================= */
string generateImport(const ImportNode& imp){
    return "import "+imp.module+
           " from \"./"+imp.module+".js\";\n";
}

/* =========================================================
   PROGRAM
========================================================= */
string Generator::generate(const ProgramNode& program){

    string out;

    for(const auto& imp:program.imports)
        out+=generateImport(imp);

    if(!program.imports.empty())
        out+="\n";

    for(const auto& fn:program.functions)
        out+=generateFunction(fn);

    if(!program.functions.empty())
        out+="export default "+
             program.functions.back().name+";\n";

    return out;
}