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
                print("\n✨ File change detected! Rebuilding...")
                subprocess.run([sys.executable, "build.py"], check=False)
                last_mtimes = current_mtimes
                BUILD_VERSION += 1
                
    except KeyboardInterrupt:
        print("\n🛑 Stopping dev server...")
        sys.exit(0)

if __name__ == "__main__":
    main()
