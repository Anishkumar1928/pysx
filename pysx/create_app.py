import os
import sys

def create_file(path, content):
    with open(path, 'w') as f:
        f.write(content.strip() + '\n')
    print(f"Created {path}")

def main():
    if len(sys.argv) > 1 and sys.argv[1] in ["-h", "--help"]:
        print("Usage: create-pysx-app [project-name]")
        print("\\nOptions:")
        print("  -h, --help       Show this help message and exit.")
        print("  -v, --version    Show the version of create-pysx-app.")
        print("\\nExample:")
        print("  create-pysx-app my-new-project")
        sys.exit(0)

    if len(sys.argv) > 1 and sys.argv[1] in ["-v", "--version"]:
        print("create-pysx-app version 0.1.0")
        sys.exit(0)

    print("\\033[0;34mWelcome to create-pysx-app! Let's scaffold your new project.\\033[0m\\n")

    project_name = ""
    if len(sys.argv) > 1:
        project_name = sys.argv[1]
        
    while not project_name:
        try:
            project_name = input("? What is your project named? ")
        except (KeyboardInterrupt, EOFError):
            print("\\nOperation cancelled.")
            sys.exit(1)
            
    project_name = project_name.strip()
    if not project_name:
        print("Error: Project name cannot be empty.")
        sys.exit(1)
    
    if os.path.exists(project_name):
        print(f"Error: Directory '{project_name}' already exists.")
        sys.exit(1)
        
    print(f"\\n🚀 Creating a new PYSX project in {os.path.abspath(project_name)}...")
    
    # Create directory structure
    dirs = [
        project_name,
        os.path.join(project_name, "public"),
        os.path.join(project_name, "src"),
        os.path.join(project_name, "src", "components"),
        os.path.join(project_name, "runtime")
    ]
    
    for d in dirs:
        os.makedirs(d, exist_ok=True)
        
    # App.pysx
    app_pysx = """
import TodoApp

def App():
    return <TodoApp />
"""

    # TodoApp.pysx
    todoapp_pysx = """
def TodoApp(props):
    todos, setTodos = useState(JSON.parse(localStorage.getItem("todos")) || [])
    inputValue, setInputValue = useState("")

    useEffect(lambda: localStorage.setItem("todos", JSON.stringify(todos)), [todos])

    return <div class="todo-wrapper">
        <div class="todo-card">
            <div class="todo-header">
                <h2>✨ Task Manager</h2>
                <p>Manage your daily goals with PYSX v2 state hooks!</p>
            </div>
            
            <div class="todo-input-group">
                <input type="text" placeholder="What needs to be done?" value={inputValue} onChange={lambda e: setInputValue(e.target.value)} />
                <button class="btn-primary" onClick={lambda: setInputValue("") || setTodos([...todos, {id: Date.now(), text: inputValue, done: false}])}>Add Task</button>
            </div>

            <div class="todo-list">
                {todos.length === 0 ? React.createElement("p", {className: "empty-state"}, "No tasks yet! Add one above.") : todos.map(lambda t: React.createElement("div", {className: t.done ? "todo-item done" : "todo-item", key: t.id}, React.createElement("div", {className: "todo-content"}, React.createElement("input", {type: "checkbox", checked: t.done, onChange: lambda: setTodos(todos.map(lambda item: item.id === t.id ? {...item, done: !item.done} : item))}), React.createElement("span", null, t.text)), React.createElement("button", {className: "btn-icon red", onClick: lambda: setTodos(todos.filter(lambda item: item.id !== t.id))}, "🗑️")))}
            </div>
            
            <div class="todo-footer">
                <span>{todos.filter(lambda t: !t.done).length} items remaining</span>
                <button class="btn-outline" onClick={lambda: setTodos([])}>Clear All</button>
            </div>
        </div>
    </div>
"""

    # runtime.js
    runtime_js = """
// runtime/runtime.js

// -----------------------------
// Hook wrappers (simple bridge)
// -----------------------------
function useState(initial) {
  return React.useState(initial);
}

function useEffect(effect, deps) {
  return React.useEffect(effect, deps);
}

// -----------------------------
// App runner
// -----------------------------
function run(App) {

  const rootElement =
    document.getElementById("root") ||
    document.body;

  const root = ReactDOM.createRoot(rootElement);

  root.render(
    React.createElement(App)
  );
}

// -----------------------------
// Global API
// -----------------------------
window.Pysx = {
  run,
  useState,
  useEffect
};
"""

    # index.html
    index_html = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>PYSX App</title>
    <script src="https://unpkg.com/react@18/umd/react.development.js"></script>
    <script src="https://unpkg.com/react-dom@18/umd/react-dom.development.js"></script>
    <link rel="stylesheet" href="./style.css">
</head>
<body>
    <div id="root"></div>
    <script src="../runtime/runtime.js"></script>
    <script src="../dist/bundle.js"></script>
    <script>
        Pysx.run(App);
    </script>
</body>
</html>
"""

    # style.css
    style_css = """
:root {
    --bg-main: #f8fafc;
    --sidebar-bg: #ffffff;
    --card-bg: #ffffff;
    --text-primary: #0f172a;
    --text-secondary: #64748b;
    --accent: #4f46e5;
    --accent-hover: #4338ca;
    --border: #f1f5f9;
    --danger: #ef4444;
    --success: #10b981;
    --warning: #f59e0b;
}

body {
    margin: 0;
    font-family: 'Inter', system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
    background-color: var(--bg-main);
    color: var(--text-primary);
    -webkit-font-smoothing: antialiased;
    display: flex;
    justify-content: center;
    padding-top: 50px;
}

h1, h2, h3, p { margin: 0; }
button { font-family: inherit; cursor: pointer; transition: 0.2s; border: none; }

.btn-primary { background: var(--accent); color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; box-shadow: 0 4px 6px rgba(79, 70, 229, 0.2); }
.btn-primary:hover { background: var(--accent-hover); transform: translateY(-1px); }
.btn-outline { border: 1px solid #cbd5e1; background: white; padding: 10px 16px; border-radius: 8px; font-weight: 500; }
.btn-outline:hover { background: #f8fafc; }

/* Todo App Styling */
.todo-wrapper { margin-bottom: 24px; width: 100vw; max-width: 600px;}
.todo-card { background: var(--card-bg); border-radius: 20px; padding: 32px; box-shadow: 0 10px 30px rgba(0,0,0,0.03); }
.todo-header { display: flex; flex-direction: column; margin-bottom: 24px; }
.todo-header h2 { font-size: 1.5rem; color: var(--text-primary); font-weight: 700; margin-bottom: 4px; }
.todo-header p { color: var(--text-secondary); font-size: 0.9rem; }

.todo-input-group { display: flex; gap: 12px; margin-bottom: 32px; }
.todo-input-group input { flex: 1; padding: 14px 20px; border-radius: 12px; border: 1px solid var(--border); background: #f8fafc; font-size: 1rem; color: var(--text-primary); outline: none; transition: 0.2s; }
.todo-input-group input:focus { border-color: var(--accent); background: white; box-shadow: 0 0 0 3px rgba(79, 70, 229, 0.1); }
.todo-input-group button { padding: 14px 24px; border-radius: 12px; font-weight: 600; white-space: nowrap; }

.todo-list { display: flex; flex-direction: column; gap: 12px; margin-bottom: 24px; }
.todo-item { display: flex; align-items: center; justify-content: space-between; padding: 16px 20px; background: #f8fafc; border-radius: 12px; border: 1px solid transparent; transition: 0.2s; }
.todo-item:hover { background: white; border-color: var(--border); box-shadow: 0 4px 12px rgba(0,0,0,0.02); }
.todo-item.done span { text-decoration: line-through; color: var(--text-secondary); opacity: 0.7; }
.todo-content { display: flex; align-items: center; gap: 16px; font-weight: 500; font-size: 1.05rem; color: var(--text-primary); }
.todo-content input[type="checkbox"] { width: 22px; height: 22px; cursor: pointer; accent-color: var(--success); }

.empty-state { text-align: center; color: var(--text-secondary); font-size: 0.95rem; padding: 32px 0; background: #f8fafc; border-radius: 12px; border: 1px dashed var(--border); }

.todo-footer { display: flex; justify-content: space-between; align-items: center; padding-top: 16px; border-top: 1px solid var(--border); color: var(--text-secondary); font-size: 0.9rem; font-weight: 500; }
.todo-footer button { padding: 8px 16px; border-radius: 8px; font-size: 0.85rem; }
.btn-icon { background: transparent; border: none; font-size: 1.25rem; color: var(--text-secondary); border-radius: 50%; width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; }
.btn-icon:hover { background: #fee2e2; color: var(--danger); }
"""

    # build.py
    build_py = """
import os
import subprocess
from pathlib import Path

def main():
    print("🚀 Building PYSX App...")
    
    os.makedirs("dist", exist_ok=True)
    compiled_files = []
    
    from shutil import which
    if which("pysx") is None:
        print("❌ Error: pysx compiler not found. Please install the pysx pip package globally.")
        return
        
    for file in Path("src").rglob("*.pysx"):
        basename = file.stem
        output_path = f"dist/{basename}.js"
        
        print(f"⚡ Compiling: {file} -> {output_path}")
        subprocess.run(["pysx", str(file), "-o", output_path], check=True)
        compiled_files.append(output_path)
        
    if compiled_files:
        print("\\n🔗 Linking into dist/bundle.js...")
        subprocess.run(["pysx", "--bundle"] + compiled_files + ["dist/bundle.js"], check=True)
        print("\\n✅ Success! Build complete.")
    else:
        print("❌ Error: No .pysx files found in src directory.")

if __name__ == "__main__":
    main()
"""

    # dev.py
    dev_py = """
import os
import sys
import time
import subprocess
import threading
from pathlib import Path

def get_mtimes():
    mtimes = {}
    for p in Path("src").rglob("*.pysx"):
        mtimes[str(p)] = p.stat().st_mtime
    return mtimes

def serve():
    from http.server import SimpleHTTPRequestHandler
    from socketserver import TCPServer
    
    class QuietHandler(SimpleHTTPRequestHandler):
        def log_message(self, format, *args):
            pass # Keep terminal clean

    TCPServer.allow_reuse_address = True
    try:
        httpd = TCPServer(("", 3000), QuietHandler)
        print("🌐 Live Server running at http://localhost:3000/public/index.html")
        httpd.serve_forever()
    except OSError as e:
        print(f"❌ Could not start dev server: {e}")
        sys.exit(1)

def main():
    print("🚀 Starting PYSX Live Development Server...")
    subprocess.run([sys.executable, "build.py"], check=False)
    
    server_thread = threading.Thread(target=serve, daemon=True)
    server_thread.start()
    
    print("👀 Watching for file changes in src/ directory...")
    last_mtimes = get_mtimes()
    
    try:
        while True:
            time.sleep(0.5)
            current_mtimes = get_mtimes()
            
            if current_mtimes != last_mtimes:
                print("\\n✨ File change detected! Rebuilding...")
                subprocess.run([sys.executable, "build.py"], check=False)
                last_mtimes = current_mtimes
                
    except KeyboardInterrupt:
        print("\\n🛑 Stopping dev server...")
        sys.exit(0)

if __name__ == "__main__":
    main()
"""

    create_file(os.path.join(project_name, "src", "App.pysx"), app_pysx)
    create_file(os.path.join(project_name, "src", "components", "TodoApp.pysx"), todoapp_pysx)
    create_file(os.path.join(project_name, "runtime", "runtime.js"), runtime_js)
    create_file(os.path.join(project_name, "public", "index.html"), index_html)
    create_file(os.path.join(project_name, "public", "style.css"), style_css)
    
    build_script = os.path.join(project_name, "build.py")
    create_file(build_script, build_py)
    
    dev_script = os.path.join(project_name, "dev.py")
    create_file(dev_script, dev_py)
    
    os.chmod(build_script, 0o755) # Make executable
    os.chmod(dev_script, 0o755)
    
    print(f"\\n✅ Project '{project_name}' created successfully!")
    print("\\nNext steps:")
    print(f"  cd {project_name}")
    print("  python3 dev.py")

if __name__ == "__main__":
    main()
