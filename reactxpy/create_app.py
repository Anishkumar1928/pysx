import os
import sys

def create_file(path, content):
    with open(path, 'w') as f:
        f.write(content.strip() + '\n')
    print(f"Created {path}")

def main():
    if len(sys.argv) > 1 and sys.argv[1] in ["-h", "--help"]:
        print("Usage: create-reactxpy-app [project-name]")
        print("\\nOptions:")
        print("  -h, --help       Show this help message and exit.")
        print("  -v, --version    Show the version of create-reactxpy-app.")
        print("\\nExample:")
        print("  create-reactxpy-app my-new-project")
        sys.exit(0)

    if len(sys.argv) > 1 and sys.argv[1] in ["-v", "--version"]:
        print("create-reactxpy-app version 0.1.0")
        sys.exit(0)

    print("\\033[0;34mWelcome to create-reactxpy-app! Let's scaffold your new project.\\033[0m\\n")

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
        
    print(f"\\n🚀 Creating a new ReactXPy project in {os.path.abspath(project_name)}...")
    
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
import CounterApp

def App():
    isAdmin = true

    return <main class="dashboard-wrapper">
        <h1>Admin Control Panel</h1>
        
        {isAdmin && <div class="secure-badge">Verified Session</div>}
        
        <div class="widgets">
            <CounterApp />
        </div>
    </main>
"""

    # CounterApp.pysx
    counterapp_pysx = """
def CounterApp():
    count, setCount = useState(0)

    useEffect(lambda: localStorage.setItem("clicks", JSON.stringify(count)), [count])

    return <div class="counter-panel">
        <p>Total Clicks: {count}</p>
        <button onClick={lambda: setCount(count + 1)}>Increment Value</button>
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
window.ReactXPy = {
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
    <title>ReactXPy App</title>
    <script src="https://unpkg.com/react@18/umd/react.development.js"></script>
    <script src="https://unpkg.com/react-dom@18/umd/react-dom.development.js"></script>
    <link rel="stylesheet" href="./style.css">
</head>
<body>
    <div id="root"></div>
    <script src="../runtime/runtime.js"></script>
    <script src="../dist/bundle.js"></script>
    <script>
        ReactXPy.run(App);
        
        // Live Reload Hook
        let currentVersion = null;
        setInterval(() => {
            fetch('/version').then(res => res.text()).then(version => {
                if (currentVersion === null) currentVersion = version;
                else if (currentVersion !== version) location.reload();
            }).catch(() => {});
        }, 1000);
    </script>
</body>
</html>
"""

    # style.css
    style_css = """
:root {
    --bg-main: #f8fafc;
    --card-bg: #ffffff;
    --text-primary: #0f172a;
    --text-secondary: #64748b;
    --accent: #4f46e5;
    --accent-hover: #4338ca;
    --success-bg: #dcfce7;
    --success-text: #166534;
}

body {
    margin: 0;
    font-family: 'Inter', system-ui, -apple-system, sans-serif;
    background-color: var(--bg-main);
    color: var(--text-primary);
    display: flex;
    justify-content: center;
    padding-top: 60px;
}

.dashboard-wrapper {
    background: var(--card-bg);
    padding: 40px;
    border-radius: 16px;
    box-shadow: 0 10px 25px rgba(0,0,0,0.05);
    text-align: center;
    max-width: 400px;
    width: 100%;
}

h1 {
    font-size: 1.5rem;
    margin-top: 0;
    margin-bottom: 12px;
}

.secure-badge {
    display: inline-block;
    background: var(--success-bg);
    color: var(--success-text);
    padding: 6px 12px;
    border-radius: 20px;
    font-size: 0.85rem;
    font-weight: 600;
    margin-bottom: 30px;
}

.counter-panel {
    background: #f1f5f9;
    padding: 24px;
    border-radius: 12px;
}

.counter-panel p {
    font-size: 1.25rem;
    font-weight: 600;
    margin-top: 0;
    color: var(--text-secondary);
}

button {
    background: var(--accent);
    color: white;
    border: none;
    padding: 12px 24px;
    border-radius: 8px;
    font-weight: 600;
    font-size: 1rem;
    cursor: pointer;
    transition: 0.2s;
    width: 100%;
}

button:hover {
    background: var(--accent-hover);
    transform: translateY(-2px);
}
"""

    # build.py
    build_py = """
import os
import subprocess
from pathlib import Path

def main():
    print("🚀 Building ReactXPy App...")
    
    os.makedirs("dist", exist_ok=True)
    compiled_files = []
    
    from shutil import which
    if which("reactxpy") is None:
        print("❌ Error: reactxpy compiler not found. Please install the reactxpy pip package globally.")
        return
        
    for file in Path("src").rglob("*.pysx"):
        basename = file.stem
        output_path = f"dist/{basename}.js"
        
        print(f"⚡ Compiling: {file} -> {output_path}")
        subprocess.run(["reactxpy", str(file), "-o", output_path], check=True)
        compiled_files.append(output_path)
        
    if compiled_files:
        print("\\n🔗 Linking into dist/bundle.js...")
        subprocess.run(["reactxpy", "--bundle"] + compiled_files + ["dist/bundle.js"], check=True)
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

BUILD_VERSION = 1

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
            
        def do_GET(self):
            if self.path == '/version':
                self.send_response(200)
                self.send_header('Content-type', 'text/plain')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(str(BUILD_VERSION).encode())
            else:
                super().do_GET()

    TCPServer.allow_reuse_address = True
    try:
        httpd = TCPServer(("", 3000), QuietHandler)
        print("🌐 Live Server running at http://localhost:3000/public/index.html")
        httpd.serve_forever()
    except OSError as e:
        print(f"❌ Could not start dev server: {e}")
        sys.exit(1)

def main():
    global BUILD_VERSION
    print("🚀 Starting ReactXPy Live Development Server...")
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
                BUILD_VERSION += 1
                
    except KeyboardInterrupt:
        print("\\n🛑 Stopping dev server...")
        sys.exit(0)

if __name__ == "__main__":
    main()
"""

    create_file(os.path.join(project_name, "src", "App.pysx"), app_pysx)
    create_file(os.path.join(project_name, "src", "components", "CounterApp.pysx"), counterapp_pysx)
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
